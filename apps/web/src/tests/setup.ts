import '@testing-library/jest-dom';
import { vi } from 'vitest';

// Mock Supabase to prevent real network calls during testing
vi.mock('@supabase/supabase-js', () => ({
	createClient: vi.fn(() => ({
		from: vi.fn(() => ({
			select: vi.fn(),
			insert: vi.fn(),
			update: vi.fn(),
			upsert: vi.fn(),
			on: vi.fn()
		})),
		auth: {
			getSession: vi.fn(),
			onAuthStateChange: vi.fn()
		}
	}))
}));
