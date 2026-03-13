<script lang="ts">
	interface Props {
		watering?: boolean;
		onpressstart?: () => void;
		onpressend?: () => void;
	}
	let { watering = false, onpressstart, onpressend }: Props = $props();

	function handlePointerDown(e: PointerEvent) {
		// Only primary button (left click or touch)
		if (e.isPrimary && (e.button === 0 || e.pointerType === 'touch')) {
			e.preventDefault(); // Prevent default text selection/scrolling
			const currentTarget = e.currentTarget as HTMLElement;
			currentTarget.setPointerCapture(e.pointerId);
			if (onpressstart) onpressstart();
		}
	}

	function handlePointerUp(e: PointerEvent) {
		if (e.isPrimary) {
			const currentTarget = e.currentTarget as HTMLElement;
			if (currentTarget.hasPointerCapture(e.pointerId)) {
				currentTarget.releasePointerCapture(e.pointerId);
			}
			if (onpressend) onpressend();
		}
	}

	function handlePointerCancel(e: PointerEvent) {
		handlePointerUp(e);
	}
</script>

<button
	onpointerdown={handlePointerDown}
	onpointerup={handlePointerUp}
	onpointercancel={handlePointerCancel}
	oncontextmenu={(e) => e.preventDefault()}
	class="flex h-16 w-full cursor-pointer touch-none select-none items-center justify-center gap-3 rounded-xl text-lg font-bold transition-all duration-200 active:scale-95"
	style="
		background-color: {watering ? 'var(--color-primary-active)' : 'var(--color-primary)'};
		color: var(--color-text-on-primary);
		box-shadow: var(--shadow-m3-2);
	"
	onmouseenter={(e) => {
		if (!watering)
			(e.currentTarget as HTMLElement).style.backgroundColor = 'var(--color-primary-hover)';
	}}
	onmouseleave={(e) => {
		if (!watering) (e.currentTarget as HTMLElement).style.backgroundColor = 'var(--color-primary)';
		// Use native mouseleave as a fallback to cancel if pointer capture missed something
		if (watering && onpressend) onpressend();
	}}
>
	{#if watering}
		<span class="material-symbols-outlined animate-spin font-bold">progress_activity</span>
		Watering...
	{:else}
		<span class="material-symbols-outlined font-bold">water_drop</span>
		Water Plant
	{/if}
</button>
