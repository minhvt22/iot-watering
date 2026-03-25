// ===========================================================
// Database Type Definitions
// Generated manually to match the proposed schema.
// Run: supabase gen types typescript --local > types.ts
// once Supabase project is linked.
// ===========================================================

export type Json = string | number | boolean | null | { [key: string]: Json } | Json[];

export interface Database {
	public: {
		Tables: {
			devices: {
				Row: {
					id: string;
					name: string;
					owner_id: string;
					online: boolean;
					last_seen: string | null;
					auto_water_threshold: number;
					pump_duration_seconds: number;
					created_at: string;
				};
				Insert: {
					id?: string;
					name: string;
					owner_id: string;
					online?: boolean;
					last_seen?: string | null;
					auto_water_threshold?: number;
					pump_duration_seconds?: number;
					created_at?: string;
				};
				Update: {
					id?: string;
					name?: string;
					owner_id?: string;
					online?: boolean;
					last_seen?: string | null;
					auto_water_threshold?: number;
					pump_duration_seconds?: number;
					created_at?: string;
				};
				Relationships: never[];
			};
			telemetry: {
				Row: {
					id: number;
					device_id: string;
					moisture: number;
					temperature: number | null;
					recorded_at: string;
				};
				Insert: {
					id?: number;
					device_id: string;
					moisture: number;
					temperature?: number | null;
					recorded_at?: string;
				};
				Update: {
					id?: number;
					device_id?: string;
					moisture?: number;
					temperature?: number | null;
					recorded_at?: string;
				};
				Relationships: never[];
			};
			commands: {
				Row: {
					id: number;
					device_id: string;
					command: 'pump_on' | 'pump_off';
					duration_seconds: number | null;
					status: Database['public']['Enums']['command_status'];
					issued_at: string;
					completed_at: string | null;
				};
				Insert: {
					id?: number;
					device_id: string;
					command: 'pump_on' | 'pump_off';
					duration_seconds?: number | null;
					status?: Database['public']['Enums']['command_status'];
					issued_at?: string;
					completed_at?: string | null;
				};
				Update: {
					id?: number;
					device_id?: string;
					command?: 'pump_on' | 'pump_off';
					duration_seconds?: number | null;
					status?: Database['public']['Enums']['command_status'];
					issued_at?: string;
					completed_at?: string | null;
				};
				Relationships: never[];
			};
			device_claims: {
				Row: {
					id: string;
					owner_id: string;
					claim_token: string;
					created_at: string;
					expires_at: string;
				};
				Insert: {
					id?: string;
					owner_id: string;
					claim_token: string;
					created_at?: string;
					expires_at?: string;
				};
				Update: {
					id?: string;
					owner_id?: string;
					claim_token?: string;
					created_at?: string;
					expires_at?: string;
				};
				Relationships: never[];
			};
			device_pairing_sessions: {
				Row: {
					id: string;
					mac_address: string;
					pin: string;
					owner_id: string;
					is_claimed: boolean;
					device_id: string | null;
					created_at: string;
				};
				Insert: {
					id?: string;
					mac_address: string;
					pin: string;
					owner_id: string;
					is_claimed?: boolean;
					device_id?: string | null;
					created_at?: string;
				};
				Update: {
					id?: string;
					mac_address?: string;
					pin?: string;
					owner_id?: string;
					is_claimed?: boolean;
					device_id?: string | null;
					created_at?: string;
				};
				Relationships: never[];
			};
		};
		Views: Record<string, never>;
		Functions: Record<string, never>;
		Enums: {
			command_status: 'pending' | 'delivered' | 'completed' | 'failed';
		};
	};
}

// ─── Convenience aliases ────────────────────────────────────
export type DeviceRow = Database['public']['Tables']['devices']['Row'];
export type TelemetryRow = Database['public']['Tables']['telemetry']['Row'];
export type CommandRow = Database['public']['Tables']['commands']['Row'];
export type DeviceClaimRow = Database['public']['Tables']['device_claims']['Row'];
