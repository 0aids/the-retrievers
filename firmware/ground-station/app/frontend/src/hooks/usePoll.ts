import { useEffect, useRef } from "react";

import { fetchState } from "../services/api";
import type { ApiState } from "../types/state";

export function usePoll(
    setState: (s: ApiState | null) => void,
    interval = 1000,
) {
    const mounted = useRef(true);

    useEffect(() => {
        mounted.current = true;

        async function poll() {
            const newState = await fetchState();
            if (!mounted.current) return;
            setState(newState);
        }

        poll();

        const id = window.setInterval(poll, interval);

        return () => {
            mounted.current = false;
            clearInterval(id);
        };
    }, [setState, interval]);
}
