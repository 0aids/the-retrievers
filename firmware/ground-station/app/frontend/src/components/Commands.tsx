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
                        console.log("Running Command Placeholder");
                    }}
                >
                    Beep Short
                </button>

                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        console.log("Running Command Placeholder");
                    }}
                >
                    Beep Long
                </button>
                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        console.log("Running Command Placeholder");
                    }}
                >
                    Fast Forward State
                </button>

                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        console.log("Running Command Placeholder");
                    }}
                >
                    State Override
                </button>

                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        console.log("Running Command Placeholder");
                    }}
                >
                    Enable Component
                </button>

                <button
                    className="bg-gray-800 gs-btn border border-gray-700 py-3 rounded-xl text-sm"
                    onClick={() => {
                        console.log("Running Command Placeholder");
                    }}
                >
                    Disable Component
                </button>
            </div>
        </section>
    );
}
