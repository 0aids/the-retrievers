import { useAppState } from "../hooks/StateContext";
import { isTasked, capitalizeFirstLetter } from "../utils";

export default function ComponentPanel() {
    const { state } = useAppState();
    const components = state?.components ?? [];

    return (
        <section className="space-y-2">
            <div className="bg-gray-900 border border-gray-800 rounded-xl p-3">
                <h3 className="text-xs uppercase tracking-wider text-gray-400 mb-1">
                    Components
                </h3>

                <div className="space-y-1 text-sm">
                    {components.map((c) => {
                        const name = capitalizeFirstLetter(
                            c.name.replace("psatFSM_component_", ""),
                        );
                        const taskRunning = c.task ?? false;

                        return (
                            <div
                                key={c.id}
                                className="flex justify-between items-center py-1 border-b border-gray-800"
                            >
                                <span>{name}</span>

                                <div className="flex gap-2 text-xs font-semibold">
                                    <span
                                        style={{
                                            color: c.inited
                                                ? "#63ba80"
                                                : "#ef4444",
                                        }}
                                    >
                                        {c.inited ? "Enabled" : "Disabled"}
                                    </span>

                                    {isTasked(c.id) && (
                                        <span
                                            style={{
                                                color: taskRunning
                                                    ? "#F59E0B"
                                                    : "#6B7280",
                                            }}
                                        >
                                            {taskRunning
                                                ? "Task Running"
                                                : "Task Stopped"}
                                        </span>
                                    )}

                                    {c.error && (
                                        <span
                                            className="animate-pulse"
                                            style={{ color: "#ef4444" }}
                                        >
                                            Uh oh Error!!!
                                        </span>
                                    )}
                                </div>
                            </div>
                        );
                    })}
                </div>
            </div>
        </section>
    );
}
