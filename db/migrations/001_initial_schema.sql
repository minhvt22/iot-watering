-- ======================================================
-- water-sys schema migrations (Optimized)
-- Apply via the Supabase SQL Editor
-- ======================================================
-- ─── Extensions ────────────────────────────────────────────
create extension if not exists "uuid-ossp";
-- ─── Custom Types ──────────────────────────────────────────
create type public.command_status as enum ('pending', 'delivered', 'completed', 'failed');
-- ─── 1. Devices ────────────────────────────────────────────
create table if not exists public.devices (
  id uuid primary key default uuid_generate_v4(),
  owner_id uuid not null references auth.users(id) on delete cascade,
  name text not null default 'My Plant',
  online boolean not null default false,
  last_seen timestamptz,
  auto_water_threshold integer not null default 30 check (
    auto_water_threshold between 0 and 100
  ),
  pump_duration_seconds integer not null default 15 check (
    pump_duration_seconds between 5 and 120
  ),
  created_at timestamptz not null default now()
);
-- Crucial Index for RLS Performance: RLS frequently checks owner_id
create index if not exists idx_devices_owner on public.devices (owner_id);
-- RLS
alter table public.devices enable row level security;
create policy "Owner can manage their devices" on public.devices for all using (auth.uid() = owner_id);
-- ─── 2. Telemetry ──────────────────────────────────────────
create table if not exists public.telemetry (
  id bigint generated always as identity primary key,
  device_id uuid not null references public.devices(id) on delete cascade,
  -- Optimized from numeric(5,2) to real (4 bytes vs up to 8 bytes, better for millions of IoT events)
  moisture real not null check (
    moisture between 0 and 100
  ),
  temperature real,
  recorded_at timestamptz not null default now()
);
-- Composite B-Tree index for finding the N most recent readings for a specific device quickly
create index if not exists idx_telemetry_device_time on public.telemetry (device_id, recorded_at desc);
-- Optional: For massive data at scale, consider a BRIN index on recorded_at instead 
-- if full table scans aggregated by time are common, but for dashboard "latest 48", 
-- the composite B-Tree above is optimal.
-- RLS
alter table public.telemetry enable row level security;
-- Policies
create policy "Owner can read telemetry" on public.telemetry for
select using (
    exists (
      select 1
      from public.devices d
      where d.id = telemetry.device_id
        and d.owner_id = auth.uid()
    )
  );
create policy "Device can insert telemetry (service role)" on public.telemetry for
insert with check (true);
-- ─── 3. Commands ───────────────────────────────────────────
create table if not exists public.commands (
  id bigint generated always as identity primary key,
  device_id uuid not null references public.devices(id) on delete cascade,
  command text not null check (command in ('pump_on', 'pump_off')),
  duration_seconds integer,
  -- Optimized from brittle boolean 'acknowledged' to a state machine
  status public.command_status not null default 'pending',
  issued_at timestamptz not null default now(),
  completed_at timestamptz
);
-- Index for ESP32 polling efficiency
create index if not exists idx_commands_pending on public.commands (device_id, status, issued_at desc)
where status = 'pending';
-- RLS
alter table public.commands enable row level security;
create policy "Owner can issue commands" on public.commands for
insert with check (
    exists (
      select 1
      from public.devices d
      where d.id = commands.device_id
        and d.owner_id = auth.uid()
    )
  );
create policy "Owner can read commands" on public.commands for
select using (
    exists (
      select 1
      from public.devices d
      where d.id = commands.device_id
        and d.owner_id = auth.uid()
    )
  );
-- ─── 4. Device Claims ────────────────────────────────────────
create table if not exists public.device_claims (
  id uuid primary key default uuid_generate_v4(),
  owner_id uuid not null references auth.users(id) on delete cascade,
  claim_token text not null unique,
  created_at timestamptz not null default now(),
  expires_at timestamptz not null default now() + interval '1 hour'
);
-- Index for ESP32 and Edge Function lookup
create index if not exists idx_device_claims_token on public.device_claims (claim_token);
-- RLS
alter table public.device_claims enable row level security;
create policy "Owner can manage claims" on public.device_claims for all using (auth.uid() = owner_id);
-- ─── 5. Realtime ───────────────────────────────────────────
alter publication supabase_realtime
add table public.telemetry;
alter publication supabase_realtime
add table public.commands;
alter publication supabase_realtime
add table public.devices;
alter publication supabase_realtime
add table public.device_claims;