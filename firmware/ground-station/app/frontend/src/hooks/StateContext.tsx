import React, { createContext, useContext, useState } from "react";

import type { ApiState } from "../types/state";

type StateContextType = {
    state: ApiState | null;
    setState: React.Dispatch<React.SetStateAction<ApiState | null>>;
};

const StateContext = createContext<StateContextType | undefined>(undefined);

export const StateProvider: React.FC<{ children: React.ReactNode }> = ({
    children,
}) => {
    const [state, setState] = useState<ApiState | null>(null);

    return (
        <StateContext.Provider value={{ state, setState }}>
            {children}
        </StateContext.Provider>
    );
};

// eslint-disable-next-line react-refresh/only-export-components
export function useAppState(): StateContextType {
    const ctx = useContext(StateContext);
    if (!ctx) {
        throw new Error("cmon man just use it correctly ");
    }

    return ctx;
}
