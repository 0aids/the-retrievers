import * as L from "leaflet";
import { map } from "./map";
import type { GPSState } from "../types/state";

let psatCircle: L.Circle | undefined;
let followPsat = false;

const trail: L.LatLngExpression[] = [];
const polyline = L.polyline(trail, { color: "red" });
const followEl = document.getElementById("follow-psat");

export function togglePath(show: boolean) {
    show ? polyline.addTo(map) : polyline.remove();
}

export function toggleFollowPSAT() {
    followPsat = !followPsat;

    if (followEl) {
        followEl.textContent = followPsat ? "Follow PSAT" : "Unfollow PSAT";
    }
}

export function updatePsat(gps: GPSState) {
    if (!gps.latitude || !gps.longitude) return;

    const pos: L.LatLngExpression = [gps.latitude, gps.longitude];

    if (!psatCircle) {
        psatCircle = L.circle(pos, {
            color: "red",
            fillColor: "#d93939",
            fillOpacity: 0.5,
            radius: 3,
        }).addTo(map);
        polyline.addTo(map);
    } else {
        psatCircle.setLatLng(pos);
    }

    trail.push(pos);
    if (trail.length > 100) trail.shift();
    polyline.setLatLngs(trail);

    if (followPsat) map.panTo(pos);
}
