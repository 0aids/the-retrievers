import { useAppState } from "../hooks/StateContext";

export function DataPanels() {
    const { state } = useAppState();

    return (
        <section className="space-y-2">
            <div className="bg-gray-900 border border-gray-800 rounded-xl p-3">
                <h3 className="text-xs uppercase tracking-wider text-gray-400 mb-2">
                    GPS
                </h3>
                <div className="text-sm space-y-1">
                    <p>
                        Pos:{" "}
                        <span>
                            {state?.gps?.latitude && state?.gps?.longitude
                                ? `(${state.gps.latitude}, ${state.gps.longitude})`
                                : "-"}
                        </span>
                    </p>
                    <p>
                        Altitude: <span>{state?.gps?.altitude ?? "-"}</span>
                    </p>
                    <p>
                        Speed: <span>{state?.gps?.speedKph ?? "-"}</span>
                    </p>

                    <p>
                        Sats:{" "}
                        <span>{`In View: ${state?.gps?.satsInView ?? "-"}, Tracked: ${state?.gps?.satellitesTracked ?? "-"}`}</span>
                    </p>
                </div>
            </div>

            <div className="bg-gray-900 border border-gray-800 rounded-xl p-3">
                <h3 className="text-xs uppercase tracking-wider text-gray-400 mb-2">
                    FSM
                </h3>
                <p className="text-sm">
                    Current:{" "}
                    <span className="font-semibold">
                        {state?.fsm?.currentStateName ?? "-"}
                    </span>
                </p>
                <p className="text-xs text-gray-400">
                    Prev: <span>{state?.fsm?.prevStateName ?? "-"}</span>
                </p>
            </div>

            <div className="bg-gray-900 border border-gray-800 rounded-xl p-3">
                <h3 className="text-xs uppercase tracking-wider text-gray-400 mb-2">
                    LoRa
                </h3>
                <p className="text-sm">
                    Last: <span>{state?.radio?.lastPacketTime ?? "-"}</span>
                </p>
                <p className="text-sm">
                    RX: <span>{state?.stats?.packetsReceived ?? "-"}</span>
                </p>
            </div>
        </section>
    );
}

export function LogPanel() {
    const { state } = useAppState();

    return (
        <section className="">
            <div className="bg-black border border-gray-800 rounded-xl p-3">
                <h3 className="text-xs uppercase tracking-wider text-gray-400 mb-2">
                    Raw State JSON
                </h3>
                <pre className="text-xs text-green-400 max-h-48 overflow-y-auto no-scrollbar">
                    {JSON.stringify(state, null, 2)}
                </pre>
            </div>
        </section>
    );
}

export function SensorsDataPanel() {
    return (
        <section>
            <div className="bg-gray-900 border border-gray-800 rounded-xl p-3">
                <h3 className="text-xs uppercase tracking-wider text-gray-400 mb-2">
                    Sensors
                </h3>
                <div className="text-sm space-y-1">
                    <p>
                        Pressure: <span>Not Implemented Yet</span>
                    </p>
                    <p>
                        Temperature: <span>Not Implemented Yet</span>
                    </p>
                    <p>
                        Accelerometer: <span>Not Implemented Yet</span>
                    </p>
                    <p>
                        IMU: <span>Not Implemented Yet</span>
                    </p>
                </div>
            </div>
        </section>
    );
}

export function OtherDataPanel() {
    return (
        <section>
            <div className="bg-gray-900 border border-gray-800 rounded-xl p-3">
                <h3 className="text-xs uppercase tracking-wider text-gray-400 mb-2">
                    Other
                </h3>
                <div className="text-sm space-y-1">
                    <p>
                        Battery Level: <span>Not Implemented Yet</span>
                    </p>
                    <p>
                        Servo Angle: <span>Not Implemented Yet</span>
                    </p>
                    <p>
                        Camera Status: <span>Not Implemented Yet</span>
                    </p>
                    <p>
                        LDR Voltage: <span>Not Implemented Yet</span>
                    </p>
                </div>
            </div>
        </section>
    );
}
