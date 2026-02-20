import type { ApiState } from "../types/state";

const BASE = "/api";

export async function fetchState(): Promise<ApiState | null> {
    try {
        const res = await fetch(`${BASE}/state`);
        if (!res.ok) return null;
        return (await res.json()) as ApiState;
    } catch (e) {
        console.error("error while fetching the state :(", e);
        return null;
    }
}
