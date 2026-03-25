<script lang="ts">
	import { deviceService } from '$lib/services/device.svelte';
	import { pairingService } from '$lib/services/pairing.svelte';
	import { Dialog } from 'bits-ui';
	import { fly, fade } from 'svelte/transition';

	interface Props {
		open?: boolean;
	}
	let { open = $bindable(false) }: Props = $props();

	let macAddress = $state('');
	let pin = $state('');

	async function startPairing() {
		await pairingService.startPairing(macAddress, pin, async () => {
			await deviceService.init();
		});
	}

	function close() {
		open = false;
		pairingService.reset();
		macAddress = '';
		pin = '';
	}
</script>

<Dialog.Root bind:open>
	<Dialog.Portal>
		<Dialog.Overlay class="fixed inset-0 z-50 bg-surface/80 backdrop-blur-sm">
			<div transition:fade={{ duration: 150 }} class="absolute inset-0"></div>
		</Dialog.Overlay>
		<Dialog.Content class="fixed inset-0 z-50 flex w-full items-center justify-center p-4">
			<div
				transition:fly={{ y: 8, duration: 200 }}
				class="relative flex w-full max-w-md flex-col overflow-hidden rounded-3xl bg-surface p-8 shadow-lg"
			>
				<Dialog.Close
					class="absolute right-4 top-4 flex h-11 w-11 items-center justify-center rounded-full text-on-surface-variant transition-colors hover:bg-on-surface-variant/10 focus-visible:ring-2 focus-visible:ring-primary outline-none"
					onclick={close}
					aria-label="Close dialog"
				>
					<span class="material-symbols-outlined">close</span>
				</Dialog.Close>

				{#if pairingService.step === 'input'}
					<div class="flex flex-col gap-2 pb-6 pt-2">
						<Dialog.Title class="text-2xl font-semibold text-on-surface"
							>Pair New Device</Dialog.Title
						>
						<Dialog.Description class="text-sm text-on-surface-variant">
							Enter the pairing details shown on your device's screen.
						</Dialog.Description>
					</div>

					<div class="flex flex-col gap-4">
						<div class="flex flex-col gap-2">
							<label for="mac" class="pl-1 text-sm font-bold text-on-surface">Device ID/MAC</label>
							<input
								id="mac"
								bind:value={macAddress}
								placeholder="ABC123DEF456"
								class="h-12 rounded-xl border border-outline/20 bg-surface-variant px-4 py-3 text-on-surface focus:border-primary focus:outline-none focus-visible:ring-2 focus-visible:ring-primary focus-visible:ring-offset-2"
							/>
						</div>

						<div class="flex flex-col gap-2">
							<label for="pin" class="pl-1 text-sm font-bold text-on-surface">6-Digit PIN</label>
							<input
								id="pin"
								bind:value={pin}
								maxlength="6"
								placeholder="000000"
								class="h-16 rounded-xl border border-outline/20 bg-surface-variant px-4 py-3 text-center text-2xl font-bold tracking-widest text-on-surface focus:border-primary focus:outline-none focus-visible:ring-2 focus-visible:ring-primary focus-visible:ring-offset-2"
							/>
						</div>

						{#if pairingService.error}
							<p class="pl-1 text-xs font-bold text-error">
								{pairingService.error}
							</p>
						{/if}

						<div class="pt-4 flex">
							<button
								onclick={startPairing}
								class="flex h-14 w-full cursor-pointer items-center justify-center gap-2 rounded-full bg-primary font-bold text-on-primary shadow-sm outline-none transition-all hover:shadow-md focus-visible:ring-2 focus-visible:ring-primary focus-visible:ring-offset-2 active:shadow-sm disabled:opacity-50 disabled:cursor-not-allowed"
								disabled={!macAddress || pin.length !== 6}
							>
								Pair Device
							</button>
						</div>
					</div>
				{:else if pairingService.step === 'pairing'}
					<div class="flex flex-col items-center gap-8 py-12 text-center">
						<div class="relative flex h-20 w-20 items-center justify-center">
							<div
								class="absolute h-full w-full animate-spin rounded-full border-4 border-primary/20 border-t-primary"
							></div>
							<span class="material-symbols-outlined text-4xl text-primary">sync</span>
						</div>
						<div class="flex flex-col gap-2">
							<h2 class="text-xl font-bold text-on-surface">Connecting...</h2>
							<p class="px-8 text-sm text-on-surface-variant">
								Establishing secure connection with device <strong>{macAddress}</strong>. This may
								take a few seconds.
							</p>
						</div>
					</div>
				{:else if pairingService.step === 'success'}
					<div class="flex flex-col items-center gap-6 py-8 text-center">
						<div class="flex h-20 w-20 items-center justify-center rounded-full bg-success/10">
							<span class="material-symbols-outlined text-5xl text-success">check_circle</span>
						</div>
						<div class="flex flex-col gap-2">
							<h2 class="text-2xl font-bold text-on-surface">Device Paired!</h2>
							<p class="text-sm text-on-surface-variant">
								Your watering system is now linked and ready to go.
							</p>
						</div>
						<div class="w-full pt-4 flex">
							<button
								onclick={close}
								class="h-14 w-full cursor-pointer rounded-full bg-primary font-bold text-on-primary shadow-sm outline-none transition-all hover:shadow-md focus-visible:ring-2 focus-visible:ring-primary focus-visible:ring-offset-2 active:shadow-sm"
							>
								Go to Dashboard
							</button>
						</div>
					</div>
				{:else if pairingService.step === 'error'}
					<div class="flex flex-col items-center gap-6 py-8 text-center text-error">
						<div class="flex h-20 w-20 items-center justify-center rounded-full bg-error/10">
							<span class="material-symbols-outlined text-5xl">warning</span>
						</div>
						<div class="flex flex-col gap-2">
							<h2 class="text-xl font-bold text-on-surface">Pairing Failed</h2>
							<p class="px-4 text-sm text-on-surface-variant">
								{pairingService.error || 'Something went wrong while connecting to the device.'}
							</p>
						</div>
						<div class="w-full pt-4 flex">
							<button
								onclick={() => (pairingService.step = 'input')}
								class="h-14 w-full cursor-pointer rounded-full bg-primary font-bold text-on-primary shadow-sm outline-none transition-all hover:shadow-md focus-visible:ring-2 focus-visible:ring-primary focus-visible:ring-offset-2 active:shadow-sm"
							>
								Try Again
							</button>
						</div>
					</div>
				{/if}
			</div>
		</Dialog.Content>
	</Dialog.Portal>
</Dialog.Root>
