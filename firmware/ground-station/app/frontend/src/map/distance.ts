import * as L from "leaflet";
import { map } from "./map";

const calculateDistance = (
    lat1: number,
    lon1: number,
    lat2: number,
    lon2: number,
) => {
    // stolen from https://www.movable-type.co.uk/scripts/latlong.html

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

class DistanceBox extends L.Control {
    onAdd() {
        const div = L.DomUtil.create("div");
        div.innerHTML = `
            <div class="bg-gray-800 border border-gray-700 rounded-lg px-3 py-1 flex flex-col items-center shadow-lg">
                <span class="text-[0.6rem] text-gray-400 uppercase">Distance:</span>
                <p class="text-sm font-bold">Unknown</p>
            </div>`;
        return div;
    }

    update(text: string) {
        const el = this.getContainer()?.querySelector("p");
        if (el) el.textContent = text;
    }
}

export const distanceBox = new DistanceBox({ position: "topright" });
distanceBox.addTo(map);

export function updateDistance(
    a?: { lat: number; lon: number },
    b?: { lat: number; lon: number },
) {
    if (!a || !b) return;
    const d = calculateDistance(a.lat, a.lon, b.lat, b.lon);
    distanceBox.update(`${d.toFixed(2)}m`);
}
