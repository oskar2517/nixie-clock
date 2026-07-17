<script lang="ts">
    import Button from "./settings/Button.svelte";
    import SettingGroup from "./settings/SettingGroup.svelte";
    import TextInputSetting from "./settings/TextInputSetting.svelte";
    import {
        getWifiStatus,
        setupWifi as setupWifiApi,
        forgetWifi as forgetWifiApi,
        type WiFiResponse,
    } from "../../api";
    import { onMount } from "svelte";
    import Result from "./common/Result.svelte";

    let ssid = $state("");
    let password = $state("");

    let error = $state("");
    let connecting = $state(false);
    let wifiStatus: WiFiResponse | null = $state(null);
 
    onMount(async () => {
        try {
            wifiStatus = await getWifiStatus();
        } catch (err) {}
    });

    async function setupWifi() {
        try {
            connecting = true;
            wifiStatus = await setupWifiApi(ssid, password);
            connecting = false;
            error = "";
            ssid = "";
            password = "";
        } catch (err: any) {
            connecting = false;
            error = err.toString();
        }
    }

    async function forgetWifi() {
        try {
            await forgetWifiApi();
            wifiStatus = null;
            error = "";
            ssid = "";
            password = "";
        } catch (err: any) {
            error = err;
        }
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
    {:else}
        <TextInputSetting
            name="SSID"
            description="This has to be the exact name of the WiFi network you want to connect to."
            disabled={connecting}
            bind:value={ssid}
        ></TextInputSetting>

        <TextInputSetting
            name="Password"
            type="password"
            bind:value={password}
            disabled={connecting}
        ></TextInputSetting>

        <Button
            name="Setup WiFi"
            busy={connecting}
            disabled={connecting || ssid.length === 0}
            onclick={setupWifi}
        ></Button>
    {/if}

    {#if error !== ""}
        <Result error={true} message={error}></Result>
    {/if}
</SettingGroup>

<style>
    .higlight {
        color: white;
    }
</style>
