export interface WiFiResponse {
    ssid: string;
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
}

export type ClockConfigUpdate = Partial<ClockConfig>;

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

export async function updateConfig(update: ClockConfigUpdate): Promise<ClockConfig> {
    const response = await createRequest("POST", "/api/config", JSON.stringify(update));

    if (!response.ok) {
        throw new Error("Failed to update config");
    }

    return await response.json();
}
