<script lang="ts">
	import { Button } from 'bits-ui';
	import { cn } from '$lib/utils';

	interface Props {
		watering?: boolean;
		onpressstart?: () => void;
		onpressend?: () => void;
	}
	let { watering = false, onpressstart, onpressend }: Props = $props();

	function handlePointerDown(e: PointerEvent) {
		if (onpressstart) onpressstart();
	}

	function handlePointerUp() {
		if (onpressend) onpressend();
	}
</script>

<Button.Root
	class={cn(
		'relative flex h-20 w-full cursor-pointer items-center justify-center gap-4 overflow-hidden rounded-3xl px-8 py-4 font-bold shadow-md outline-none transition-all active:shadow-sm focus-visible:ring-2 focus-visible:ring-primary focus-visible:ring-offset-2',
		watering
			? 'bg-primary text-on-primary'
			: 'bg-surface-variant text-on-surface-variant hover:bg-surface-variant/80'
	)}
	onpointerdown={handlePointerDown}
	onpointerup={handlePointerUp}
	onpointerleave={handlePointerUp}
>
	{#if watering}
		<div class="absolute inset-0 animate-pulse bg-primary-container/20"></div>
		<span class="material-symbols-outlined animate-bounce text-3xl">water_drop</span>
		<div class="z-10 flex flex-col items-start gap-1">
			<span class="text-xs uppercase leading-none tracking-widest opacity-80">Status</span>
			<span class="text-xl leading-none">Watering...</span>
		</div>
	{:else}
		<span class="material-symbols-outlined text-3xl">opacity</span>
		<div class="flex flex-col items-start text-left gap-1">
			<span class="text-xs uppercase leading-none tracking-widest opacity-60">Manual Override</span>
			<span class="text-xl leading-none">Water Plant</span>
		</div>
	{/if}
</Button.Root>
