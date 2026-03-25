<script lang="ts">
	import { deviceService } from '$lib/services/device.svelte';
	import { debounce } from '$lib/utils';
	import { Accordion, Slider, Label } from 'bits-ui';
	import { slide } from 'svelte/transition';

	let threshold = $state(deviceService.device?.auto_water_threshold ?? 30);
	let duration = $state(deviceService.device?.pump_duration_seconds ?? 15);

	const debouncedSave = debounce(async () => {
		await deviceService.updateSettings(threshold, duration);
	}, 1000);

	// Sync only once on load or when device data is refreshed from server
	$effect(() => {
		if (deviceService.device) {
			threshold = deviceService.device.auto_water_threshold;
			duration = deviceService.device.pump_duration_seconds;
		}
	});

	function handleUpdate() {
		debouncedSave();
	}
</script>

<Accordion.Root
	type="single"
	class="w-full overflow-hidden rounded-3xl border border-outline/20 bg-surface shadow-sm transition-all focus-within:ring-2 focus-within:ring-primary/20"
>
	<Accordion.Item value="settings" class="w-full">
		<Accordion.Header class="flex w-full">
			<Accordion.Trigger
				class="group flex w-full cursor-pointer items-center justify-between p-6 outline-none transition-all hover:bg-surface-variant/30 data-[state=open]:bg-surface-variant/10 focus-visible:ring-2 focus-visible:ring-primary"
			>
				<div class="flex items-center gap-4">
					<div
						class="flex h-11 w-11 items-center justify-center rounded-full bg-surface-variant/50 text-on-surface-variant transition-colors group-data-[state=open]:bg-primary group-data-[state=open]:text-on-primary"
					>
						<span class="material-symbols-outlined text-lg">settings</span>
					</div>
					<div class="flex flex-col text-left gap-1">
						<h3 class="font-bold leading-none tracking-tight text-on-surface">Advanced Settings</h3>
						<p class="text-xs leading-none text-on-surface-variant">
							Automation schedule and thresholds
						</p>
					</div>
				</div>
				<span
					class="material-symbols-outlined text-2xl text-on-surface-variant transition-transform data-[state=open]:rotate-180"
				>
					expand_more
				</span>
			</Accordion.Trigger>
		</Accordion.Header>

		<Accordion.Content class="border-t border-outline/10 bg-surface/50">
			<div transition:slide class="flex flex-col gap-10 p-8">
				<!-- Auto-Water Threshold -->
				<div class="flex flex-col gap-4">
					<div class="flex items-center justify-between gap-4">
						<Label.Root for="threshold" class="flex flex-col gap-1">
							<span class="text-sm font-bold leading-none text-on-surface"
								>Auto-Water Threshold</span
							>
							<span class="text-xs leading-tight text-on-surface-variant"
								>Trigger pump automatically when moisture drops below this level.</span
							>
						</Label.Root>
						<span class="text-sm font-bold text-primary">{threshold}%</span>
					</div>
					<Slider.Root
						id="threshold"
						bind:value={threshold}
						min={0}
						max={100}
						type="single"
						onValueChange={handleUpdate}
						class="relative flex h-12 w-full touch-none select-none items-center rounded-full"
					>
						<div class="h-2 w-full rounded-full bg-surface-variant relative overflow-hidden">
							<Slider.Range class="absolute h-full rounded-full bg-primary" />
						</div>
						<Slider.Thumb
							index={0}
							class="block h-6 w-6 cursor-pointer rounded-full bg-primary shadow-md outline-none ring-4 ring-primary/20 transition-all hover:ring-primary/40 focus-visible:ring-4 focus-visible:ring-primary/40 focus-visible:ring-offset-2"
						/>
					</Slider.Root>
				</div>

				<!-- Pump Duration -->
				<div class="flex flex-col gap-4">
					<div class="flex items-center justify-between gap-4">
						<Label.Root for="duration" class="flex flex-col gap-1">
							<span class="text-sm font-bold leading-none text-on-surface">Pump Duration</span>
							<span class="text-xs leading-tight text-on-surface-variant"
								>How long the pump runs during each automated phase.</span
							>
						</Label.Root>
						<span class="text-sm font-bold text-primary">{duration}s</span>
					</div>
					<Slider.Root
						id="duration"
						bind:value={duration}
						min={5}
						max={60}
						step={5}
						type="single"
						onValueChange={handleUpdate}
						class="relative flex h-12 w-full touch-none select-none items-center rounded-full"
					>
						<div class="h-2 w-full rounded-full bg-surface-variant relative overflow-hidden">
							<Slider.Range class="absolute h-full rounded-full bg-primary" />
						</div>
						<Slider.Thumb
							index={0}
							class="block h-6 w-6 cursor-pointer rounded-full bg-primary shadow-md outline-none ring-4 ring-primary/20 transition-all hover:ring-primary/40 focus-visible:ring-4 focus-visible:ring-primary/40 focus-visible:ring-offset-2"
						/>
					</Slider.Root>
				</div>
			</div>
		</Accordion.Content>
	</Accordion.Item>
</Accordion.Root>
