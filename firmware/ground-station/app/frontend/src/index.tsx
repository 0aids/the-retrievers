import "./main.css";

import { usePoll } from "./hooks/usePoll";
import { StateProvider, useAppState } from "./hooks/StateContext";

function InnerApp() {
    const { setState } = useAppState();
    usePoll(setState, 1000);

    return (
        <>

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
