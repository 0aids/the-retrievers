import * as L from "leaflet";
import { map } from "./map";
import type { GPSState } from "../types/state";
import { userPos } from "./user";

let psatCircle: L.Circle | undefined;
let followPsat = false;

export let psatPos: { lat: number; lon: number } | undefined;

const trail: L.LatLngExpression[] = [];
const psatToMeTrail: [L.LatLngExpression, L.LatLngExpression] = [
    [0, 0],
    [0, 0],
];

const polyline = L.polyline(trail, { color: "red" });
const pathToPSAT = L.polyline(psatToMeTrail, {
    color: "#0ecf45",
    stroke: true,
    weight: 2,
    opacity: 10,
    dashArray: "5",
});

const followEl = document.getElementById("follow-psat");

export function togglePath(show: boolean) {
    show ? polyline.addTo(map) : polyline.remove();
    show ? pathToPSAT.addTo(map) : pathToPSAT.remove();
}

export function toggleFollowPSAT() {
    followPsat = !followPsat;

    if (followEl) {
        followEl.textContent = followPsat ? "Unfollow PSAT" : "Follow PSAT";
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
        pathToPSAT.addTo(map);
        console.log(pathToPSAT);
    } else {
        psatCircle.setLatLng(pos);
    }

    psatPos = { lat: pos[0], lon: pos[1] };

    trail.push(pos);
    if (trail.length > 100) trail.shift();
    polyline.setLatLngs(trail);

    if (userPos) {
        psatToMeTrail[0] = pos;
        psatToMeTrail[1] = [userPos.lat, userPos.lon];
        pathToPSAT.setLatLngs(psatToMeTrail);
    }

    if (followPsat)
        map.panTo(pos, {
            animate: true,
            duration: 0.6,
        });
}
