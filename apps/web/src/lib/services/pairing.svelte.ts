import { supabase } from '$lib/supabase/client';

export class PairingService {
	// --- Pairing State ---
	step = $state<'input' | 'pairing' | 'success' | 'error'>('input');
	error = $state<string | null>(null);

	async startPairing(macAddress: string, pin: string, onDone: () => Promise<void>) {
		this.step = 'pairing';
		this.error = null;

		try {
			const cleanMac = macAddress.toUpperCase().replace(/[:\-]/g, '');

			// Invoke Edge Function for secure session creation (Step is next in plan)
			const { data, error: invokeError } = await supabase.functions.invoke(
				'create-pairing-session',
				{
					body: { mac_address: cleanMac, pin: pin }
				}
			);

			if (invokeError) throw invokeError;

			// Subscribe for this pairing status
			const pairingChannel = supabase
				.channel(`pairing`)

				.on(
					'postgres_changes',
					{
						event: 'UPDATE',
						schema: 'public',
						table: 'device_pairing_sessions',
						filter: `mac_address=eq.${cleanMac}`
					},
					async (payload) => {
						if (payload.new.is_claimed) {
							this.step = 'success';
							supabase.removeChannel(pairingChannel);
							await onDone(); // Refresh global app state
						}
					}
				)
				.subscribe();

			// Timeout after 1 minute
			setTimeout(() => {
				if (this.step === 'pairing') {
					this.step = 'error';
					this.error = 'Pairing timed out. Ensure your device is online and showing the PIN.';
					supabase.removeChannel(pairingChannel);
				}
			}, 60000);
		} catch (e: any) {
			this.step = 'error';
			this.error = e.message;
		}
	}

	reset() {
		this.step = 'input';
		this.error = null;
	}
}

export const pairingService = new PairingService();
