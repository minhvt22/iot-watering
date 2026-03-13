<script lang="ts">
	interface Props {
		connected?: boolean;
	}
	let { connected = false }: Props = $props();
</script>

<header
	style="background-color: var(--color-bg-overlay); border-bottom-color: var(--color-border);"
	class="sticky top-0 z-50 h-16 border-b backdrop-blur-md"
>
	<div class="mx-auto flex h-full max-w-4xl items-center justify-between px-4">
		<div class="flex items-center gap-3">
			<span class="material-symbols-outlined" style="color: var(--color-primary);">
				potted_plant
			</span>
			<h1 class="text-xl font-semibold tracking-tight" style="color: var(--color-text-primary);">
				Plant Monitor
			</h1>
		</div>

		<div
			class="flex items-center gap-2 rounded-full border px-3 py-1"
			style="border-color: var(--color-primary-border); background-color: var(--color-primary-muted);"
		>
			<span class="relative flex h-2 w-2">
				{#if connected}
					<span
						class="absolute inline-flex h-full w-full animate-ping rounded-full opacity-75"
						style="background-color: var(--color-primary);"
					></span>
					<span
						class="relative inline-flex h-2 w-2 rounded-full"
						style="background-color: var(--color-primary);"
					></span>
				{:else}
					<span
						class="relative inline-flex h-2 w-2 rounded-full"
						style="background-color: var(--color-text-muted);"
					></span>
				{/if}
			</span>
			<span class="text-xs font-medium" style="color: var(--color-text-secondary);">
				{connected ? 'Connected' : 'Disconnected'}
			</span>
		</div>
	</div>
</header>
