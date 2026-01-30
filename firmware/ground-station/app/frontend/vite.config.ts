import path from "path";
import { defineConfig } from "vite";
import tailwindcss from "@tailwindcss/vite";

export default defineConfig({
    plugins: [tailwindcss()],
    root: ".",
    base: "/static/",
    build: {
        outDir: path.resolve(__dirname, "../static"),
        emptyOutDir: true,
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
