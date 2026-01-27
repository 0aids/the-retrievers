import { type FSMState } from "../types/state";

const el = document.getElementById("fsm-state");

export function updateFSM(fsm: FSMState) {
    if (!el) return;
    el.textContent = (fsm?.name ?? "Unknown")
        .replace("psatFSM_state_", "")
        .toUpperCase();
}
