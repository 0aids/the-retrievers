import { type GPSState } from "../types/state";

const stateEl = document.getElementById("gps-state");
const fixEl = document.getElementById("fix-status");

const gpsPositionEl = document.getElementById("gps-position");
const gpsAltEl = document.getElementById("gps-altitude");
const gpsSpeedEl = document.getElementById("gps-speed");
const gpsSatEl = document.getElementById("gps-satelites");

export function updateGPSStatus(gps: GPSState) {
    if (!stateEl) return;

    if (!gps.positionValid) {
        stateEl.textContent = "No Fix";
        return;
    }

    stateEl.textContent = "Fix";

    if (fixEl && fixEl.style.backgroundColor !== "green") {
        fixEl.style.backgroundColor = "green";
    }

    if (!gpsPositionEl || !gpsAltEl || !gpsSpeedEl || !gpsSatEl) {
        return;
    }

    gpsPositionEl.textContent = `(${gps.latitude}, ${gps.longitude})`;
    gpsAltEl.textContent = `${gps.altitude}`;
    gpsSpeedEl.textContent = `${gps.speedKph}`;
    gpsSatEl.textContent = `In View: ${gps.satsInView}, Tracked: ${gps.satellitesTracked}`;
}
