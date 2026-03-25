<script lang="ts">
	import { Tooltip } from 'bits-ui';
	import { fade } from 'svelte/transition';

	interface Props {
		connected?: boolean;
		onadd?: () => void;
	}
	let { connected = false, onadd }: Props = $props();
</script>

<div class="sticky top-4 z-50 mx-4 max-w-4xl lg:mx-auto">
	<header
		class="flex h-16 items-center justify-between rounded-2xl border border-outline-variant bg-surface-glass px-6 shadow-sm backdrop-blur-xl"
	>
		<div class="flex items-center gap-4">
			<div
				class="flex h-10 w-10 items-center justify-center rounded-2xl bg-primary-container text-primary"
			>
				<span class="material-symbols-outlined text-2xl">potted_plant</span>
			</div>
			<div class="flex flex-col gap-1">
				<h1 class="text-lg font-bold leading-none tracking-tight text-on-surface">Plant Monitor</h1>
				<p class="text-xs text-on-surface-variant leading-none">Automated Irrigation</p>
			</div>
		</div>

		<div class="flex items-center gap-6">
			<Tooltip.Root>
				<Tooltip.Trigger
					onclick={onadd}
					class="flex h-11 w-11 cursor-pointer items-center justify-center rounded-full text-on-surface-variant outline-none transition-colors hover:bg-on-surface-variant/10 focus-visible:ring-2 focus-visible:ring-primary"
					aria-label="Add Device"
				>
					<span class="material-symbols-outlined">add</span>
				</Tooltip.Trigger>
				<Tooltip.Content
					transition={fade}
					class="z-50 rounded-lg bg-on-surface px-3 py-1.5 text-xs font-bold text-surface shadow-md"
				>
					Pair New System
				</Tooltip.Content>
			</Tooltip.Root>

			<div
				class="flex items-center gap-2 rounded-full border border-outline-variant bg-surface-variant px-4 py-1.5 shadow-sm"
			>
				<div class="relative flex h-2 w-2 items-center justify-center">
					{#if connected}
						<span
							class="absolute inline-flex h-full w-full animate-ping rounded-full bg-primary opacity-60"
						></span>
						<span class="relative inline-flex h-2 w-2 rounded-full bg-primary"></span>
					{:else}
						<span class="relative inline-flex h-2 w-2 rounded-full bg-on-surface-variant/30"></span>
					{/if}
				</div>
				<span class="text-xs font-bold uppercase tracking-wider text-on-surface-variant">
					{connected ? 'Live' : 'Offline'}
				</span>
			</div>
		</div>
	</header>
</div>
