import { writable, type Writable } from "svelte/store";

interface Notification {
    message: string;
    severity: "normal" | "error";
}

export const notification: Writable<Notification | null> = writable(null);

export function notificationErrorMessage(error: unknown): string {
    return error instanceof Error ? error.message : String(error);
}
