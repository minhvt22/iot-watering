<script lang="ts">
	let expanded = $state(false);

	let threshold = $state(30); // auto-water below 30%
	let duration = $state(15); // pump for 15 seconds

	function toggle() {
		expanded = !expanded;
	}
</script>

<div
	class="mt-10 overflow-hidden rounded-3xl border transition-colors duration-300"
	style="
		background-color: var(--color-bg-surface);
		border-color: {expanded ? 'var(--color-primary-border)' : 'var(--color-border)'};
		box-shadow: {expanded ? 'var(--shadow-soft)' : 'none'};
	"
>
	<button
		onclick={toggle}
		class="flex w-full cursor-pointer items-center justify-between p-6 focus:outline-none"
	>
		<div class="flex items-center gap-4">
			<div
				class="flex h-10 w-10 items-center justify-center rounded-full transition-colors duration-300"
				style="background-color: {expanded ? 'var(--color-primary)' : 'var(--color-bg-base)'};"
			>
				<span
					class="material-symbols-outlined text-[20px]"
					style="color: {expanded
						? 'var(--color-text-on-primary)'
						: 'var(--color-text-secondary)'};"
				>
					settings
				</span>
			</div>
			<div class="text-left">
				<h3 class="font-bold tracking-tight" style="color: var(--color-text-primary);">
					Advanced Settings
				</h3>
				<p class="text-xs mt-0.5" style="color: var(--color-text-muted);">
					Automation schedule and thresholds
				</p>
			</div>
		</div>
		<span
			class="material-symbols-outlined text-[24px] transition-transform duration-300"
			style="color: var(--color-text-muted); transform: rotate({expanded ? 180 : 0}deg);"
		>
			expand_more
		</span>
	</button>

	<!-- Content -->
	<div
		class="grid transition-all duration-300 ease-in-out"
		style="grid-template-rows: {expanded ? '1fr' : '0fr'};"
	>
		<div class="overflow-hidden">
			<div class="border-t p-6 pt-2" style="border-color: var(--color-border-muted);">
				<div class="space-y-8 py-2">
					<!-- Auto-Water Threshold -->
					<div class="flex flex-col gap-3 md:flex-row md:items-center md:justify-between">
						<div>
							<span class="text-sm font-bold block" style="color: var(--color-text-primary);">
								Auto-Water Threshold
							</span>
							<span class="text-xs" style="color: var(--color-text-muted);">
								Trigger pump automatically when moisture drops below this level.
							</span>
						</div>
						<div class="flex w-full items-center gap-4 md:w-auto">
							<input
								type="range"
								min="0"
								max="100"
								bind:value={threshold}
								class="h-2 w-full cursor-pointer appearance-none rounded-full md:w-48"
								style="
									background-color: var(--color-border-strong);
									accent-color: var(--color-primary);
								"
							/>
							<span class="w-12 text-right text-sm font-bold" style="color: var(--color-primary);">
								{threshold}%
							</span>
						</div>
					</div>

					<!-- Pump Duration -->
					<div class="flex flex-col gap-3 md:flex-row md:items-center md:justify-between">
						<div>
							<span class="text-sm font-bold block" style="color: var(--color-text-primary);">
								Pump Duration
							</span>
							<span class="text-xs" style="color: var(--color-text-muted);">
								How long the pump runs during each automated phase.
							</span>
						</div>
						<div class="flex w-full items-center gap-4 md:w-auto">
							<input
								type="range"
								min="5"
								max="60"
								step="5"
								bind:value={duration}
								class="h-2 w-full cursor-pointer appearance-none rounded-full md:w-48"
								style="
									background-color: var(--color-border-strong);
									accent-color: var(--color-primary);
								"
							/>
							<span class="w-12 text-right text-sm font-bold" style="color: var(--color-primary);">
								{duration}s
							</span>
						</div>
					</div>
				</div>
			</div>
		</div>
	</div>
</div>
