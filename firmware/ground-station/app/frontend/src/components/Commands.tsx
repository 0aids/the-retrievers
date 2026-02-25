import { useState } from "react";

import SelectionModal from "./Modal";
import { PacketType } from "../types/enums";
import { sendCommand } from "../services/api";
import { useAppState } from "../hooks/StateContext";

export const FSM_STATES = [
    { id: 0, name: "Start" },
    { id: 1, name: "Prelaunch" },
    { id: 2, name: "Ascent" },
    { id: 3, name: "Deploy Pending" },
    { id: 4, name: "Deployed" },
    { id: 5, name: "Descent" },
    { id: 6, name: "Landing" },
    { id: 7, name: "Recovery" },
    { id: 8, name: "Low Power" },
    { id: 9, name: "Error" },
    { id: 10, name: "Permanent Error" },
];

export default function Commands() {
    const [modalOpen, setModalOpen] = useState(false);
    const [modalMode, setModalMode] = useState<
        "enable" | "disable" | "override" | "fastForward" | null
    >(null);

    const { state } = useAppState();

    return (
        <section className="space-y-1">
            <h3 className="text-xs uppercase tracking-wider text-gray-400">
                Commands
            </h3>

            <div className="grid grid-cols-2 gap-2">
                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        sendCommand(PacketType.loraFsm_packetType_buzzShortReq);
                    }}
                >
                    Beep Short
                </button>

                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        sendCommand(PacketType.loraFsm_packetType_buzzLongReq);
                    }}
                >
                    Beep Long
                </button>

                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        setModalMode("fastForward");
                        setModalOpen(true);
                    }}
                >
                    Fast Forward State
                </button>

                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        setModalMode("override");
                        setModalOpen(true);
                    }}
                >
                    State Override
                </button>

                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        setModalMode("enable");
                        setModalOpen(true);
                    }}
                >
                    Enable Component
                </button>

                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        setModalMode("disable");
                        setModalOpen(true);
                    }}
                >
                    Disable Component
                </button>
            </div>

            <SelectionModal
                isOpen={modalOpen}
                title={
                    modalMode === "override" || modalMode === "fastForward"
                        ? "Choose State"
                        : modalMode === "enable"
                          ? "Choose Component to Enable"
                          : modalMode === "disable"
                            ? "Choose Component to Disable"
                            : ""
                }
                items={
                    modalMode === "override" || modalMode === "fastForward"
                        ? FSM_STATES
                        : (state?.components ?? [])
                }
                onClose={() => setModalOpen(false)}
                onSelect={(item) => {
                    if (modalMode === "override") {
                        sendCommand(
                            PacketType.loraFsm_packetType_stateOverrideReq,
                            item.id,
                        );
                    } else if (modalMode === "fastForward") {
                        sendCommand(
                            PacketType.loraFsm_packetType_fastForwardReq,
                            item.id,
                        );
                    } else if (modalMode === "enable") {
                        sendCommand(
                            PacketType.loraFsm_packetType_enableComponentReq,
                            item.id,
                        );
                    } else if (modalMode === "disable") {
                        sendCommand(
                            PacketType.loraFsm_packetType_disableComponentReq,
                            item.id,
                        );
                    }
                }}
                getKey={(item) => item.id}
                renderLabel={(item) => item.name}
                renderSubLabel={(item) => `ID ${item.id}`}
            />
        </section>
    );
}
