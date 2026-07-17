import { writable, type Writable } from "svelte/store";

interface Notification {
    message: string;
    severity: "normal" | "error"
}

export const notification: Writable<Notification | null> = writable(null);