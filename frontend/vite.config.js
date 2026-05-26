import { defineConfig } from "vite";
import { svelte } from "@sveltejs/vite-plugin-svelte";

export default defineConfig(({ mode }) => {
    const isDev = mode === "development";

    return {
        plugins: [svelte()],
        base: "./",
        build: {
            outDir: "../public",
            emptyOutDir: true,
            minify: isDev ? false : "esbuild",
            sourcemap: isDev,
        }
    };
});