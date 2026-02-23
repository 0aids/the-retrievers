import { useEffect, useState, useRef } from "react";

import type { GeoPosition } from "../types/state";

export function useWatchPosition(onFirstFix: (pos: GeoPosition) => void) {
    const firstFix = useRef(true);
    const [position, setPosition] = useState<GeoPosition | null>(null);

    useEffect(() => {
        if (!navigator.geolocation) return;

        const watchId = navigator.geolocation.watchPosition(
            (pos) => {
                const position = {
                    lat: pos.coords.latitude,
                    lon: pos.coords.longitude,
                    accuracy: pos.coords.accuracy,
                };

                setPosition(position);

                if (firstFix.current) {
                    firstFix.current = false;
                    onFirstFix(position);
                }
            },
            console.error,
            { enableHighAccuracy: true, maximumAge: 0 },
        );

        return () => navigator.geolocation.clearWatch(watchId);
    }, [onFirstFix]);

    return position;
}
