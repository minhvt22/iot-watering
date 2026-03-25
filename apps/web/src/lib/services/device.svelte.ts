import { supabase } from '$lib/supabase/client';
import type { DeviceRow, TelemetryRow } from '$lib/supabase/types';

export class DeviceService {
	// --- App State (Svelte 5 Reactive) ---
	device = $state<DeviceRow | null>(null);
	telemetry = $state<TelemetryRow[]>([]);
	latestTelemetry = $derived(this.telemetry[0] || null);
	online = $derived(this.device?.online ?? false);

	loading = $state(true);
	error = $state<string | null>(null);

	private channel: any;

	async init() {
		this.loading = true;
		try {
			// Get owner device
			const { data: userData } = await supabase.auth.getUser();
			if (!userData.user) return;

			const { data: devices, error: devError } = await supabase
				.from('devices')
				.select('*')
				.eq('owner_id', userData.user.id)
				.limit(1);

			if (devError) throw devError;

			if (devices && (devices as DeviceRow[]).length > 0) {
				this.device = (devices as DeviceRow[])[0];
				await this.fetchTelemetry();
				this.subscribeToRealtime();
			}
		} catch (e: any) {
			this.error = e.message;
		} finally {
			this.loading = false;
		}
	}

	private async fetchTelemetry() {
		if (!this.device) return;
		const { data, error } = await supabase
			.from('telemetry')
			.select('*')
			.eq('device_id', this.device.id)
			.order('recorded_at', { ascending: false })
			.limit(50);

		if (!error && data) {
			this.telemetry = data as TelemetryRow[];
		}
	}

	private subscribeToRealtime() {
		if (!this.device) return;

		console.log(`[Device] Subscribing to realtime for ${this.device.id}`);

		this.channel = supabase
			.channel('realtime:commands')
			.on(
				'postgres_changes',
				{
					event: '*',
					schema: 'public',
					table: 'devices',
					filter: `id=eq.${this.device.id}`
				},
				(payload) => {
					console.log('[Device] Update detected:', payload);
					this.device = payload.new as DeviceRow;
				}
			)
			.on(
				'postgres_changes',
				{
					event: 'INSERT',
					schema: 'public',
					table: 'telemetry',
					filter: `device_id=eq.${this.device.id}`
				},
				(payload) => {
					console.log('[Telemetry] New reading:', payload.new);
					this.telemetry = [payload.new as TelemetryRow, ...this.telemetry].slice(0, 50);
				}
			)
			.subscribe((status) => {
				console.log('[Realtime] Subscription status:', status);
				if (status === 'CHANNEL_ERROR') {
					this.error = 'Connection lost. Retrying...';
				}
			});
	}

	// --- Actions ---
	async updateSettings(threshold: number, duration: number) {
		if (!this.device) return;
		const { error } = await supabase
			.from('devices')
			.update({
				auto_water_threshold: threshold,
				pump_duration_seconds: duration
			})
			.eq('id', this.device.id);

		if (!error) {
			this.device = {
				...this.device,
				auto_water_threshold: threshold,
				pump_duration_seconds: duration
			};
		} else {
			this.error = error.message;
		}
	}

	async sendCommand(command: 'pump_on' | 'pump_off', duration?: number) {
		if (!this.device) return;
		const { error } = await supabase.from('commands').insert({
			device_id: this.device.id,
			command,
			duration_seconds: duration || 10,
			status: 'pending'
		});

		if (error) this.error = error.message;
	}

	dispose() {
		if (this.channel) supabase.removeChannel(this.channel);
	}
}

export const deviceService = new DeviceService();
