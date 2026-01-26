import { type ApiState } from "../types/state";

export async function fetchState(): Promise<ApiState | null> {
    try {
        const res = await fetch("/api/state");
        return await res.json();
    } catch {
        return null;
    }
}

export async function buzzer2500(): Promise<ApiState | null> {
    try {
        const res = await fetch("/api/command", {
            method: "POST",
            body: JSON.stringify({
                cmd: 5,
                args: {},
            }),
            headers: {
                Accept: "application/json",
                "Content-Type": "application/json",
            },
        });
        return await res.json();
    } catch {
        return null;
    }
}
