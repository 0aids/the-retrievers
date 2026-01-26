import "./main.css";
import "./map/map";

import { updateFSM } from "./ui/fsm";
import { userPos, panToMe } from "./map/user";
import { fetchState, buzzer2500 } from "./api/state";
import { updateGPSStatus } from "./ui/gps";
import { updateDistance } from "./map/distance";
import { updatePsat, toggleFollowPSAT, togglePath } from "./map/psat";

let showPath = true;

async function poll() {
    const state = await fetchState();
    if (!state) return;

    updateFSM(state.fsm);
    updatePsat(state.gps);
    updateGPSStatus(state.gps);

    if (userPos && state.gps.latitude && state.gps.longitude) {
        updateDistance(userPos, {
            lat: state.gps.latitude,
            lon: state.gps.longitude,
        });
    }
}

setInterval(poll, 1000);

document.getElementById("toggle-path")?.addEventListener("click", () => {
    showPath = !showPath;
    togglePath(showPath);
});

document.getElementById("follow-psat")?.addEventListener("click", () => {
    toggleFollowPSAT();
});

document.getElementById("follow-me")?.addEventListener("click", () => {
    panToMe();
});

document.getElementById("beep-2500-button")?.addEventListener("click", () => {
    alert("Sending buzz request");
    buzzer2500();
});
