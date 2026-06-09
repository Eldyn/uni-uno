<script lang="ts">
    import { storeAuth } from "../stores/auth.svelte";

    // Statistiche personali dell'utente loggato
    let myStats = $state({
        wins: 69,
        losses: 21,
        
        get wlRatio() {
            if (this.losses === 0) return this.wins.toFixed(2);
            return (this.wins / this.losses).toFixed(2);
        },
        cardsPlayed: {
            red: 340,
            blue: 310,
            green: 295,
            yellow: 325,
            jolly: 150,
            get numbers() {
                return this.red + this.blue + this.green + this.yellow;
            },
            get total() {
                return this.numbers + this.jolly;
            }
        }
    });

    function handleImageChange(event: Event) {
        const target = event.target as HTMLInputElement;
        if (target.files && target.files[0]) {
            const file = target.files[0];
            storeAuth.updateAvatar(file);
        }
    }
</script>

<div class="stats-container">
    <div class="stats-card">
        <h3>📈 Le tue Statistiche</h3>
        
        <div class="main-stats-grid">
            <div class="mini-stat-box win">
                <span class="label">Vinte</span>
                <span class="value">{myStats.wins}</span>
            </div>
            <div class="mini-stat-box loss">
                <span class="label">Perse</span>
                <span class="value">{myStats.losses}</span>
            </div>
            <div class="mini-stat-box ratio">
                <span class="label">Rateo W/L</span>
                <span class="value">{myStats.wlRatio}</span>
            </div>
        </div>

        <h4 class="section-sub-title">🃏 Carte Giocate ({myStats.cardsPlayed.total} totali)</h4>
        
        <div class="cards-color-grid">
            <div class="card-count-badge red">
                <span>Rosse</span>
                <strong>{myStats.cardsPlayed.red}</strong>
            </div>
            <div class="card-count-badge blue">
                <span>Blu</span>
                <strong>{myStats.cardsPlayed.blue}</strong>
            </div>
            <div class="card-count-badge green">
                <span>Verdi</span>
                <strong>{myStats.cardsPlayed.green}</strong>
            </div>
            <div class="card-count-badge yellow">
                <span>Gialle</span>
                <strong>{myStats.cardsPlayed.yellow}</strong>
            </div>
        </div>

        <div class="cards-type-grid">
            <div class="type-box">
                <span class="type-label">🔢 Carte Numeriche:</span>
                <span class="type-value">{myStats.cardsPlayed.numbers}</span>
            </div>
            <div class="type-box">
                <span class="type-label">🌈 Pride / Jolly:</span>
                <span class="type-value">{myStats.cardsPlayed.jolly}</span>
            </div>
        </div>

    </div>
</div>

<style>  
    /* --- RESET GLOBALE --- */
    :global(html, body) {
        margin: 0;
        padding: 0;
        width: 100%;
        height: 100%;
        background-color: #11111b; 
    }

    /* --- STATS SCREEN & LEADERBOARD STYLES --- */
    .stats-container {
        display: flex;
        flex-direction: column;
        width: 80%; 
        min-height: 65vh; 
        box-sizing: border-box;
        padding: 20px; 
        margin: 0 auto; 
    }

    .stats-card {
        background: var(--accent-bg, #1e1e2e);
        border: 1px solid var(--accent-border, #313244);
        padding: 28px;
        border-radius: 16px;
        width: 100%;
        flex: 1; 
        display: flex;
        flex-direction: column;
        box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
        box-sizing: border-box;
    }

    h3, h4 {
        margin-top: 0;
        margin-bottom: 20px;
        color: var(--text-h, #cdd6f4);
    }

    .section-sub-title {
        margin-top: 48px; 
        margin-bottom: 12px;
        font-size: 15px;
        color: var(--text-muted, #a6adc8);
    }

    .main-stats-grid {
        display: grid;
        grid-template-columns: repeat(3, 1fr);
        gap: 12px;
    }

    .mini-stat-box {
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        padding: 20px 14px; 
        border-radius: 10px;
        background: rgba(0, 0, 0, 0.2);
        height: 100%;
    }

    .mini-stat-box .label { font-size: 12px; color: #a6adc8; margin-bottom: 8px; text-transform: uppercase; letter-spacing: 0.5px;}
    .mini-stat-box .value { font-size: 24px; font-weight: bold; }
    
    .mini-stat-box.win .value { color: #a6e3a1; }
    .mini-stat-box.loss .value { color: #f38ba8; }
    .mini-stat-box.ratio .value { color: #89b4fa; }

    /* --- DETTAGLIO COLORI CARTE --- */
    .cards-color-grid {
        display: grid;
        grid-template-columns: repeat(2, 1fr);
        gap: 12px;
    }

    @media (min-width: 480px) {
        .cards-color-grid { grid-template-columns: repeat(4, 1fr); }
    }

    .card-count-badge {
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        padding: 16px 10px;
        border-radius: 8px;
        font-size: 14px;
        color: #11111b;
        height: 100%;
    }

    .card-count-badge strong { font-size: 18px; margin-top: 4px; }

    .red { background-color: #f38ba8; }
    .blue { background-color: #89b4fa; }
    .green { background-color: #a6e3a1; }
    .yellow { background-color: #f9e2af; }

    /* --- TIPOLOGIE CARTE --- */
    .cards-type-grid {
        display: flex;
        flex-direction: column;
        gap: 12px;
        margin-top: auto; 
        padding-top: 24px;
    }

    .type-box {
        display: flex;
        justify-content: space-between;
        align-items: center;
        padding: 16px 20px;
        background: rgba(255, 255, 255, 0.02);
        border: 1px solid #313244;
        border-radius: 8px;
        font-size: 15px;
    }

    .type-label { color: #a6adc8; }
    .type-value { font-weight: bold; color: #cdd6f4; font-size: 18px; }
</style>