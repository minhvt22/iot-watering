<script lang="ts">
	import TopAppBar from '$lib/components/TopAppBar.svelte';
	import StatusSurface from '$lib/components/StatusSurface.svelte';
	import WaterPlantButton from '$lib/components/WaterPlantButton.svelte';
	import ChartCard from '$lib/components/ChartCard.svelte';
	import AdvancedSettings from '$lib/components/AdvancedSettings.svelte';
	import DevicePairing from '$lib/components/DevicePairing.svelte';
	import { deviceService } from '$lib/services/device.svelte';
	import { onMount } from 'svelte';

	let isWatering = $state(false);
	let showPairing = $state(false);

	// --- Derived Display Values ---
	const moisture = $derived(deviceService.latestTelemetry?.moisture ?? 0);
	const lastSync = $derived(
		deviceService.latestTelemetry
			? new Date(deviceService.latestTelemetry.recorded_at).toLocaleTimeString([], {
					hour: '2-digit',
					minute: '2-digit'
				})
			: 'No Signal'
	);

	const moistureData = $derived(deviceService.telemetry.map((r) => Number(r.moisture)).reverse());
	const moistureLabels = $derived(
		deviceService.telemetry
			.map((r) =>
				new Date(r.recorded_at).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
			)
			.reverse()
	);

	const temperatureData = $derived(
		deviceService.telemetry.map((r) => Number(r.temperature ?? 0)).reverse()
	);
	const temperatureLabels = $derived(moistureLabels);

	onMount(() => {
		deviceService.init();
		return () => deviceService.dispose();
	});

	let wateringInterval: ReturnType<typeof setInterval> | undefined;

	function startWatering() {
		if (!deviceService.device) return;
		isWatering = true;
		deviceService.sendCommand('pump_on');
	}

	function stopWatering() {
		isWatering = false;
		deviceService.sendCommand('pump_off');
	}
</script>

<div class="flex min-h-dvh flex-col bg-surface">
	<TopAppBar connected={true} onadd={() => (showPairing = true)} />

	<main class="mx-auto flex w-full max-w-4xl flex-1 flex-col gap-12 p-4 pt-12 pb-24 lg:p-8">
		{#if !deviceService.device && !showPairing && !deviceService.loading}
			<div
				class="flex flex-col items-center justify-center gap-4 rounded-3xl border border-dashed border-primary/20 bg-primary/5 p-12 text-center"
			>
				<span class="material-symbols-outlined text-6xl text-primary">potted_plant</span>
				<div class="flex flex-col gap-2">
					<h2 class="text-xl font-bold text-on-surface">No Device Found</h2>
					<p class="max-w-sm text-on-surface-variant">
						You haven't paired any watering systems yet. Pair your device to start monitoring and
						watering your plants.
					</p>
				</div>
				<div class="pt-4 flex">
					<button
						onclick={() => (showPairing = true)}
						class="rounded-full bg-primary px-8 py-3 font-bold text-on-primary shadow-sm outline-none transition-all hover:bg-primary-hover focus-visible:ring-2 focus-visible:ring-primary focus-visible:ring-offset-2"
					>
						Start Pairing
					</button>
				</div>
			</div>
		{/if}

		{#if deviceService.loading}
			<div class="flex h-64 items-center justify-center">
				<div
					class="h-12 w-12 animate-spin rounded-full border-4 border-primary border-t-transparent"
				></div>
			</div>
		{:else}
			<!-- Hero Section -->
			<div
				class="grid grid-cols-1 gap-6 md:grid-cols-12 md:gap-8"
				class:opacity-50={!deviceService.device}
				class:pointer-events-none={!deviceService.device}
			>
				<div class="md:col-span-7 lg:col-span-8 flex flex-col">
					<StatusSurface {moisture} online={deviceService.online} {lastSync} />
				</div>

				<div class="flex flex-col justify-center gap-4 px-2 md:col-span-5 md:px-0 lg:col-span-4">
					<WaterPlantButton
						watering={isWatering}
						onpressstart={startWatering}
						onpressend={stopWatering}
					/>
					<p class="text-center text-sm leading-relaxed text-on-surface-variant">
						Press and hold to manually override.<br class="hidden lg:block" />
						Release to stop pump.
					</p>
				</div>
			</div>

			{#if deviceService.device}
				<AdvancedSettings />

				<!-- History Section -->
				<div class="flex flex-col gap-6">
					<h2 class="px-1 text-xl font-bold tracking-tight text-on-surface">Dashboard</h2>
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
			{/if}
		{/if}
	</main>
</div>

<DevicePairing bind:open={showPairing} />
