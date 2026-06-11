import { storeToast } from "./toast.svelte";

export interface PlayerStats {
    username: string;
    total_wins: number;
    total_losses: number;
    rank: number | null;
}

class StoreStats {
    myStats = $state<PlayerStats | null>(null);
    leaderboard = $state<PlayerStats[]>([]);
    isLoading = $state(false);

    async fetchMe(): Promise<void> {
        try {
            const res = await fetch("/stats/me", {
                credentials: "include",
                headers: { "Content-Type": "application/json" }
            });

            if (res.ok) {
                this.myStats = await res.json();
            }
        } catch {
            storeToast.error("Failed to load personal statistics.");
        }
    }

    async fetchLeaderboard(): Promise<void> {
        this.isLoading = true;
        try {
            const res = await fetch("/stats/leaderboard", {
                credentials: "include",
                headers: { "Content-Type": "application/json" }
            });

            if (res.ok) {
                const data = await res.json();
                this.leaderboard = data.leaderboard || [];
            } else {
                storeToast.error("Failed to load leaderboard.");
            }
        } catch {
            storeToast.error("Network error while loading leaderboard.");
        } finally {
            this.isLoading = false;
        }
    }
}

export const storeStats = new StoreStats();
