<script lang="ts">
    import { storeNavigation } from "../stores/navigation.svelte";
    import GameCard from "./game/GameCard.svelte"; 

    const standardColors = ["red", "yellow", "blue", "green"];
    const standardValues = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "skip", "reverse", "+2"];
    const wildColors = ["wild"];
    const wildValues = ["colorswitch", "+4"];

    const TOTAL_CARDS = 15; 
    const FALL_DURATION = 5; 

    function getCardRandomProps() {
        const isWild = Math.random() < 0.20; 
        
        const color = isWild 
            ? wildColors[Math.floor(Math.random() * wildColors.length)]
            : standardColors[Math.floor(Math.random() * standardColors.length)];
            
        const value = isWild 
            ? wildValues[Math.floor(Math.random() * wildValues.length)]
            : standardValues[Math.floor(Math.random() * standardValues.length)];

        return {
            left: `${Math.random() * 90}%`,
            rotateZ: `${Math.floor(Math.random() * 60) - 30}deg`,   
            initialRX: `${Math.floor(Math.random() * 40) - 20}deg`, 
            scale: `${(Math.random() * 0.4 + 0.7).toFixed(2)}`,    
            cardData: { id: Math.floor(Math.random() * 10000), value, color } 
        };
    }

    function createInitialCard(index: number) {
        const calculatedDelay = ((index * FALL_DURATION) / TOTAL_CARDS).toFixed(2);
        return {
            slotIndex: index,
            delay: `${calculatedDelay}s`, 
            duration: `${FALL_DURATION}s`, 
            ...getCardRandomProps()
        };
    }

    let backgroundCards = $state(Array.from({ length: TOTAL_CARDS }, (_, i) => createInitialCard(i)));

    function regenerateCard(index: number) {
        backgroundCards[index] = {
            ...backgroundCards[index],
            ...getCardRandomProps() 
        };
    }
</script>

<div class="screen-container">
    
    <div class="background-overlay">
        {#each backgroundCards as card, index (card.slotIndex)}
            <div 
                class="bg-card-wrapper"
                onanimationiteration={() => regenerateCard(index)}
                style="left: {card.left}; --rotate-z: {card.rotateZ}; --initial-rx: {card.initialRX}; --delay: {card.delay}; --duration: {card.duration}; --scale: {card.scale};"
            >
                <div class="card-3d-inner">
                    <div class="card-side card-front">
                        <GameCard card={card.cardData} turned={false} style="position: absolute; left: 0; top: 0; transform: none; margin: 0;" />
                    </div>
                    <div class="card-side card-back">
                        <GameCard card={card.cardData} turned={true} style="position: absolute; left: 0; top: 0; transform: none; margin: 0;" />
                    </div>
                </div>
            </div>
        {/each}
    </div>

    <header class="logo-container">
        <img 
            src="/assets/logo.png" 
            alt="Logo" 
            class="logo" 
        />
    </header>

    <main class="button-container">
        <button onclick={() => storeNavigation.goto("auth")}>
            Accedi / Registrati
        </button>
    </main>

</div>

<style>
  .screen-container {
    position: relative;
    display: flex;
    flex-direction: column;
    min-height: 100vh;
    background: var(--bg);
    overflow: hidden;
  }

  .background-overlay {
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    z-index: 0;
    pointer-events: none;
    perspective: 1200px; 
  }

  .bg-card-wrapper {
    position: absolute;
    top: -180px; 
    width: 5em;       
    height: 7.68em;   
    opacity: 0;
    animation: fallAndFade var(--duration) linear var(--delay) infinite;
  }

  .card-3d-inner {
    position: relative;
    width: 100%;
    height: 100%;
    transform-style: preserve-3d; 
    animation: spin3D var(--duration) linear var(--delay) infinite;
  }

  .card-side {
    position: absolute;
    inset: 0;
    backface-visibility: hidden;
    -webkit-backface-visibility: hidden;
  }

  .card-front { transform: rotateY(0deg) translateZ(1px); }
  .card-back { transform: rotateY(180deg) translateZ(1px); }

  @keyframes fallAndFade {
    0% { transform: translateY(0); opacity: 0; }
    8% { opacity: 0.45; }
    92% { opacity: 0.45; }
    100% { transform: translateY(125vh); opacity: 0; }
  }

  @keyframes spin3D {
    0% { transform: rotateX(var(--initial-rx)) rotateY(0deg) rotateZ(var(--rotate-z)) scale(var(--scale)); }
    100% { transform: rotateX(calc(var(--initial-rx) + 40deg)) rotateY(720deg) rotateZ(calc(var(--rotate-z) + 150deg)) scale(var(--scale)); }
  }

  .logo-container, .button-container {
    position: relative;
    z-index: 1;
  }

  .logo-container {
    margin-top: 5%;
    display: flex;
    justify-content: center;
    padding-top: 2rem; 
  }

  .logo {
    width: 400px; 
    height: auto; 
    border-radius: 8px; 
  }

  .button-container {
    display: flex;
    justify-content: center;
    align-items: center;
    padding: 10vh 0; 
  }

  /* ==========================================================================
     STILE RETRO POP / ARCADE
     ========================================================================== */
  button {
    padding: 1.1rem 2.6rem;
    font-size: 1.35rem;
    /* Font massiccio e geometrico per il feeling Arcade/Retro */
    font-family: 'Arial Black', 'Impact', 'Arial', sans-serif;
    font-weight: 900;
    text-transform: uppercase;
    letter-spacing: 1.5px;
    
    color: #ffffff;
    background-color: #8d42d3; 
    
    /* Bordi netti e geometrici perfettamente simmetrici */
    border: 3px solid #1a1a1a;
    border-radius: 12px; /* Un arrotondamento pulito e moderno/retro */
    
    cursor: pointer;
    
    /* Ombra Flat netta geometrica a 45 gradi */
    box-shadow: 5px 5px 0px #1a1a1a;
    
    /* Transizione lineare e reattiva */
    transition: transform 0.08s ease, box-shadow 0.08s ease, background-color 0.2s ease;
  }

  button:hover {
    background-color: #9c4fe3;
    /* Si alza diagonalmente in modo perfetto */
    transform: translate(-3px, -3px);
    /* L'ombra cresce proporzionalmente per simulare l'altezza */
    box-shadow: 8px 8px 0px #1a1a1a;
  }

  button:active {
    /* Effetto "pressione del tasto del cabinato": si schiaccia coprendo l'ombra */
    transform: translate(5px, 5px);
    box-shadow: 0px 0px 0px #1a1a1a;
  }
</style>