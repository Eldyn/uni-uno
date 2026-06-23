/**
 * @file navigation.svelte.ts
 * @brief Manager of the application's internal routing (Single Page Application).
 * Keeps a history limited to one level in memory to handle the "Back" action.
 */

import { storeAuth } from "./auth.svelte";
import { ws } from "./ws.svelte";

/**
 * @typedef AppScreen
 * @brief List of the screens available in the frontend application.
 */
export type AppScreen =
	| "main"
	| "auth"
	| "lobbies"
	| "lobby"
	| "game"
	| "settings"
	| "stats"
	| "detailedStats";

/**
 * @class StoreNavigation
 * @brief Reactive store for screen switching.
 * Uses localStorage to persist the current screen and restore it
 * after a reload (F5), after verifying the login state.
 * @tag FRONT-NAV-001
 */
class StoreNavigation {
	/** The screen currently displayed to the user. */
	current = $state<AppScreen>("main");

	/** Stores the previous screen for 'back' navigation. */
	#previous: AppScreen | null = null;

	#screenRestored = false;

	constructor() {
		ws.onOpen(() => {
			if (this.#screenRestored) return;
			this.#screenRestored = true;

			const localScreen = localStorage.getItem("currentScreen");
			if (!localScreen) return;
			if (!storeAuth.isLoggedIn) {
				localStorage.removeItem("currentScreen");
				return;
			}

			this.goto(localScreen as AppScreen);
		});
	}

	/**
	 * @brief Changes the current screen.
	 * Automatically saves the screen to `localStorage`.
	 * @param screen The new destination screen.
	 */
	goto(screen: AppScreen): void {
		if (screen === this.current) return;
		this.#previous = this.current;
		this.current = screen;
		localStorage.setItem("currentScreen", screen);
	}

	/**
	 * @brief Returns to the previous screen, if available in memory.
	 */
	back(): void {
		if (this.#previous) this.goto(this.#previous);
	}
}

export const storeNavigation = new StoreNavigation();
