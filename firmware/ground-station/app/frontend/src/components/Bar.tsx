import { useAppState } from "../hooks/StateContext";

export default function StatusTopBar() {
    const { state } = useAppState();

    return (
        <nav className="sticky top-0 z-50 bg-[#050809]/90 backdrop-blur-md border-b border-gray-800 px-4 py-3 flex items-center justify-between">
            <div>
                <h1 className="text-[0.6rem] uppercase tracking-widest text-gray-400 font-bold">
                    The Retrievers
                </h1>
                <h2 className="text-lg font-bold">Ground Station</h2>
            </div>

            <div className="flex gap-2">
                <div className="bg-gray-900 border border-gray-800 rounded-lg px-2 py-1 text-center">
                    <p className="text-[0.55rem] uppercase text-gray-400">
                        FSM
                    </p>
                    <p className="text-xs font-semibold">
                        {state?.fsm?.currentStateName?.replace(
                            "psatFSM_state_",
                            "",
                        ) ?? "IDK YET"}
                    </p>
                </div>

                <div className="bg-gray-900 border border-gray-800 rounded-lg px-2 py-1 text-center">
                    <p className="text-[0.55rem] uppercase text-gray-400">
                        GPS
                    </p>
                    <div className="flex items-center gap-1 justify-center">
                        <span
                            style={{
                                backgroundColor: state?.gps?.positionValid
                                    ? "green"
                                    : "red",
                            }}
                            className="w-2 h-2 rounded-full animate-pulse"
                        />
                        <p className="text-xs font-semibold">
                            {state?.gps?.positionValid ? "Fix" : "No Fix"}
                        </p>
                    </div>
                </div>
            </div>
        </nav>
    );
}
