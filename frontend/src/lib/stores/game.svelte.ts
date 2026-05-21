/**
 * Game state management for lobbies, rooms, and game data
 */

export interface Lobby {
    lobby_id: string;
    host: string;
    name: string;
    player_count: number;
    players?: unknown[];
    max_players: number;
}

export interface GameState {
    currentLobby: Lobby | null;
    availableLobbies: Lobby[];
    isLoadingLobbies: boolean;
    gameData: Record<string, unknown>;
}

// Keep the raw state mutable internally, but scoped inside the module
let innerState = $state<GameState>({
    currentLobby: null,
    availableLobbies: [],
    isLoadingLobbies: false,
    gameData: {},
});

export const gameStore = {
    /**
     * Get current game state (reactive & readonly)
     */
    get state(): Readonly<GameState> {
        return innerState;
    },

    /**
     * Set current lobby
     */
    set currentLobby(lobby: Lobby | null) {
        innerState.currentLobby = lobby;
    },

    /**
     * Set available lobbies
     */
    set availableLobbies(lobbies: Lobby[]) {
        innerState.availableLobbies = lobbies;
    },

    /**
     * Set loading state for lobbies
     */
    set loadingLobbies(loading: boolean) {
        innerState.isLoadingLobbies = loading;
    },

    /**
     * Add lobby to available lobbies
     */
    addLobby(lobby: Lobby): void {
        innerState.availableLobbies.push(lobby);
    },

    /**
     * Remove lobby from available lobbies
     */
    removeLobby(lobbyId: string): void {
        innerState.availableLobbies = innerState.availableLobbies.filter(
            (l) => l.lobby_id !== lobbyId,
        );
    },

    /**
     * Update a lobby in available lobbies
     */
    updateLobby(lobbyId: string, updates: Partial<Lobby>): void {
        const lobby = innerState.availableLobbies.find(
            (l) => l.lobby_id === lobbyId,
        );
        if (lobby) {
            Object.assign(lobby, updates);
        }
    },

    /**
     * Update game data
     */
    updateGameData(data: Record<string, unknown>): void {
        innerState.gameData = { ...innerState.gameData, ...data };
    },

    /**
     * Clear game data
     */
    clearGameData(): void {
        innerState.gameData = {};
    },
};
