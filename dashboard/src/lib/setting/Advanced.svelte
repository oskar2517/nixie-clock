<script lang="ts">
    import { onMount } from "svelte";
    import SettingGroup from "./settings/SettingGroup.svelte";
    import SwitchSetting from "./settings/SwitchSetting.svelte";
    import TextInputSetting from "./settings/TextInputSetting.svelte";
    import { getConfig, resetConfig, updateConfig } from "../../api";
    import {
        notification,
        notificationErrorMessage,
    } from "./common/notification_store";
    import Button from "./settings/Button.svelte";

    let ntpServer = $state("");
    let ntpFrequency = $state("");
    let neonsFrequency = $state("");
    let neonsBrightness = $state("");
    let healingMode = $state(false);

    onMount(async () => {
        const config = await getConfig();

        ntpServer = config.ntpServer;
        ntpFrequency = config.ntpFrequency.toString();
        healingMode = config.healingMode;
        neonsFrequency = config.neonsFrequency.toString();
        neonsBrightness = config.neonsBrightness.toString();
    });

    async function handleNtpServerChange() {
        if (ntpServer === "") return;

        try {
            const response = await updateConfig({
                ntpServer,
            });
            ntpServer = response.ntpServer;
            $notification = {
                severity: "normal",
                message: `Set NTP server to ${ntpServer}`,
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: notificationErrorMessage(err),
            };
        }
    }

    async function handleSyncFreqChange() {
        if (ntpFrequency === "") return;

        try {
            const response = await updateConfig({
                ntpFrequency: parseInt(ntpFrequency),
            });
            ntpFrequency = response.ntpFrequency.toString();
            $notification = {
                severity: "normal",
                message: `Set NTP sync to ${ntpFrequency} min`,
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: notificationErrorMessage(err),
            };
        }
    }

    async function handleHealingModeChange() {
        try {
            const response = await updateConfig({
                healingMode,
            });
            healingMode = response.healingMode;
            $notification = {
                severity: "normal",
                message: healingMode
                    ? "Enabled healing mode"
                    : "Disabled healing mode",
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: notificationErrorMessage(err),
            };
        }
    }

    async function handleResetClockConfigClick() {
        try {
            await resetConfig();
            window.location.reload();
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: notificationErrorMessage(err),
            };
        }
    }

    async function handleNeonsFrequencyChange() {
        if (neonsFrequency === "") return;

        try {
            const response = await updateConfig({
                neonsFrequency: parseInt(neonsFrequency),
            });
            neonsFrequency = response.neonsFrequency.toString();
            $notification = {
                severity: "normal",
                message: `Set neons PWM to ${neonsFrequency} Hz`,
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: notificationErrorMessage(err),
            };
        }
    }

    async function handleNeonsBrightnessChange() {
        if (neonsBrightness === "") return;

        try {
            const response = await updateConfig({
                neonsBrightness: parseInt(neonsBrightness),
            });
            neonsBrightness = response.neonsBrightness.toString();
            $notification = {
                severity: "normal",
                message: `Set neons brightness to ${neonsBrightness}%`,
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: notificationErrorMessage(err),
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

    <TextInputSetting
        name="Neons PWM Frequency"
        description="Selecting an appropriate frequency may help with flickering seconds indicators."
        bind:value={neonsFrequency}
        type="number"
        onchange={handleNeonsFrequencyChange}
    ></TextInputSetting>

    <TextInputSetting
        name="Neons Brightness"
        description="Brightness of the seconds indicator neons in percent."
        bind:value={neonsBrightness}
        type="number"
        min={0}
        max={100}
        step={1}
        onchange={handleNeonsBrightnessChange}
    ></TextInputSetting>

    <Button name="Reset Clock Config" onclick={handleResetClockConfigClick}
    ></Button>
</SettingGroup>
