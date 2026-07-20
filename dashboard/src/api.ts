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

export interface AutomaticTimeResponse {
    automatic: boolean;
}

export interface TimeDateConfig {
    timeDisplayFormat: 12 | 24,
    timezonePosix: string;
    timezoneIana: string;
    automaticTime: boolean;
}

export interface TimerConfig {
    timer: boolean;
    tubesOffHours: number;
    tubesOffMinutes: number;
    tubesOnHours: number;
    tubesOnMinutes: number;
}

export interface TimerResponse {
    timer: boolean;
}

export interface TimerIntervalResponse {
    tubesOffHours: number;
    tubesOffMinutes: number;
    tubesOnHours: number;
    tubesOnMinutes: number;
}

export interface AdvancedConfig {
    ntpServer: string;
    ntpFrequency: number;
    healingMode: boolean;
}

export interface NtpServerResponse {
    server: string;
}

export interface NtpFrequencyResponse {
    frequency: number;
}

export interface HealingModeResponse {
    healingMode: boolean;
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

export async function getTimeDateConfig(): Promise<TimeDateConfig> {
    const response = await createRequest("GET", "/api/config/time_date");

    return await response.json();
}

export async function setTimezone(posix: string, iana: string): Promise<TimezoneResponse> {
    const response = await createRequest("POST", "/api/config/time_date/timezone", JSON.stringify({
        posix,
        iana
    }));

    if (!response.ok) {
        throw new Error("Failed to set timezone");
    }

    return await response.json();
}

export async function setTimeDisplayFormat(format: 12 | 24): Promise<TimeDisplayFormatResponse> {
    const response = await createRequest("POST", "/api/config/time_date/time_display_format", JSON.stringify({
        format
    }));

    if (!response.ok) {
        throw new Error("Failed to set timezone");
    }

    return await response.json();
}

export async function setAutomaticTime(automatic: boolean): Promise<AutomaticTimeResponse> {
    const response = await createRequest("POST", "/api/config/time_date/automatic_time", JSON.stringify({
        automatic
    }));

    if (!response.ok) {
        throw new Error("Failed to update automatic time");
    }

    return await response.json();
}

export async function getTimerConfig(): Promise<TimerConfig> {
    const response = await createRequest("GET", "/api/config/timer");

    return await response.json();
}

export async function setTimer(timer: boolean): Promise<TimerResponse> {
    const response = await createRequest("POST", "/api/config/timer/timer", JSON.stringify({
        timer
    }));

    if (!response.ok) {
        throw new Error("Failed to toggle timer");
    }

    return await response.json();
}

export async function setTimerInterval(
    tubesOffHours: number,
    tubesOffMinutes: number,
    tubesOnHours: number,
    tubesOnMinutes: number
): Promise<TimerIntervalResponse> {
    const response = await createRequest("POST", "/api/config/timer/interval", JSON.stringify({
        tubesOffHours,
        tubesOffMinutes,
        tubesOnHours,
        tubesOnMinutes
    }));

    if (!response.ok) {
        throw new Error("Failed to update timer interval");
    }

    return await response.json();
}

export async function getAdvancedConfig(): Promise<AdvancedConfig> {
    const response = await createRequest("GET", "/api/config/advanced");

    return await response.json();
}

export async function setNtpServer(server: string): Promise<NtpServerResponse> {
    const response = await createRequest("POST", "/api/config/advanced/ntp_server", JSON.stringify({
        server
    }));

    if (!response.ok) {
        throw new Error("Failed to update NTP server");
    }

    return await response.json();
}

export async function setNtpFrequency(frequency: number): Promise<NtpFrequencyResponse> {
    const response = await createRequest("POST", "/api/config/advanced/ntp_frequency", JSON.stringify({
        frequency
    }));

    if (!response.ok) {
        throw new Error("Failed to update NTP frequency");
    }

    return await response.json();
}

export async function setHealingMode(healingMode: boolean): Promise<HealingModeResponse> {
    const response = await createRequest("POST", "/api/config/advanced/healing_mode", JSON.stringify({
        healingMode
    }));

    if (!response.ok) {
        throw new Error("Failed to toggle healing mode");
    }

    return await response.json();
}