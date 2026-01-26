import "./main.css";
import "./map/map";

import { updateFSM } from "./ui/fsm";
import { userPos, panToMe } from "./map/user";
import { fetchState, buzzer2500 } from "./api/state";
import { updateGPSStatus } from "./ui/gps";
import { updateLoraStatus } from "./ui/lora";
import { updateDistance } from "./map/distance";
import { updatePsat, toggleFollowPSAT, togglePath } from "./map/psat";

let showPath = true;

async function poll() {
    const state = await fetchState();
    if (!state) return;

    updateFSM(state.fsm);
    updatePsat(state.gps);
    updateGPSStatus(state.gps);
    updateLoraStatus(
        state.radio.last_packet_time,
        state.stats.packets_recieved,
    );

    if (userPos && state.gps.latitude && state.gps.longitude) {
        updateDistance(userPos, {
            lat: state.gps.latitude,
            lon: state.gps.longitude,
        });
    }
}

setInterval(poll, 1000);

const togglePathBtn = document.getElementById("toggle-path");
const followPsatBtn = document.getElementById("follow-psat");
const followMeBtn = document.getElementById("follow-me");
const beepBtn = document.getElementById("beep-2500-button");

togglePathBtn?.addEventListener("click", () => {
    showPath = !showPath;
    togglePath(showPath);
    togglePathBtn.classList.toggle("gs-btn-active", showPath);
});

followPsatBtn?.addEventListener("click", () => {
    toggleFollowPSAT();
    followPsatBtn.classList.toggle("gs-btn-active");
});

followMeBtn?.addEventListener("click", () => {
    panToMe();
    followMeBtn.classList.add("gs-btn-active");
    setTimeout(() => followMeBtn.classList.remove("gs-btn-active"), 700);
});

beepBtn?.addEventListener("click", () => {
    beepBtn.setAttribute("disabled", "true");
    beepBtn.classList.add("opacity-60");

    buzzer2500();

    setTimeout(() => {
        beepBtn.removeAttribute("disabled");
        beepBtn.classList.remove("opacity-60");
    }, 1500);
});
