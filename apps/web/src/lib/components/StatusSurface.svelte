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
	class="group relative flex flex-col justify-between overflow-hidden rounded-3xl border border-outline-variant bg-surface-glass p-8 shadow-sm backdrop-blur-xl transition-all duration-300 hover:shadow-md"
>
	<!-- Glassmorphism overlay + Watermark -->
	<div
		class="pointer-events-none absolute inset-0 bg-linear-to-br from-primary/5 to-transparent"
	></div>
	<div
		class="pointer-events-none absolute -right-10 -top-10 p-4 opacity-[0.04] transition-transform duration-700 group-hover:-rotate-12 group-hover:scale-110"
	>
		<span class="material-symbols-outlined text-primary" style="font-size: 200px;">water_drop</span>
	</div>

	<div class="relative z-10 flex h-full flex-col justify-between gap-10">
		<div class="flex flex-col gap-3">
			<p class="text-xs font-bold uppercase tracking-widest text-on-surface-variant/80">
				Soil Moisture
			</p>

			<div class="flex items-baseline gap-4">
				<div class="flex items-baseline gap-1">
					<span class="text-8xl font-black tracking-tighter text-on-surface">{moisture}</span>
					<span class="text-4xl font-medium opacity-30">%</span>
				</div>
				<span
					class="flex items-center gap-1.5 rounded-full px-4 py-1.5 text-xs font-bold uppercase tracking-wider backdrop-blur-md"
					style="color: {stateColor}; background-color: color-mix(in srgb, {stateColor} 12%, transparent);"
				>
					<span class="h-2 w-2 animate-pulse rounded-full" style="background-color: {stateColor};"
					></span>
					{stateLabel}
				</span>
			</div>
		</div>

		<!-- Moisture bar - Animated Gradient -->
		<div class="flex w-full flex-col">
			<div class="h-4 w-full overflow-hidden rounded-full bg-surface-variant/30 shadow-inner">
				<div
					class="relative h-full rounded-full transition-all duration-1000 ease-[cubic-bezier(0.34,1.56,0.64,1)]"
					style="width: {moisture}%; background-color: {stateColor};"
				>
					<!-- Shine effect -->
					<div
						class="animate-shimmer absolute inset-0 bg-linear-to-r from-transparent via-white/20 to-transparent"
					></div>
				</div>
			</div>
		</div>

		<!-- Status Footer -->
		<div class="flex items-center gap-8 border-t border-outline/10 pt-6">
			<div class="flex items-center gap-3">
				<div class="rounded-2xl bg-surface-variant/40 p-2">
					<span
						class="material-symbols-outlined text-2xl"
						style="color: {online ? 'var(--color-success)' : 'var(--color-error)'};"
					>
						{online ? 'cloud_done' : 'cloud_off'}
					</span>
				</div>
				<div class="flex flex-col gap-1">
					<span
						class="text-xs font-bold uppercase tracking-widest text-on-surface-variant/60 leading-none"
						>Network</span
					>
					<span class="text-sm font-bold text-on-surface leading-none"
						>{online ? 'Online' : 'Offline'}</span
					>
				</div>
			</div>

			<div class="flex items-center gap-3">
				<div class="rounded-2xl bg-surface-variant/40 p-2">
					<span class="material-symbols-outlined text-2xl text-on-surface-variant/70">schedule</span
					>
				</div>
				<div class="flex flex-col gap-1">
					<span
						class="text-xs font-bold uppercase tracking-widest text-on-surface-variant/60 leading-none"
						>Updated</span
					>
					<span class="text-sm font-bold text-on-surface leading-none">{lastSync}</span>
				</div>
			</div>
		</div>
	</div>
</div>

<style>
	@keyframes shimmer {
		0% {
			transform: translateX(-100%);
		}
		100% {
			transform: translateX(100%);
		}
	}
	.animate-shimmer {
		animation: shimmer 2s infinite;
	}
</style>
