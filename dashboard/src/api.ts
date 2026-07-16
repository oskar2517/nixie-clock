interface WiFiResponse {
    ssid: string;
}

function createRequest(method: "POST" | "GET", route: string, body: string): Promise<Response> {
    return fetch(route, {
        method,
        headers: {
            "Content-Type": "application/json"
        },
        body
    });
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