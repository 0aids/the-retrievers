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

import { usePoll } from "./hooks/usePoll";
import { StateProvider, useAppState } from "./hooks/StateContext";

function InnerApp() {
    const { setState } = useAppState();
    usePoll(setState, 1000);

    return (
        <>
            <StatusTopBar />

            <div className="p-3 space-y-2">
                <MapView />

                <DataPanels />

                <Commands />

                <SensorsDataPanel />
                <OtherDataPanel />
                <LogPanel />
            </div>
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
