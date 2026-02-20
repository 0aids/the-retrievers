import "./main.css";

import React from "react";
import { createRoot } from "react-dom/client";

import GroundStationApp from "./index.tsx";

createRoot(document.getElementById("root")!).render(
    <React.StrictMode>
        <GroundStationApp />
    </React.StrictMode>,
);
