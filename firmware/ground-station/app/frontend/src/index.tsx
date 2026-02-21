import "./main.css";

import {
    DataPanels,
    LogPanel,
    SensorsDataPanel,
    OtherDataPanel,
} from "./components/Panels";
import MapView from "./components/map/MapView";
import Commands from "./components/Commands";
import StatusTopBar from "./components/Bar";
import Prelaunch from "./components/Prelaunch";

import { usePoll } from "./hooks/usePoll";
import { StateProvider, useAppState } from "./hooks/StateContext";

function InnerApp() {
    const { state, setState } = useAppState();
    usePoll(setState, 1000);

    const PRELAUNCH_STATE_ENUM = 1;

    const inPrelaunch =
        state === null ||
        state.fsm.currentState === null ||
        state.fsm.currentState <= PRELAUNCH_STATE_ENUM;

    return (
        <>
            <StatusTopBar />

            {inPrelaunch ? (
                <Prelaunch />
            ) : (
                <div className="p-3 space-y-2">
                    <MapView />

                    <DataPanels />

                    <Commands />

                    <SensorsDataPanel />
                    <OtherDataPanel />
                    <LogPanel />
                </div>
            )}
        </>
    );
}

export default function GroundStationApp() {
    return (
        <StateProvider>
            <InnerApp />
        </StateProvider>
    );
}
