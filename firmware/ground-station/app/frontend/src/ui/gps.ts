import { type GPSState } from "../types/state";

const stateEl = document.getElementById("gps-state");
const fixEl = document.getElementById("fix-status");

export function updateGPSStatus(gps: GPSState) {
    if (!stateEl) return;

    if (!gps.positionValid) {
        stateEl.textContent = "No Fix";
        return;
    }

    stateEl.textContent = "Fix";
    if (fixEl) fixEl.style.backgroundColor = "green";
}
