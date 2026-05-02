import { writable, type Writable } from "svelte/store";

export type AppSocketAction = "join" | "query" | "click";
export interface AppSocketData {
  username?: string;
  room?: string;
}

export default class SocketManager {
  private static instance: SocketManager;
  public socket: WebSocket | null = null;

  public syncedSocketData: AppSocketData;
  public connectionStatus: Writable<
    "disconnected" | "connecting" | "connected"
  >;

  private constructor() {
    this.syncedSocketData = $state({});
    this.connectionStatus = writable("disconnected");
  }

  public static getInstance(): SocketManager {
    if (!SocketManager.instance) {
      SocketManager.instance = new SocketManager();
    }
    return SocketManager.instance;
  }

  public async connect(): Promise<WebSocket> {
    if (
      this.socket &&
      (this.socket.readyState === WebSocket.OPEN ||
        this.socket.readyState === WebSocket.CONNECTING)
    ) {
      return this.socket;
    }

    const host = window.location.host;
    const protocol = window.location.protocol === "https:" ? "wss:" : "ws:";
    const socketUrl = `${protocol}//${host}/ws`;

    this.socket = new WebSocket(socketUrl);
    this.connectionStatus.set("connecting");

    return new Promise((resolve, reject) => {
      this.socket!.addEventListener(
        "open",
        () => {
          this.connectionStatus.set("connected");
          resolve(this.socket!);
        },
        { once: true },
      );

      this.socket!.addEventListener(
        "error",
        (err) => {
          reject(err);
        },
        { once: true },
      );

      this.socket!.addEventListener("message", (event: MessageEvent) => {
        if (!String(event.data).startsWith("{")) return;
        try {
          const eventData = JSON.parse(event.data);

          if (eventData.action === "sync_data") {
            this.syncedSocketData = {
              room: eventData.room,
              username: eventData.username,
            };
          }
        } catch (e) {
          console.error(e);
        }
      });

      this.socket!.onclose = () => {
        this.connectionStatus.set("disconnected");
        this.socket = null;

        console.log("WS Disconnesso");
      };
    });
  }

  public emit(action: string, payload: object): void {
    if (this.socket?.readyState === WebSocket.OPEN) {
      this.socket.send(JSON.stringify({ action, ...payload }));
    }
  }

  /**
   * @brief emette un payload con un'azione verso il server, e attende una risposta di tipo `expectedAction`
   *  @param action l'azione che il server interpreterà
   *  @param payload l'oggetto JSON che verrà mandato al server
   *  @param expectedAction l'azione che ci aspettiamo di ricevere indietro dal server, o `any` per una qualsiasi risposta (ATTENZIONE, potrebbe essere una risposta non correlata al messaggio emesso)
   */
  public async emitAndWait(
    action: string,
    payload: object,
    expectedAction: "any" | string,
    timeoutMs: number = 5000,
  ): Promise<any> {
    this.emit(action, payload);
    return this.waitFor(expectedAction, timeoutMs);
  }

  private async waitFor(
    expectedAction: "any" | string,
    timeoutMs: number = 5000,
  ): Promise<any> {
    return new Promise((resolve, reject) => {
      const timeout = setTimeout(() => {
        cleanup();
        reject(
          new Error(
            `Timeout: attesa per un messaggio di tipo: '${expectedAction}' scaduta.`,
          ),
        );
      }, timeoutMs);

      const handler = (event: MessageEvent) => {
        try {
          const data = JSON.parse(event.data);
          if (expectedAction === "any" || data.action === expectedAction) {
            cleanup();
            resolve(data);
          }
        } catch (e) { }
      };

      const cleanup = () => {
        clearTimeout(timeout);
        this.socket?.removeEventListener("message", handler);
      };

      this.socket?.addEventListener("message", handler);
    });
  }
}
