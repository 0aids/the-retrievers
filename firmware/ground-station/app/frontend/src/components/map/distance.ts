import L from "leaflet";
import { useMap } from "react-leaflet";
import { useRef, useEffect } from "react";

export const calculateDistance = (
    lat1: number,
    lon1: number,
    lat2: number,
    lon2: number,
) => {
    // stolen from https://www.movable-type.co.uk/scripts/latlong.html
    // this usage of unicode characters is crazy but im lazy to change it

    const R = 6371e3; // metres
    const φ1 = (lat1 * Math.PI) / 180; // φ, λ in radians
    const φ2 = (lat2 * Math.PI) / 180;
    const Δφ = ((lat2 - lat1) * Math.PI) / 180;
    const Δλ = ((lon2 - lon1) * Math.PI) / 180;

    const a =
        Math.sin(Δφ / 2) * Math.sin(Δφ / 2) +
        Math.cos(φ1) * Math.cos(φ2) * Math.sin(Δλ / 2) * Math.sin(Δλ / 2);
    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));

    const d = R * c; // in metres

    return d;
};

export function DistanceControl({
    initial = "Unknown",
    position = "topright",
    value,
}: {
    initial?: string;
    position?: L.ControlPosition;
    value?: string;
}) {
    const map = useMap();
    const controlRef = useRef<L.Control | null>(null);

    useEffect(() => {
        const DistanceBox = L.Control.extend({
            onAdd() {
                const div = L.DomUtil.create("div");
                div.innerHTML = `
          <div class="bg-gray-800 border border-gray-700 rounded-lg px-3 py-1 flex flex-col items-center shadow-lg">
            <span class="text-[0.6rem] text-gray-400 uppercase">Distance:</span>
            <p class="text-sm font-bold">${initial}</p>
          </div>`;
                return div;
            },
        });

        const ctrl = new DistanceBox({ position });
        ctrl.addTo(map);
        controlRef.current = ctrl;

        return () => {
            ctrl.remove();
            controlRef.current = null;
        };
    }, [map, position]);

    useEffect(() => {
        const container = controlRef.current?.getContainer();
        if (!container) return;
        const p = container.querySelector("p");
        if (p && value !== undefined) p.textContent = value;
    }, [value]);

    return null;
}
