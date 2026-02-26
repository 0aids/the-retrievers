import { useState } from "react";

import SelectionModal from "./Modal";
import { PacketType } from "../types/enums";
import { sendCommand } from "../services/api";
import { useAppState } from "../hooks/StateContext";
import { capitalizeFirstLetter } from "../utils";

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
                    {state?.components.map(
                        ({ name, enabled, preflightSuccess, id }) => {
                            return (
                                <div
                                    key={id}
                                    className="bg-gray-800 rounded-lg px-3 py-2 flex flex-col"
                                >
                                    <div className="flex justify-between items-center">
                                        <span className="font-bold">
                                            {capitalizeFirstLetter(
                                                name.replace(
                                                    "psatFSM_component_",
                                                    "",
                                                ),
                                            )}
                                        </span>

                                        <span
                                            className="text-xs font-semibold"
                                            style={{
                                                color: enabled
                                                    ? "#63ba80"
                                                    : "#ef4444",
                                            }}
                                        >
                                            {enabled ? "ENABLED" : "DISABLED"}
                                        </span>
                                    </div>

                                    <div className="flex justify-between mt-1 text-xs">
                                        <span className="text-gray-400">
                                            Preflight
                                        </span>
                                        <span
                                            style={{
                                                color:
                                                    preflightSuccess === null
                                                        ? "#9CA3AF"
                                                        : preflightSuccess
                                                          ? "#63ba80"
                                                          : "#ef4444",
                                            }}
                                        >
                                            {preflightSuccess === null
                                                ? "NOT RUN"
                                                : preflightSuccess
                                                  ? "PASS"
                                                  : "FAIL"}
                                        </span>
                                    </div>
                                </div>
                            );
                        },
                    )}
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
                        Preflight Tests:
                    </h3>
                    <button
                        className="bg-gray-700 gs-btn border border-gray-700 py-3 rounded-xl text-sm w-full"
                        onClick={() => {
                            sendCommand(
                                PacketType.loraFsm_packetType_preflightReq,
                            );
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
                        sendCommand(
                            PacketType.loraFsm_packetType_prelaunchCompleteReq,
                        );

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
                    const command =
                        modalMode === "enable"
                            ? PacketType.loraFsm_packetType_markComponentEnabledReq
                            : PacketType.loraFsm_packetType_markComponentDisabledReq;
                    sendCommand(command, c.id);
                }}
                getKey={(c) => c.id}
                renderLabel={(c) =>
                    capitalizeFirstLetter(
                        c.name.replace("psatFSM_component_", ""),
                    )
                }
                renderSubLabel={(c) => `ID ${c.id}`}
            />
        </section>
    );
};

export default Prelaunch;
