<script lang="ts">
    import Button from "./settings/Button.svelte";
    import SettingGroup from "./settings/SettingGroup.svelte";
    import TextInputSetting from "./settings/TextInputSetting.svelte";
    import {
        getWifiStatus,
        setupWifi as setupWifiApi,
        forgetWifi as forgetWifiApi,
    } from "../../api";
    import { onMount } from "svelte";

    let ssid = $state("");
    let password = $state("");

    let error = $state("");
    let setupSsid = $state("");

    onMount(async () => {
        try {
            const status = await getWifiStatus();
            setupSsid = status.ssid;
        } catch (err) {}
    });

    async function setupWifi() {
        try {
            const response = await setupWifiApi(ssid, password);
            setupSsid = response.ssid;
            error = "";
        } catch (err: any) {
            error = err.toString();
        }
    }

    async function forgetWifi() {
        try {
            await forgetWifiApi();
            setupSsid = "";
            error = "";
        } catch (err: any) {
            error = err;
        }
    }
</script>

<SettingGroup title="Connectivity">
    {#if setupSsid !== ""}
        <span>Setup for WiFi {setupSsid}</span>
        <Button name="Forget WiFi" onclick={forgetWifi}></Button>
    {:else}
        <TextInputSetting
            name="SSID"
            description="This has to be the exact name of the WiFi network you want to connect to."
            bind:value={ssid}
        ></TextInputSetting>

        <TextInputSetting name="Password" type="password" bind:value={password}
        ></TextInputSetting>

        <Button name="Setup WiFi" onclick={setupWifi}></Button>
    {/if}

    {#if error !== ""}
        <span class="error">{error}</span>
    {/if}
</SettingGroup>
