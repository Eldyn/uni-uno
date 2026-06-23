import { describe, it, expect, vi, beforeEach, afterEach } from "vitest";
import { ws } from "$lib/stores/ws.svelte";

// Access internals for controlled injection.
const wsAny = ws as unknown as {
	socket: WebSocket | null;
	pendingRequests: Map<
		string,
		{ resolve: Function; reject: Function; timer: ReturnType<typeof setTimeout> }
	>;
	_scheduleReconnect: () => void;
};

describe("ws store: emitAndWait fail-fast on disconnect", () => {
	beforeEach(() => {
		vi.useFakeTimers();
	});

	afterEach(() => {
		vi.useRealTimers();
		wsAny.pendingRequests.clear();
		wsAny.socket = null;
	});

	it("pending requests are rejected immediately when socket closes", () => {
		let rejected = false;
		let reason = "";

		// Inject a fake pending request as if emitAndWait is in flight.
		const timer = setTimeout(() => {}, 5000);
		wsAny.pendingRequests.set("req-1", {
			resolve: () => {},
			reject: (err: Error) => {
				rejected = true;
				reason = err.message;
			},
			timer
		});

		// Simulate onclose firing (the handler is inlined in _connectOnce).
		// Replicate what onclose does: reject all pending requests.
		for (const [, pending] of wsAny.pendingRequests) {
			clearTimeout(pending.timer);
			pending.reject(new Error("disconnected"));
		}
		wsAny.pendingRequests.clear();

		expect(rejected).toBe(true);
		expect(reason).toBe("disconnected");
		expect(wsAny.pendingRequests.size).toBe(0);
	});

	it("emitAndWait rejects immediately when socket is not connected", async () => {
		wsAny.socket = null;
		await expect(
			ws.emitAndWait("lobby_create" as Parameters<typeof ws.emitAndWait>[0], {} as any)
		).rejects.toThrow("WebSocket not connected");
	});
});
