import { defineConfig } from "vite";
import { svelte } from "@sveltejs/vite-plugin-svelte";
import tailwindcss from "@tailwindcss/vite";
import fs from "node:fs";
import path from "node:path";

const watchPublicDirPlugin = {
	name: "watch-public-dir",
	buildStart() {
		const publicDir = path.resolve("public");
		if (fs.existsSync(publicDir)) {
			const addFilesRecursively = (dir) => {
				const entries = fs.readdirSync(dir, { withFileTypes: true });
				for (const entry of entries) {
					const fullPath = path.resolve(dir, entry.name);
					if (entry.isDirectory()) {
						addFilesRecursively(fullPath);
					} else {
						// Tells the bundler to watch this specific file for modifications
						this.addWatchFile(fullPath);
					}
				}
			};
			addFilesRecursively(publicDir);
		}
	}
};

const appVersion = fs.readFileSync(path.resolve("../VERSION"), "utf8").trim();

export default defineConfig(({ mode }) => {
	const isDev = mode === "development";
	const watch = isDev ? { watch: {} } : {};

	return {
		// Only run the watch plugin when running in development/watch mode
		plugins: [tailwindcss(), svelte(), isDev && watchPublicDirPlugin].filter(Boolean),
		resolve: {
			alias: { $lib: path.resolve("./src/lib") }
		},
		define: {
			__APP_VERSION__: JSON.stringify(appVersion)
		},
		base: "./",
		build: {
			outDir: "../public",
			emptyOutDir: true,
			minify: isDev ? false : "esbuild",
			sourcemap: isDev,
			...watch
		}
	};
});
