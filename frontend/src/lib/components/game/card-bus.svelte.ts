import { getContext, setContext } from "svelte";

export type ElementRole = "draw-pile" | "discard-pile" | "hand-local" | `hand-opponent-${number}`;

export interface CardFlightEvent {
    key: number;
    color: string;
    value: string;
    turned: boolean;
    from: ElementRole;
    to: ElementRole;
}

export class CardBus {
    readonly #registry = new Map<ElementRole, HTMLElement>();

    register(role: ElementRole, el: HTMLElement) {
        this.#registry.set(role, el);
    }

    unregister(role: ElementRole) {
        this.#registry.delete(role);
    }

    getEl(role: ElementRole): HTMLElement | null {
        return this.#registry.get(role) ?? null;
    }

    flights = $state<CardFlightEvent[]>([]);
    #nextKey = 0;

    launch(event: Omit<CardFlightEvent, "key">) {
        this.flights = [...this.flights, { ...event, key: this.#nextKey++ }];
    }

    land(key: number) {
        this.flights = this.flights.filter((f) => f.key !== key);
    }

    hiddenCardIds = $state(new Set<number>());

    hide(id: number) {
        this.hiddenCardIds = new Set([...this.hiddenCardIds, id]);
    }

    show(id: number) {
        this.hiddenCardIds = new Set([...this.hiddenCardIds].filter((x) => x !== id));
    }
}

const CARD_BUS_KEY = Symbol("card-bus");

export function createCardBus(): CardBus {
    const bus = new CardBus();
    setContext(CARD_BUS_KEY, bus);
    return bus;
}

export function useCardBus(): CardBus {
    return getContext<CardBus>(CARD_BUS_KEY);
}
