-- ======================================================
-- water-sys schema migrations (Merged Base)
-- Apply via the Supabase SQL Editor
-- ======================================================

-- ─── Extensions ────────────────────────────────────────────
create extension if not exists "uuid-ossp";

-- ─── Custom Types ──────────────────────────────────────────
create type public.command_status as enum ('pending', 'delivered', 'completed', 'failed');

-- ─── 0. Profiles (Decoupling from Auth Schema) ─────────────
create table if not exists public.profiles (
  id uuid primary key references auth.users(id) on delete cascade,
  email text,
  updated_at timestamptz default now()
);

-- RLS for Profiles
alter table public.profiles enable row level security;
create policy "Users can read all profiles" on public.profiles for select using (true);
create policy "Users can update own profile" on public.profiles for update using (auth.uid() = id);

-- Trigger for Auto-Profile creation
create or replace function public.handle_new_user()
returns trigger as $$
begin
  insert into public.profiles (id, email)
  values (new.id, new.email);
  return new;
end;
$$ language plpgsql security definer;

create or replace trigger on_auth_user_created
  after insert on auth.users
  for each row execute procedure public.handle_new_user();

-- ─── 1. Devices ────────────────────────────────────────────
create table if not exists public.devices (
  id uuid primary key default uuid_generate_v4(),
  owner_id uuid not null references public.profiles(id) on delete cascade,
  name text not null default 'My Plant',
  mac_address text unique,
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

create index if not exists idx_devices_owner on public.devices (owner_id);

alter table public.devices enable row level security;
create policy "Owner can see own devices" on public.devices for select using (auth.uid() = owner_id);
create policy "Allow anon to check devices" on public.devices for select to anon using (true);
create policy "Owner can update own devices" on public.devices for update using (auth.uid() = owner_id);
create policy "Owner can delete own devices" on public.devices for delete using (auth.uid() = owner_id);

-- ─── 2. Telemetry ──────────────────────────────────────────
create table if not exists public.telemetry (
  id bigint generated always as identity primary key,
  device_id uuid not null references public.devices(id) on delete cascade,
  moisture real not null check (
    moisture between 0 and 100
  ),
  temperature real,
  recorded_at timestamptz not null default now()
);

create index if not exists idx_telemetry_device_time on public.telemetry (device_id, recorded_at desc);

alter table public.telemetry enable row level security;
create policy "Owner can read telemetry" on public.telemetry for
select using (
    exists (
      select 1
      from public.devices d
      where d.id = telemetry.device_id
        and d.owner_id = auth.uid()
    )
  );

-- Allow anonymous devices to insert telemetry only for existing devices
create policy "Insert telemetry (Anon)" on public.telemetry for
insert to anon with check (
    exists (
      select 1
      from public.devices
      where id = telemetry.device_id
    )
  );

-- ─── 3. Commands ───────────────────────────────────────────
create table if not exists public.commands (
  id bigint generated always as identity primary key,
  device_id uuid not null references public.devices(id) on delete cascade,
  command text not null check (command in ('pump_on', 'pump_off')),
  duration_seconds integer,
  status public.command_status not null default 'pending',
  issued_at timestamptz not null default now(),
  completed_at timestamptz
);

create index if not exists idx_commands_pending on public.commands (device_id, status, issued_at desc)
where status = 'pending';

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

-- Device access to commands (restricted to pending status)
create policy "Devices can read pending commands" on public.commands for select to anon using (
    status = 'pending' 
    and exists (select 1 from public.devices where id = commands.device_id)
);

create policy "Devices can update command status" on public.commands for update to anon 
using (
    status in ('pending', 'delivered')
    and exists (select 1 from public.devices where id = commands.device_id)
)
with check (status in ('delivered', 'completed'));

-- ─── 4. Device Pairing Sessions ──────────────────────────────
create table if not exists public.device_pairing_sessions (
  id uuid primary key default uuid_generate_v4(),
  owner_id uuid not null references public.profiles(id) on delete cascade,
  mac_address text not null,
  pin text not null,
  is_claimed boolean not null default false,
  device_id uuid references public.devices(id),
  created_at timestamptz not null default now(),
  expires_at timestamptz not null default now() + interval '1 hour'
);

create index if not exists idx_pairing_lookup on public.device_pairing_sessions (mac_address, pin) where is_claimed = false;

alter table public.device_pairing_sessions enable row level security;
create policy "Owner can manage sessions" on public.device_pairing_sessions for all using (auth.uid() = owner_id);

-- Anonymous pairing check requires MAC + PIN to prevent discovery
create policy "Allow anonymous pairing check" on public.device_pairing_sessions for select to anon 
using (
    is_claimed = false 
    and expires_at > now()
);

-- ─── 5. Grants & Realtime Optimization ─────────────────────
-- Global Schema Usage
grant usage on schema public to anon, authenticated;
grant usage on type public.command_status to anon, authenticated;
grant usage on all sequences in schema public to anon, authenticated;

-- Profiles: Authenticated users need to read/update profiles
grant select, update on table public.profiles to authenticated;
grant select on table public.profiles to anon;

-- Devices: Authenticated users manage them. Anon (devices) syncs settings.
grant select, insert, update, delete on table public.devices to authenticated;
grant select on table public.devices to anon;

-- Telemetry: Anon (devices) inserts. Authenticated reads/deletes.
grant insert on table public.telemetry to anon;
grant select, insert, delete on table public.telemetry to authenticated;

-- Commands: Authenticated issues. Anon (devices) pulls/updates.
grant insert, select, update on table public.commands to anon;
grant select, insert, update, delete on table public.commands to authenticated;

-- Pairing Sessions: Authenticated manages. Anon checks.
grant select, insert, update, delete on table public.device_pairing_sessions to authenticated;
grant select on table public.device_pairing_sessions to anon;

-- Realtime Configuration
alter table public.device_pairing_sessions replica identity full;
alter table public.commands replica identity full;
alter table public.telemetry replica identity full;

alter publication supabase_realtime add table public.telemetry;
alter publication supabase_realtime add table public.commands;
alter publication supabase_realtime add table public.devices;
alter publication supabase_realtime add table public.device_pairing_sessions;
alter table public.devices replica identity full;