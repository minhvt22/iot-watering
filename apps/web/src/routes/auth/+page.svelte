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
		class="w-full max-w-md overflow-hidden rounded-3xl border p-8 transition-colors duration-300"
		style="
			background-color: var(--color-bg-surface);
			border-color: var(--color-border);
			box-shadow: var(--shadow-soft);
		"
	>
		<div class="mb-8 text-center">
			<div
				class="mx-auto mb-4 flex h-16 w-16 items-center justify-center rounded-full"
				style="background-color: var(--color-primary-muted);"
			>
				<span class="material-symbols-outlined text-4xl" style="color: var(--color-primary);">
					water_drop
				</span>
			</div>
			<h1 class="text-2xl font-bold tracking-tight" style="color: var(--color-text-primary);">
				{isRegister ? 'Create an account' : 'Welcome back'}
			</h1>
			<p class="mt-2 text-sm" style="color: var(--color-text-muted);">
				{isRegister ? 'Sign up to manage your smart planters.' : 'Sign in to monitor your plants.'}
			</p>
		</div>

		<form
			onsubmit={(e) => {
				e.preventDefault();
				handleSubmit();
			}}
			class="space-y-5"
		>
			{#if errorMsg}
				<div
					class="rounded-xl p-4 text-sm font-medium"
					style="color: var(--color-error); background-color: color-mix(in srgb, var(--color-error) 10%, transparent);"
				>
					{errorMsg}
				</div>
			{/if}

			<div class="space-y-1">
				<label
					for="email"
					class="text-sm font-bold block"
					style="color: var(--color-text-secondary);"
				>
					Email
				</label>
				<input
					id="email"
					name="email"
					type="email"
					required
					bind:value={email}
					disabled={loading}
					class="w-full rounded-xl border p-3 text-sm outline-none transition-colors"
					style="
						background-color: var(--color-bg-base);
						border-color: var(--color-border);
						color: var(--color-text-primary);
					"
					onfocus={(e) => (e.currentTarget.style.borderColor = 'var(--color-primary)')}
					onblur={(e) => (e.currentTarget.style.borderColor = 'var(--color-border)')}
				/>
			</div>

			<div class="space-y-1">
				<label
					for="password"
					class="text-sm font-bold block"
					style="color: var(--color-text-secondary);"
				>
					Password
				</label>
				<input
					id="password"
					name="password"
					type="password"
					required
					bind:value={password}
					disabled={loading}
					class="w-full rounded-xl border p-3 text-sm outline-none transition-colors"
					style="
						background-color: var(--color-bg-base);
						border-color: var(--color-border);
						color: var(--color-text-primary);
					"
					onfocus={(e) => (e.currentTarget.style.borderColor = 'var(--color-primary)')}
					onblur={(e) => (e.currentTarget.style.borderColor = 'var(--color-border)')}
				/>
			</div>

			<button
				type="submit"
				disabled={loading}
				class="mt-2 flex h-12 w-full cursor-pointer items-center justify-center gap-2 rounded-xl text-base font-bold transition-all duration-200 active:scale-95 disabled:cursor-not-allowed disabled:opacity-50"
				style="
					background-color: var(--color-primary);
					color: var(--color-text-on-primary);
					box-shadow: var(--shadow-m3-2);
				"
				onmouseenter={(e) => {
					if (!loading)
						(e.currentTarget as HTMLElement).style.backgroundColor = 'var(--color-primary-hover)';
				}}
				onmouseleave={(e) => {
					(e.currentTarget as HTMLElement).style.backgroundColor = 'var(--color-primary)';
				}}
			>
				{#if loading}
					<span class="material-symbols-outlined animate-spin font-bold">progress_activity</span>
					Processing...
				{:else}
					{isRegister ? 'Sign Up' : 'Sign In'}
				{/if}
			</button>
		</form>

		<div class="mt-8 text-center text-sm" style="color: var(--color-text-muted);">
			{#if isRegister}
				Already have an account?
				<button
					onclick={() => {
						isRegister = false;
						errorMsg = '';
					}}
					class="cursor-pointer font-bold underline transition-colors focus:outline-none"
					style="color: var(--color-primary);"
					onmouseenter={(e) => (e.currentTarget.style.color = 'var(--color-primary-hover)')}
					onmouseleave={(e) => (e.currentTarget.style.color = 'var(--color-primary)')}
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
					class="cursor-pointer font-bold underline transition-colors focus:outline-none"
					style="color: var(--color-primary);"
					onmouseenter={(e) => (e.currentTarget.style.color = 'var(--color-primary-hover)')}
					onmouseleave={(e) => (e.currentTarget.style.color = 'var(--color-primary)')}
				>
					Sign up
				</button>
			{/if}
		</div>
	</div>
</div>
