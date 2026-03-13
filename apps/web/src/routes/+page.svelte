<script lang="ts">
	import TopAppBar from '$lib/components/TopAppBar.svelte';
	import StatusSurface from '$lib/components/StatusSurface.svelte';
	import WaterPlantButton from '$lib/components/WaterPlantButton.svelte';
	import ChartCard from '$lib/components/ChartCard.svelte';
	import AdvancedSettings from '$lib/components/AdvancedSettings.svelte';
	import { supabase } from '$lib/supabase/client';
	import type { TelemetryRow, Database } from '$lib/supabase/types';

	// ─── Reactive state ────────────────────────────────────────
	let connected = $state(true);
	let moisture = $state(0);
	let online = $state(false);
	let lastSync = $state('Connecting...');
	let isWatering = $state(false);

	// ─── Chart data ────────────────────────────────────────────
	let moistureLabels = $state<string[]>([]);
	let moistureData = $state<number[]>([]);
	let temperatureLabels = $state<string[]>([]);
	let temperatureData = $state<number[]>([]);

	// Hardcoded device ID - in production this comes from auth/session
	const DEVICE_ID = 'YOUR_DEVICE_UUID';

	// ─── Helpers ───────────────────────────────────────────────
	function formatTime(iso: string) {
		return new Date(iso).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
	}

	function applyTelemetry(rows: TelemetryRow[]) {
		if (!rows.length) return;
		const latest = rows[0];
		moisture = Number(latest.moisture);
		lastSync = formatTime(latest.recorded_at);
		online = true;
		// Chart: oldest → newest
		const ordered = [...rows].reverse();
		moistureLabels = ordered.map((r) => formatTime(r.recorded_at));
		moistureData = ordered.map((r) => Number(r.moisture));
		temperatureLabels = ordered.map((r) => formatTime(r.recorded_at));
		temperatureData = ordered.map((r) => Number(r.temperature ?? 0));
	}

	// ─── Initial load ──────────────────────────────────────────
	async function loadHistory() {
		const since = new Date(Date.now() - 24 * 60 * 60 * 1000).toISOString();
		const { data, error } = await supabase
			.from('telemetry')
			.select('*')
			.eq('device_id', DEVICE_ID)
			.gte('recorded_at', since)
			.order('recorded_at', { ascending: false })
			.limit(48); // ~30-min intervals across 24h

		if (error) {
			console.error('[Supabase] History fetch error:', error.message);
			useFallbackMocks();
			return;
		}
		if (data && data.length) {
			applyTelemetry(data as TelemetryRow[]);
		} else {
			useFallbackMocks(); // No data yet → show mocks during dev
		}
	}

	// ─── Realtime subscription ─────────────────────────────────
	function subscribeRealtime() {
		supabase
			.channel(`telemetry:${DEVICE_ID}`)
			.on(
				'postgres_changes',
				{
					event: 'INSERT',
					schema: 'public',
					table: 'telemetry',
					filter: `device_id=eq.${DEVICE_ID}`
				},
				(payload) => {
					const row = payload.new as TelemetryRow;
					moisture = Number(row.moisture);
					lastSync = formatTime(row.recorded_at);
					online = true;
					// Append to chart (drop oldest to keep 48 points)
					moistureLabels = [...moistureLabels.slice(-47), formatTime(row.recorded_at)];
					moistureData = [...moistureData.slice(-47), Number(row.moisture)];
					temperatureLabels = [...temperatureLabels.slice(-47), formatTime(row.recorded_at)];
					temperatureData = [...temperatureData.slice(-47), Number(row.temperature ?? 0)];
				}
			)
			.subscribe((status) => {
				connected = status === 'SUBSCRIBED';
			});
	}

	// ─── Device devices realtime (online/offline status) ───────
	function subscribeDevice() {
		supabase
			.channel(`devices:${DEVICE_ID}`)
			.on(
				'postgres_changes',
				{
					event: 'UPDATE',
					schema: 'public',
					table: 'devices',
					filter: `id=eq.${DEVICE_ID}`
				},
				(payload) => {
					online = (payload.new as { online: boolean }).online;
				}
			)
			.subscribe();
	}

	// ─── Pump command ──────────────────────────────────────────
	async function issuePumpCommand(cmd: 'pump_on' | 'pump_off', durationSeconds?: number) {
		await supabase.from('commands').insert({
			device_id: DEVICE_ID,
			command: cmd,
			duration_seconds: durationSeconds ?? null
		} as Database['public']['Tables']['commands']['Insert']);
	}

	let wateringInterval: ReturnType<typeof setInterval> | undefined;

	function startWatering() {
		isWatering = true;
		issuePumpCommand('pump_on');
		// Optimistic local moisture bump while holding (replaced by realtime when device responds)
		wateringInterval = setInterval(() => {
			if (moisture < 100) moisture += 0.5;
		}, 200);
	}

	function stopWatering() {
		isWatering = false;
		issuePumpCommand('pump_off');
		if (wateringInterval) {
			clearInterval(wateringInterval);
			wateringInterval = undefined;
		}
	}

	// ─── Dev fallback mocks ────────────────────────────────────
	function useFallbackMocks() {
		console.info('[Dev] Using mock data — connect device or set DEVICE_ID.');
		moisture = 85;
		online = false;
		lastSync = 'No data';
		moistureLabels = Array.from({ length: 12 }, (_, i) => `${i * 2}h`);
		moistureData = [65, 70, 68, 80, 85, 82, 78, 85, 90, 88, 85, 85];
		temperatureLabels = Array.from({ length: 12 }, (_, i) => `${i * 2}h`);
		temperatureData = [22, 22.5, 23, 24.5, 26, 27, 28, 27.5, 26, 24, 23, 22.5];
	}

	// ─── Lifecycle ──────────────────────────────────────────────
	$effect(() => {
		loadHistory();
		subscribeRealtime();
		subscribeDevice();

		return () => {
			supabase.removeAllChannels();
		};
	});
</script>

<TopAppBar {connected} />

<main class="mx-auto flex w-full max-w-4xl flex-1 flex-col p-4 pb-24 lg:p-8">
	<!-- Hero Section -->
	<div class="grid grid-cols-1 gap-6 md:grid-cols-12 md:gap-8">
		<div class="md:col-span-7 lg:col-span-8">
			<StatusSurface {moisture} {online} {lastSync} />
		</div>

		<div class="flex flex-col justify-center space-y-4 px-2 md:col-span-5 lg:col-span-4 md:px-0">
			<div class="flex flex-col gap-2">
				<WaterPlantButton
					watering={isWatering}
					onpressstart={startWatering}
					onpressend={stopWatering}
				/>
				<p class="text-center text-sm leading-relaxed" style="color: var(--color-text-muted);">
					Press and hold to manually override.<br class="hidden lg:block" />
					Release to stop pump.
				</p>
			</div>
		</div>
	</div>

	<AdvancedSettings />

	<!-- History Section -->
	<div class="mt-12 mb-4">
		<h2
			class="mb-6 px-1 text-xl font-bold tracking-tight"
			style="color: var(--color-text-primary);"
		>
			Dashboard
		</h2>

		<div class="grid grid-cols-1 gap-6 md:grid-cols-2">
			<ChartCard title="Moisture History" data={moistureData} labels={moistureLabels} />

			<ChartCard
				title="Temperature"
				type="line"
				color="#f59e0b"
				icon="device_thermostat"
				data={temperatureData}
				labels={temperatureLabels}
			/>
		</div>
	</div>
</main>
