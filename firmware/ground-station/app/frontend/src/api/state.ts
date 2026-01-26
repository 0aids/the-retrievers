import { type ApiState } from "../types/state";

export async function fetchState(): Promise<ApiState | null> {
    try {
        const res = await fetch("/api/state");
        return await res.json();
    } catch {
        return null;
    }
}
