<script lang="ts">
    import { onMount } from "svelte";
    import SettingGroup from "./settings/SettingGroup.svelte";
    import SwitchSetting from "./settings/SwitchSetting.svelte";
    import TextInputSetting from "./settings/TextInputSetting.svelte";
    import {
        getAdvancedConfig,
        setHealingMode,
        setNtpFrequency,
        setNtpServer,
    } from "../../api";
    import { notification } from "./common/notification_store";

    let ntpServer = $state("");
    let ntpFrequency = $state("");
    let healingMode = $state(false);

    onMount(async () => {
        const config = await getAdvancedConfig();

        ntpServer = config.ntpServer;
        ntpFrequency = config.ntpFrequency.toString();
        healingMode = config.healingMode;
    });

    async function handleNtpServerChange() {
        if (ntpServer === "") return;

        try {
            const response = await setNtpServer(ntpServer);
            ntpServer = response.server;
            $notification = {
                severity: "normal",
                message: `Set NTP server to ${ntpServer}`,
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: err.toString(),
            };
        }
    }

    async function handleSyncFreqChange() {
        if (ntpFrequency === "") return;

        try {
            const response = await setNtpFrequency(parseInt(ntpFrequency));
            ntpFrequency = response.frequency.toString();
            $notification = {
                severity: "normal",
                message: `Set NTP sychronization frequency to ${ntpFrequency} minutes`,
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: err.toString(),
            };
        }
    }

    async function handleHealingModeChange() {
        try {
            const response = await setHealingMode(healingMode);
            healingMode = response.healingMode;
            $notification = {
                severity: "normal",
                message: healingMode ? "Enabled healing mode" : "Disabled healing mode"
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: err.toString(),
            };
        }
    }
</script>

<SettingGroup title="Advanced">
    <TextInputSetting
        name="NTP Server"
        bind:value={ntpServer}
        onchange={handleNtpServerChange}
    ></TextInputSetting>

    <TextInputSetting
        name="NTP Synchronization Frequency"
        bind:value={ntpFrequency}
        description="How often, in minutes, the current time is fetched from the configured NTP server."
        type="number"
        onchange={handleSyncFreqChange}
    ></TextInputSetting>

    <SwitchSetting
        name="Healing Mode"
        description="When enabled, the clock will cycle through all digits on every tube. This may heal cathode poisoning."
        bind:value={healingMode}
        onchange={handleHealingModeChange}
    ></SwitchSetting>
</SettingGroup>
