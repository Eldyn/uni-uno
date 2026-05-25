import { defineConfig } from "vite";
import { svelte } from "@sveltejs/vite-plugin-svelte";

export default defineConfig(({ mode }) => {
    // Check if we are running the development watch command
    const isDev = mode === "development";

    return {
        plugins: [svelte()],
        base: "./",
        build: {
            outDir: "../public",
            emptyOutDir: true,
            // Disable minification during development for easier debugging
            minify: isDev ? false : "esbuild",
            // Generate sourcemaps in dev mode to trace errors back to your original Svelte files
            sourcemap: isDev ? true : false
        }
    };
});
