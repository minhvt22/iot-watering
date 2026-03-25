import { serve } from "https://deno.land/std@0.168.0/http/server.ts"
import { corsHeaders } from "../_shared/cors.ts"
import { getSupabaseClient } from "../_shared/supabase.ts"

serve(async (req) => {
  if (req.method === 'OPTIONS') {
    return new Response('ok', { headers: corsHeaders })
  }

  console.log(`[create-pairing-session] Received ${req.method} request`);

  try {
    const supabaseClient = getSupabaseClient()

    // 1. Validate Authorization
    const authHeader = req.headers.get('Authorization') || req.headers.get('authorization')
    if (!authHeader) {
      console.error("[create-pairing-session] Missing Authorization header. Headers received:", 
        JSON.stringify(Object.fromEntries(req.headers.entries())));
      return new Response(JSON.stringify({ error: 'Missing Authorization header' }), {
        headers: { ...corsHeaders, 'Content-Type': 'application/json' },
        status: 401,
      })
    }

    const token = authHeader.replace(/^[Bb]earer\s+/, '');
    const { data: { user }, error: userError } = await supabaseClient.auth.getUser(token)
    
    if (userError || !user) {
      console.error("[create-pairing-session] Auth failed for token:", token.slice(0, 10) + "...", "Error:", userError?.message);
      return new Response(JSON.stringify({ error: 'Unauthorized', details: userError?.message }), {
        headers: { ...corsHeaders, 'Content-Type': 'application/json' },
        status: 401,
      })
    }


    // 2. Parsed Body
    const { mac_address, pin } = await req.json().catch(() => ({}));
    if (!mac_address || !pin) {
      return new Response(JSON.stringify({ error: 'Missing mac_address or pin' }), {
        headers: { ...corsHeaders, 'Content-Type': 'application/json' },
        status: 400,
      })
    }

    const cleanMac = mac_address.toUpperCase().replace(/[:\-]/g, '');

    // 3. Invalidate old pending sessions for this MAC to prevent "ghost" triggers
    await supabaseClient
      .from('device_pairing_sessions')
      .update({ expires_at: new Date().toISOString() })
      .eq('mac_address', cleanMac)
      .is('is_claimed', false);

    // 4. Create the pairing session (15m validity)
    const { data, error } = await supabaseClient
      .from('device_pairing_sessions')
      .insert({
        mac_address: cleanMac,
        pin: pin,
        owner_id: user.id,
        expires_at: new Date(Date.now() + 15 * 60 * 1000).toISOString()
      })
      .select()
      .single()

    if (error) {
      console.error("[create-pairing-session] Insert failed:", error.message);
      throw error;
    }

    console.log(`[create-pairing-session] Session ${data.id} created for MAC ${cleanMac}`);

    return new Response(JSON.stringify(data), {
      headers: { ...corsHeaders, 'Content-Type': 'application/json' },
      status: 200,
    })

  } catch (error: any) {
    console.error("[create-pairing-session] Error:", error.message);
    return new Response(JSON.stringify({ error: error.message }), {
      headers: { ...corsHeaders, 'Content-Type': 'application/json' },
      status: 400,
    })
  }
})
