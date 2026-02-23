import { sendCommand } from "../services/api";

export default function Commands() {
    return (
        <section className="space-y-1">
            <h3 className="text-xs uppercase tracking-wider text-gray-400">
                Commands
            </h3>

            <div className="grid grid-cols-2 gap-2">
                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        sendCommand(11);
                    }}
                >
                    Beep Short
                </button>

                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        sendCommand(12);
                    }}
                >
                    Beep Long
                </button>
                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        const newState = 7; // recovery
                        sendCommand(16, newState);
                    }}
                >
                    Fast Forward State
                </button>

                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        const newState = 7; // recovery
                        sendCommand(17, newState);
                    }}
                >
                    State Override
                </button>

                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        const component = 0; // ldr
                        sendCommand(18, component);
                    }}
                >
                    Enable Component
                </button>

                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        const component = 0; // ldr
                        sendCommand(19, component);
                    }}
                >
                    Disable Component
                </button>
            </div>
        </section>
    );
}
