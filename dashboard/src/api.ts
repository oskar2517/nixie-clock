export interface WiFiResponse {
    ssid: string;
}

export interface TimezoneResponse {
    iana: string;
    posix: string;
}

export interface TimeDisplayFormatResponse {
    format: 12 | 24;
}

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

export async function getTimezone(): Promise<TimezoneResponse> {
    const response = await createRequest("GET", "/api/config/timezone");

    return await response.json();
}

export async function setTimezone(posix: string, iana: string): Promise<TimezoneResponse> {
    const response = await createRequest("POST", "/api/config/timezone", JSON.stringify({
        posix,
        iana
    }));

    if (!response.ok) {
        throw new Error("Failed to set timezone");
    }

    return await response.json();
}

export async function getTimeDisplayFormat(): Promise<TimeDisplayFormatResponse> {
    const response = await createRequest("GET", "/api/config/time_display_format");

    return await response.json();
}

export async function setTimeDisplayFormat(format: 12 | 24): Promise<TimeDisplayFormatResponse> {
    const response = await createRequest("POST", "/api/config/time_display_format", JSON.stringify({
        format
    }));

    if (!response.ok) {
        throw new Error("Failed to set timezone");
    }

    return await response.json();
}