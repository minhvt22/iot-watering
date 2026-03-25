<script lang="ts">
	import { goto } from '$app/navigation';
	import { supabase } from '$lib/supabase/client';

	let email = $state('');
	let password = $state('');
	let loading = $state(false);
	let isRegister = $state(false);
	let errorMsg = $state('');

	async function handleSubmit() {
		loading = true;
		errorMsg = '';

		if (isRegister) {
			const { error } = await supabase.auth.signUp({
				email,
				password
			});
			if (error) {
				errorMsg = error.message;
			} else {
				errorMsg = 'Check your email for the confirmation link.';
			}
		} else {
			const { error } = await supabase.auth.signInWithPassword({
				email,
				password
			});
			if (error) {
				errorMsg = error.message;
			} else {
				goto('/');
			}
		}
		loading = false;
	}
</script>

<div class="flex min-h-dvh w-full items-center justify-center p-4">
	<div
		class="flex w-full max-w-md flex-col gap-8 overflow-hidden rounded-3xl border border-outline-variant bg-surface p-8 shadow-sm transition-colors duration-300"
	>
		<div class="flex flex-col items-center text-center gap-4">
			<div
				class="flex h-16 w-16 items-center justify-center rounded-full bg-primary-container text-primary"
			>
				<span class="material-symbols-outlined text-4xl">water_drop</span>
			</div>
			<div class="flex flex-col gap-2">
				<h1 class="text-2xl font-bold tracking-tight text-on-surface">
					{isRegister ? 'Create an account' : 'Welcome back'}
				</h1>
				<p class="text-sm text-on-surface-variant">
					{isRegister
						? 'Sign up to manage your smart planters.'
						: 'Sign in to monitor your plants.'}
				</p>
			</div>
		</div>

		<form
			onsubmit={(e) => {
				e.preventDefault();
				handleSubmit();
			}}
			class="flex flex-col gap-6"
		>
			{#if errorMsg}
				<div class="rounded-xl bg-error/10 p-4 text-sm font-bold text-error">
					{errorMsg}
				</div>
			{/if}

			<div class="flex flex-col gap-2">
				<label for="email" class="block text-sm font-bold text-on-surface-variant"> Email </label>
				<input
					id="email"
					name="email"
					type="email"
					required
					bind:value={email}
					disabled={loading}
					class="h-12 w-full rounded-xl border border-outline bg-surface p-3 text-sm text-on-surface outline-none transition-colors focus-visible:border-primary disabled:opacity-50"
				/>
			</div>

			<div class="flex flex-col gap-2">
				<label for="password" class="block text-sm font-bold text-on-surface-variant">
					Password
				</label>
				<input
					id="password"
					name="password"
					type="password"
					required
					bind:value={password}
					disabled={loading}
					class="h-12 w-full rounded-xl border border-outline bg-surface p-3 text-sm text-on-surface outline-none transition-colors focus-visible:border-primary disabled:opacity-50"
				/>
			</div>

			<button
				type="submit"
				disabled={loading}
				class="flex h-14 w-full cursor-pointer items-center justify-center gap-2 rounded-xl bg-primary text-base font-bold text-on-primary shadow-sm outline-none transition-colors duration-200 hover:bg-primary-hover focus-visible:ring-2 focus-visible:ring-primary focus-visible:ring-offset-2 disabled:cursor-not-allowed disabled:opacity-50"
			>
				{#if loading}
					<span class="material-symbols-outlined animate-spin font-bold">progress_activity</span>
					Processing...
				{:else}
					{isRegister ? 'Sign Up' : 'Sign In'}
				{/if}
			</button>
		</form>

		<div class="text-center text-sm text-on-surface-variant">
			{#if isRegister}
				Already have an account?
				<button
					onclick={() => {
						isRegister = false;
						errorMsg = '';
					}}
					class="cursor-pointer font-bold text-primary underline outline-none transition-colors hover:text-primary-hover focus-visible:ring-2 focus-visible:ring-primary focus-visible:ring-offset-2"
				>
					Sign in
				</button>
			{:else}
				Don't have an account?
				<button
					onclick={() => {
						isRegister = true;
						errorMsg = '';
					}}
					class="cursor-pointer font-bold text-primary underline outline-none transition-colors hover:text-primary-hover focus-visible:ring-2 focus-visible:ring-primary focus-visible:ring-offset-2"
				>
					Sign up
				</button>
			{/if}
		</div>
	</div>
</div>
