import { getContext, setContext } from "svelte";
import type { Card } from "../../stores/game.svelte";

export type ElementRole = "draw-pile" | "discard-pile" | "hand-local" | `hand-opponent-${number}`;

/** A 2D screen-space point (CSS pixels) used as a flight endpoint. */
export interface Point {
	x: number;
	y: number;
}

/**
 * A flight endpoint. It may reference a registered pile element (`role`),
 * a specific registered card slot (`slot`), or be an explicit coordinate
 * captured ahead of time (`point`) — useful when the source slot has already
 * been removed from the DOM (e.g. a card that was just played).
 */
export type FlightTarget = { role: ElementRole } | { slot: string } | { point: Point };

export interface CardFlightEvent extends Pick<Card, "type" | "value"> {
	key: number;
	turned: boolean;
	/** Resolved screen-space origin of the flight. */
	src: Point;
	/** Resolved screen-space destination of the flight. */
	dst: Point;
}

function centerOf(rect: DOMRect): Point {
	return { x: rect.left + rect.width / 2, y: rect.top + rect.height / 2 };
}

export class CardBus {
	readonly #registry = new Map<ElementRole, HTMLElement>();
	/** Per-card slot elements, keyed e.g. `local:<id>` or `opp:<user>:<index>`. */
	readonly #slots = new Map<string, HTMLElement>();

	register(role: ElementRole, el: HTMLElement) {
		this.#registry.set(role, el);
	}

	unregister(role: ElementRole) {
		this.#registry.delete(role);
	}

	getEl(role: ElementRole): HTMLElement | null {
		return this.#registry.get(role) ?? null;
	}

	registerSlot(key: string, el: HTMLElement) {
		this.#slots.set(key, el);
	}

	unregisterSlot(key: string) {
		this.#slots.delete(key);
	}

	/** Svelte attachment factory: `{@attach bus.slotAttachment("local:5")}`. */
	slotAttachment(key: string) {
		return (node: Element) => {
			this.registerSlot(key, node as HTMLElement);
			return () => this.unregisterSlot(key);
		};
	}

	/** Snapshot the current screen positions of every registered slot. */
	snapshotSlots(): Map<string, Point> {
		const snap = new Map<string, Point>();
		for (const [key, el] of this.#slots) {
			snap.set(key, centerOf(el.getBoundingClientRect()));
		}
		return snap;
	}

	/** Resolve a flight target to a concrete screen-space point. */
	resolvePoint(target: FlightTarget): Point {
		if ("point" in target) return target.point;

		if ("slot" in target) {
			const el = this.#slots.get(target.slot);
			if (el) return centerOf(el.getBoundingClientRect());
		}

		if ("role" in target) {
			const el = this.#registry.get(target.role);
			if (el) return centerOf(el.getBoundingClientRect());
		}

		return { x: window.innerWidth / 2, y: window.innerHeight / 2 };
	}

	flights = $state<CardFlightEvent[]>([]);
	#nextKey = 0;

	launch(event: Pick<Card, "type" | "value"> & {
		turned: boolean;
		from: FlightTarget;
		to: FlightTarget;
	}) {
		const src = this.resolvePoint(event.from);
		const dst = this.resolvePoint(event.to);
		this.flights = [
			...this.flights,
			{
				key: this.#nextKey++,
				type: event.type,
				value: event.value,
				turned: event.turned,
				src,
				dst
			}
		];
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
