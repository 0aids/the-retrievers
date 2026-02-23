import path from "path";
import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";
import tailwindcss from "@tailwindcss/vite";

export default defineConfig({
    plugins: [
        react({
            babel: {
                plugins: [["babel-plugin-react-compiler"]],
            },
        }),
        tailwindcss(),
    ],
    root: ".",
    base: "/static/",
    build: {
        outDir: path.resolve(__dirname, "../static"),
        emptyOutDir: false,
        rollupOptions: {
            output: {
                assetFileNames: (chunk) => {
                    if (chunk.name?.endsWith(".css"))
                        return "css/[name][extname]";
                    return "[name][extname]";
                },
            },
        },
    },
});
