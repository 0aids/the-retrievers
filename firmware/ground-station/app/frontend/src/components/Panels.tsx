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
                            {state?.data.gps?.latitude &&
                            state?.data.gps?.longitude
                                ? `(${state.data.gps.latitude}, ${state.data.gps.longitude})`
                                : "-"}
                        </span>
                    </p>
                    <p>
                        Altitude:{" "}
                        <span>{state?.data.gps?.altitude ?? "-"}</span>
                    </p>
                    <p>
                        Speed: <span>{state?.data.gps?.speedKph ?? "-"}</span>
                    </p>

                    <p>
                        Sats:{" "}
                        <span>{`In View: ${state?.data.gps?.satsInView ?? "-"}, Tracked: ${state?.data.gps?.satellitesTracked ?? "-"}`}</span>
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
                    Last Packet Timestamp:{" "}
                    <span>
                        {state?.radio?.lastPacketTime
                            ? new Date(
                                  state.radio.lastPacketTime * 1000,
                              ).toLocaleDateString(undefined, {
                                  year: "2-digit",
                                  month: "2-digit",
                                  day: "2-digit",
                                  hour: "2-digit",
                                  minute: "2-digit",
                                  second: "2-digit",
                              })
                            : "-"}
                    </span>
                </p>
                <p className="text-sm">
                    Total Packets Recieved:{" "}
                    <span>{state?.radio?.packetsReceived ?? "-"}</span>
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
    const { state } = useAppState();

    return (
        <section>
            <div className="bg-gray-900 border border-gray-800 rounded-xl p-3">
                <h3 className="text-xs uppercase tracking-wider text-gray-400 mb-2">
                    Sensors
                </h3>
                <div className="text-sm space-y-1">
                    <p>
                        Pressure: <span>{state?.data.pressure ?? "-"}</span>
                    </p>
                    <p>
                        Temperature:{" "}
                        <span>{state?.data.temperature ?? "-"}</span>
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
    const { state } = useAppState();

    return (
        <section>
            <div className="bg-gray-900 border border-gray-800 rounded-xl p-3">
                <h3 className="text-xs uppercase tracking-wider text-gray-400 mb-2">
                    Other
                </h3>
                <div className="text-sm space-y-1">
                    <p>
                        Battery Level:{" "}
                        <span>{state?.data.batteryLevel ?? "-"}</span>
                    </p>
                    <p>
                        Servo Angle:{" "}
                        <span>{state?.data.servoAngle ?? "-"}</span>
                    </p>
                    <p>
                        Camera On: <span>{state?.data.cameraOn ?? "-"}</span>
                    </p>
                    <p>
                        LDR Voltage:{" "}
                        <span>{state?.data.ldrVoltage ?? "-"}</span>
                    </p>
                </div>
            </div>
        </section>
    );
}
