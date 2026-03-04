interface SelectionModalProps<T> {
    isOpen: boolean;
    title: string;
    items: T[];
    onClose: () => void;
    onSelect: (item: T) => void;
    getKey: (item: T) => string | number;
    renderLabel: (item: T) => React.ReactNode;
    renderSubLabel?: (item: T) => React.ReactNode;
}

export default function SelectionModal<T>({
    isOpen,
    title,
    items,
    onClose,
    onSelect,
    getKey,
    renderLabel,
    renderSubLabel,
}: SelectionModalProps<T>) {
    if (!isOpen) return null;

    return (
        <div className="fixed inset-0 z-50 flex flex-col justify-end bg-black/50 backdrop-blur-sm">
            <div className="flex-1" onClick={onClose} />

            <div className="bg-gray-900 border-t border-gray-800 rounded-t-3xl px-4 pt-3 pb-5 max-h-[60vh] overflow-y-auto shadow-2xl">
                <h2 className="text-base font-semibold mb-3 text-gray-200">
                    {title}
                </h2>

                <div className="grid grid-cols-3 gap-2">
                    {items.map((item) => (
                        <button
                            key={getKey(item)}
                            className="bg-gray-800 hover:bg-gray-700 active:scale-95 transition-all border border-gray-700 py-2.5 rounded-lg text-xs text-gray-200"
                            onClick={() => {
                                onSelect(item);
                                onClose();
                            }}
                        >
                            <div className="font-medium">
                                {renderLabel(item)}
                            </div>
                            {renderSubLabel && (
                                <div className="text-[10px] text-gray-500 mt-0.5">
                                    {renderSubLabel(item)}
                                </div>
                            )}
                        </button>
                    ))}
                </div>

                <button
                    className="mt-4 text-sm text-gray-400 hover:text-gray-200 transition w-full"
                    onClick={onClose}
                >
                    Cancel
                </button>
            </div>
        </div>
    );
}
