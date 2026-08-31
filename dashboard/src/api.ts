export interface WiFiResponse {
    ssid: string;
}

export interface WiFiNetwork {
    ssid: string;
    rssi: number;
    channel: number;
    open: boolean;
}

export interface ClockConfig {
    timeDisplayFormat: 12 | 24,
    timezonePosix: string;
    timezoneIana: string;
    automaticTime: boolean;
    timer: boolean;
    tubesOffHours: number;
    tubesOffMinutes: number;
    tubesOnHours: number;
    tubesOnMinutes: number;
    ntpServer: string;
    ntpFrequency: number;
    healingMode: boolean;
    neonsMode: number;
    acpRoutine: number;
    neonsFrequency: number;
    neonsBrightness: number;
    digitCrossFade: boolean;
    wifiIdleTransmissionPower: number;
    wifiConnectedTransmissionPower: number;
}

export type ClockConfigUpdate = Partial<ClockConfig>;

export interface Firmware {
    version: string;
}

export type JsonValue =
    | string
    | number
    | boolean
    | null
    | JsonValue[]
    | { [key: string]: JsonValue };

export type DiagnosticsResponse = Record<string, JsonValue>;

function createRequest(method: "POST" | "GET" | "DELETE", route: string, body?: string): Promise<Response> {
    if (body) {
        return fetch(route, {
            method,
            headers: {
                "Content-Type": "application/json"
            },
            body
        });
    } else {
        return fetch(route, {
            method,
            headers: {
                "Content-Type": "application/json"
            }
        });
    }
}

export async function setupWifi(ssid: string, password: string): Promise<WiFiResponse> {
    const response = await createRequest("POST", "/api/wifi", JSON.stringify({
        ssid,
        password
    }));

    if (!response.ok) {
        throw new Error("Could not connect to WiFi");
    }

    return await response.json();
}

export async function getWifiStatus(): Promise<WiFiResponse> {
    const response = await createRequest("GET", "/api/wifi");

    if (!response.ok) {
        throw new Error("WiFi not setup");
    }

    return await response.json();
}

export async function forgetWifi(): Promise<void> {
    await createRequest("DELETE", "/api/wifi");
}

export async function scanWifiNetworks(): Promise<WiFiNetwork[]> {
    for (let attempt = 0; attempt < 60; attempt++) {
        const response = await createRequest("GET", "/api/wifi/networks");

        if (response.status === 202) {
            await new Promise((resolve) => setTimeout(resolve, 250));
            continue;
        }

        if (!response.ok) {
            throw new Error("Failed to scan for networks");
        }

        return await response.json();
    }

    throw new Error("Timed out scanning for networks");
}

export async function syncTime(timestamp: number): Promise<void> {
    const response = await createRequest("POST", "/api/time", JSON.stringify({
        timestamp
    }));

    if (!response.ok) {
        throw new Error("Failed to set time");
    }
}

export async function getConfig(): Promise<ClockConfig> {
    const response = await createRequest("GET", "/api/config");

    if (!response.ok) {
        throw new Error("Failed to load config");
    }

    return await response.json();
}

export async function resetConfig(): Promise<ClockConfig> {
    const response = await createRequest("DELETE", "/api/config");

    if (!response.ok) {
        throw new Error("Failed to reset config");
    }

    return await response.json();
}

export async function updateConfig(update: ClockConfigUpdate): Promise<ClockConfig> {
    const response = await createRequest("POST", "/api/config", JSON.stringify(update));

    if (!response.ok) {
        throw new Error("Failed to update config");
    }

    return await response.json();
}

export async function runAcpRoutine(): Promise<void> {
    const response = await createRequest("POST", "/api/acp_test");

    if (!response.ok) {
        throw new Error("Failed to run ACP routine");
    }
}

export async function getFirmware(): Promise<Firmware> {
    const response = await createRequest("GET", "/api/firmware");

    return response.json();
}

export async function getDisganostics(): Promise<DiagnosticsResponse> {
    const response = await createRequest("GET", "/api/diagnostics");

    if (!response.ok) {
        throw new Error("Failed to get diagnostics");
    }

    return response.json();
}
