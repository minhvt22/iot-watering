<script lang="ts">
	import './layout.css';
	import { onMount } from 'svelte';
	import { goto } from '$app/navigation';
	import { supabase } from '$lib/supabase/client';

	import { Tooltip } from 'bits-ui';

	let { children } = $props();

	onMount(() => {
		const {
			data: { subscription }
		} = supabase.auth.onAuthStateChange((event, session) => {
			const isAuthPage = window.location.pathname.startsWith('/auth');
			if (!session && !isAuthPage) {
				goto('/auth');
			} else if (session && isAuthPage) {
				goto('/');
			}
		});

		// Check initial session
		supabase.auth.getSession().then(({ data: { session } }) => {
			const isAuthPage = window.location.pathname.startsWith('/auth');
			if (!session && !isAuthPage) {
				goto('/auth');
			} else if (session && isAuthPage) {
				goto('/');
			}
		});

		return () => subscription.unsubscribe();
	});
</script>

<svelte:head>
	<link
		href="https://fonts.googleapis.com/css2?family=Material+Symbols+Outlined:wght,FILL@100..700,0..1&display=swap"
		rel="stylesheet"
	/>
	<link
		href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap"
		rel="stylesheet"
	/>
</svelte:head>

<Tooltip.Provider>
	{@render children()}
</Tooltip.Provider>
