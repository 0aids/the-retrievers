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

export async function sendCommand(cmd: number, args: number | null = null) {
    try {
        const res = await fetch(`${BASE}/command`, {
            method: "POST",
            body: JSON.stringify({
                cmd,
                args,
            }),
            headers: {
                Accept: "application/json",
                "Content-Type": "application/json",
            },
        });
        if (!res.ok) return null;
        return await res.json();
    } catch (e) {
        console.error("error while sending command :(", e);
        return null;
    }
}
