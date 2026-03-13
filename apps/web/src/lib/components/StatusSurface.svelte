<script lang="ts">
	interface Props {
		moisture?: number;
		online?: boolean;
		lastSync?: string;
	}
	let { moisture = 0, online = false, lastSync = 'N/A' }: Props = $props();

	let stateLabel = $derived(moisture > 70 ? 'Optimal' : moisture > 30 ? 'Dry' : 'Critical');
	let stateColor = $derived(
		moisture > 70
			? 'var(--color-success)'
			: moisture > 30
				? 'var(--color-warning)'
				: 'var(--color-error)'
	);
</script>

<div
	class="relative overflow-hidden rounded-3xl border p-6 sm:p-8 transition-colors duration-300"
	style="
		background-color: var(--color-bg-surface);
		border-color: var(--color-border);
		box-shadow: var(--shadow-soft);
	"
>
	<!-- Background watermark -->
	<div class="pointer-events-none absolute -top-4 -right-4 p-4 opacity-[0.03]">
		<span class="material-symbols-outlined text-9xl" style="color: var(--color-primary);">
			water_drop
		</span>
	</div>

	<div class="relative z-10 flex h-full flex-col justify-between">
		<div>
			<p
				class="mb-2 text-xs font-bold uppercase tracking-[0.2em]"
				style="color: var(--color-text-muted);"
			>
				Soil Moisture
			</p>

			<div class="flex items-baseline gap-3">
				<span class="text-7xl font-bold tracking-tighter" style="color: var(--color-text-primary);">
					{moisture}<span class="text-5xl font-semibold opacity-50">%</span>
				</span>
				<span
					class="flex items-center gap-1 rounded-full px-2.5 py-1 text-xs font-semibold tracking-wide"
					style="color: {stateColor}; background-color: color-mix(in srgb, {stateColor} 15%, transparent);"
				>
					<span class="h-1.5 w-1.5 rounded-full" style="background-color: {stateColor};"></span>
					{stateLabel}
				</span>
			</div>
		</div>

		<!-- Moisture bar -->
		<div class="mt-8 mb-8">
			<div
				class="h-3 w-full overflow-hidden rounded-full"
				style="background-color: var(--color-bg-base); box-shadow: var(--shadow-inner-soft);"
			>
				<div
					class="h-full rounded-full transition-all duration-1000 ease-out"
					style="width: {moisture}%; background-color: {stateColor};"
				></div>
			</div>
		</div>

		<!-- Status Footer -->
		<div
			class="flex items-center gap-6 border-t pt-5"
			style="border-color: var(--color-border-muted);"
		>
			<div class="flex flex-col gap-1">
				<span
					class="text-2xs font-bold tracking-wider uppercase"
					style="color: var(--color-text-muted);"
				>
					Network
				</span>
				<span
					class="flex items-center gap-1.5 text-sm font-medium"
					style="color: var(--color-text-secondary);"
				>
					<span
						class="material-symbols-outlined text-lg"
						style="color: {online ? 'var(--color-success)' : 'var(--color-error)'};"
					>
						{online ? 'cloud_done' : 'cloud_off'}
					</span>
					{online ? 'Online' : 'Offline'}
				</span>
			</div>

			<div class="h-8 w-px" style="background-color: var(--color-border-muted);"></div>

			<div class="flex flex-col gap-1">
				<span
					class="text-2xs font-bold tracking-wider uppercase"
					style="color: var(--color-text-muted);"
				>
					Updated
				</span>
				<span
					class="flex items-center gap-1.5 text-sm font-medium"
					style="color: var(--color-text-secondary);"
				>
					<span class="material-symbols-outlined text-lg opacity-70">schedule</span>
					{lastSync}
				</span>
			</div>
		</div>
	</div>
</div>
