import { serve } from "https://deno.land/std@0.168.0/http/server.ts"
import { corsHeaders } from "../_shared/cors.ts"
import { getSupabaseClient } from "../_shared/supabase.ts"

serve(async (req) => {
  if (req.method === 'OPTIONS') return new Response('ok', { headers: corsHeaders });

  try {
    const supabaseClient = getSupabaseClient();
    const { mac_address, pin } = await req.json().catch(() => ({}));

    // 1. Guard: Validate Input
    if (!mac_address || !pin) {
      return createErrorResponse('Missing MAC or PIN', 400);
    }

    const cleanMac = mac_address.toUpperCase().replace(/[:\-]/g, '');

    // 2. Guard: Validate Pairing Session
    const { data: session, error: sessionError } = await supabaseClient
      .from('device_pairing_sessions')
      .select('*')
      .eq('mac_address', cleanMac)
      .eq('pin', pin)
      .is('is_claimed', false)
      .gt('expires_at', new Date().toISOString())
      .single();

    if (sessionError || !session) {
      return createErrorResponse('Pairing session not found or expired', 404);
    }

    // 3. Guard: Prevent Hijacking (Check current owner)
    const { data: existingDevice } = await supabaseClient
      .from('devices')
      .select('owner_id')
      .eq('mac_address', cleanMac)
      .maybeSingle();

    if (existingDevice && existingDevice.owner_id !== session.owner_id) {
      return createErrorResponse('Device is already claimed by another user. Reset hardware to re-pair.', 403);
    }

    // 4. Action: Initialize/Update Device
    const { data: device, error: deviceError } = await supabaseClient
      .from('devices')
      .upsert({
        owner_id: session.owner_id,
        mac_address: cleanMac,
        name: `Smart Planter (${cleanMac.slice(-4)})`,
        online: true,
        last_seen: new Date().toISOString()
      }, { onConflict: 'mac_address' })
      .select()
      .single();

    if (deviceError) throw deviceError;

    // 5. Action: Finalize Session
    const { error: updateError } = await supabaseClient
      .from('device_pairing_sessions')
      .update({ is_claimed: true, device_id: device.id })
      .eq('id', session.id);

    if (updateError) throw updateError;

    return new Response(JSON.stringify({ device_id: device.id }), {
      headers: { ...corsHeaders, 'Content-Type': 'application/json' },
      status: 200,
    });

  } catch (error: any) {
    console.error("[claim-device] Error:", error.message);
    return createErrorResponse(error.message, 500);
  }
});

function createErrorResponse(message: string, status: number) {
  return new Response(JSON.stringify({ error: message }), {
    headers: { ...corsHeaders, 'Content-Type': 'application/json' },
    status,
  });
}

