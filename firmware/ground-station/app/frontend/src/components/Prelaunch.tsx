import { useState } from "react";

import { useAppState } from "../hooks/StateContext";
import { sendCommand } from "../services/api";
import SelectionModal from "./Modal";

const Prelaunch = () => {
    const { state } = useAppState();

    const initialPreflightResult = "Not Run Yet";
    const [preflightResult, setPreflightResult] = useState({
        text: initialPreflightResult,
        color: "#D1D5DB",
    });

    const [modalOpen, setModalOpen] = useState(false);
    const [modalMode, setModalMode] = useState<"enable" | "disable" | null>(
        null,
    );

    return (
        <section className="p-2">
            <div className="w-full flex items-center flex-col my-2 mb-3">
                <h1 className="text-2xl font-bold">Preflight Mode</h1>
            </div>

            <div className="bg-gray-900 border border-gray-800 rounded-xl p-3">
                <h3 className="text-xs uppercase tracking-wider text-gray-400 mb-2">
                    Component Status
                </h3>
                <div className="text-sm space-y-1 grid grid-cols-2 gap-2 gap-x-4">
                    {state?.components.length == 0 && "No Data Yet"}
                    {state?.components.map(({ name, status }) => {
                        return (
                            <p className="flex justify-between">
                                <span className="font-bold">
                                    {name.replace("psatFSM_component_", "")}:
                                </span>

                                <span
                                    style={{
                                        color: status ? "#63ba80" : "red",
                                    }}
                                >
                                    {status ? "OK" : "COOKED"}
                                </span>
                            </p>
                        );
                    })}
                </div>
                <div className="flex gap-2 mt-2">
                    <button
                        className="bg-gray-700 gs-btn border border-gray-700 py-3 rounded-xl text-sm w-full"
                        onClick={() => {
                            setModalMode("enable");
                            setModalOpen(true);
                        }}
                    >
                        Enable Component
                    </button>

                    <button
                        className="bg-gray-700 gs-btn border border-gray-700 py-3 rounded-xl text-sm w-full"
                        onClick={() => {
                            setModalMode("disable");
                            setModalOpen(true);
                        }}
                    >
                        Disable Component
                    </button>
                </div>
            </div>

            <div className="my-2 space-y-2">
                <div className="bg-gray-900 border border-gray-800 rounded-xl p-2">
                    <h3 className="text-xs uppercase tracking-wider text-gray-400 mb-2 flex justify-between">
                        Preflight Result:{" "}
                        <span
                            style={{
                                color: preflightResult.color,
                            }}
                            className="text-gray-300"
                        >
                            {preflightResult.text}
                        </span>
                    </h3>
                    <button
                        className="bg-gray-700 gs-btn border border-gray-700 py-3 rounded-xl text-sm w-full"
                        onClick={() => {
                            sendCommand(9);
                        }}
                    >
                        {preflightResult.text === initialPreflightResult
                            ? "Run"
                            : "Rerun"}{" "}
                        All Tests
                    </button>
                </div>

                <button
                    className="bg-green-800 gs-btn border border-green-700 py-3 rounded-xl text-sm w-full"
                    onClick={() => {
                        sendCommand(11);

                        setPreflightResult({
                            text: "Success",
                            color: "#63ba80",
                        });
                    }}
                >
                    Prelaunch Complete
                </button>
            </div>

            <SelectionModal
                isOpen={modalOpen}
                title={`Choose Component to ${
                    modalMode === "enable" ? "Enable" : "Disable"
                }`}
                items={state?.components ?? []}
                onClose={() => setModalOpen(false)}
                onSelect={(c) => {
                    const cmd = modalMode === "enable" ? 16 : 17;
                    sendCommand(cmd, c.id);
                }}
                getKey={(c) => c.id}
                renderLabel={(c) => c.name.replace("psatFSM_component_", "")}
                renderSubLabel={(c) => `ID ${c.id}`}
            />
        </section>
    );
};

export default Prelaunch;
