<script lang="ts">
    import Button from "./settings/Button.svelte";
    import SettingGroup from "./settings/SettingGroup.svelte";
    import TextInputSetting from "./settings/TextInputSetting.svelte";
    import {
        getWifiStatus,
        setupWifi as setupWifiApi,
        forgetWifi as forgetWifiApi,
        type WiFiResponse,
        type WiFiNetwork,
        scanWifiNetworks,
    } from "../../api";
    import { onMount } from "svelte";
    import { notification } from "./common/notification_store";

    let ssid = $state("");
    let password = $state("");

    let connecting = $state(false);
    let scanning = $state(false);
    let wifiStatus: WiFiResponse | null = $state(null);
    let foundNetworks: WiFiNetwork[] = $state([]);
    let hasScanned = $state(false);

    onMount(async () => {
        try {
            wifiStatus = await getWifiStatus();
        } catch (err) {}
    });

    function reset() {
        ssid = "";
        password = "";
        connecting = false;
        scanning = false;
        foundNetworks = [];
        hasScanned = false;
    }

    async function setupWifi() {
        try {
            connecting = true;
            wifiStatus = await setupWifiApi(ssid, password);
            connecting = false;
            ssid = "";
            password = "";
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: err.toString(),
            };
            connecting = false;
            password = "";
        }
    }

    async function forgetWifi() {
        try {
            await forgetWifiApi();
            wifiStatus = null;
            reset();
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: err.toString(),
            };
            reset();
        }
    }

    async function handleScanWiFiClick() {
        try {
            scanning = true;
            foundNetworks = await scanWifiNetworks();
            scanning = false;
            hasScanned = true;
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: err.toString(),
            };
            reset();
        }
    }

    function handleCancelClick() {
        ssid = "";
        password = "";
    }
</script>

<SettingGroup title="Connectivity">
    {#if wifiStatus !== null}
        <div class="network-config">
            <div>
                Configured access point:
                <span class="higlight">{wifiStatus.ssid}</span>
            </div>
        </div>
        <Button name="Forget WiFi" onclick={forgetWifi}></Button>
    {:else if ssid === ""}
        {#if foundNetworks.length === 0}
            {#if hasScanned}
                <p>No WiFi networks found.</p>
            {:else}
                <p>Scan for WiFi networks to begin.</p>
            {/if}
        {:else}
            <div class="networks" class:disabled={scanning}>
                {#each foundNetworks as n}
                    <!-- svelte-ignore a11y_click_events_have_key_events -->
                    <!-- svelte-ignore a11y_no_static_element_interactions -->
                    <div class="network" onclick={() => (ssid = n.ssid)}>
                        <span class="ssid">{n.ssid}</span>
                        <span class="auth-type"
                            >({n.open ? "Open" : "Encrypted"})</span
                        >
                    </div>
                {/each}
            </div>
        {/if}
        <Button
            name="Scan For WiFi Networks"
            onclick={handleScanWiFiClick}
            busy={scanning}
            disabled={scanning}
        ></Button>
    {:else}
        <TextInputSetting
            name="Password"
            type="password"
            bind:value={password}
            disabled={connecting}
            description="Enter the password for WiFi {ssid}"
        ></TextInputSetting>

        <Button
            name="Setup WiFi"
            busy={connecting}
            disabled={connecting || ssid.length === 0}
            onclick={setupWifi}
        ></Button>

        <Button name="Cancel" onclick={handleCancelClick} disabled={connecting}
        ></Button>
    {/if}
</SettingGroup>

<style>
    .higlight {
        color: white;
    }

    .networks {
        max-height: 250px;
        overflow: auto;
        border: 1px solid #444;
        padding: 5px;
    }

    .networks.disabled {
        pointer-events: none;
        opacity: 0.5;
    }

    .network {
        padding: 5px;
        cursor: pointer;
    }

    .network:hover {
        background-color: #222323;
    }

    .auth-type {
        color: #979797;
        font-size: 14px;
    }
</style>
