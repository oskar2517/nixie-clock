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
    import SelectSetting from "./settings/SelectSetting.svelte";

    const WIFI_TRANSMISSION_POWERS = [
        {
            displayName: "19.5 dBm",
            value: 78,
        },
        {
            displayName: "19 dBm",
            value: 76,
        },
        {
            displayName: "18.5 dBm",
            value: 74,
        },
        {
            displayName: "17 dBm",
            value: 68,
        },
        {
            displayName: "15 dBm",
            value: 60,
        },
        {
            displayName: "13 dBm",
            value: 52,
        },
        {
            displayName: "11 dBm",
            value: 44,
        },
        {
            displayName: "8.5 dBm",
            value: 34,
        },
        {
            displayName: "7 dBm",
            value: 28,
        },
        {
            displayName: "5 dBm",
            value: 20,
        },
        {
            displayName: "2 dBm",
            value: 8,
        },
        {
            displayName: "-1 dBm",
            value: -4,
        },
    ];

    const wifiTransmissionPowerOptions = WIFI_TRANSMISSION_POWERS.map(
        (power) => power.displayName,
    );

    function getWifiTransmissionPowerName(value: number): string {
        return (
            WIFI_TRANSMISSION_POWERS.find((power) => power.value === value)
                ?.displayName ?? ""
        );
    }

    function getWifiTransmissionPowerValue(name: string): number {
        const value = WIFI_TRANSMISSION_POWERS.find(
            (p) => p.displayName === name,
        )?.value;

        if (value === undefined) {
            throw new Error(`WiFi power value ${name} invalid`);
        }

        return value;
    }

    let ntpServer = $state("");
    let ntpFrequency = $state("");
    let neonsFrequency = $state("");
    let neonsBrightness = $state("");
    let healingMode = $state(false);
    let wifiIdleTransmissionPower = $state("");
    let wifiConnectedTransmissionPower = $state("");

    onMount(async () => {
        const config = await getConfig();

        ntpServer = config.ntpServer;
        ntpFrequency = config.ntpFrequency.toString();
        healingMode = config.healingMode;
        neonsFrequency = config.neonsFrequency.toString();
        neonsBrightness = config.neonsBrightness.toString();
        wifiIdleTransmissionPower = getWifiTransmissionPowerName(
            config.wifiIdleTransmissionPower,
        );
        wifiConnectedTransmissionPower = getWifiTransmissionPowerName(
            config.wifiConnectedTransmissionPower,
        );
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

    async function handleWiFiIdleTransmissionPowerChange() {
        if (wifiIdleTransmissionPower === "") return;

        try {
            const response = await updateConfig({
                wifiIdleTransmissionPower: getWifiTransmissionPowerValue(
                    wifiIdleTransmissionPower,
                ),
            });
            wifiIdleTransmissionPower = getWifiTransmissionPowerName(
                response.wifiIdleTransmissionPower,
            );
            $notification = {
                severity: "normal",
                message: `Set idle WiFi power to ${wifiIdleTransmissionPower}`,
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: notificationErrorMessage(err),
            };
        }
    }

    async function handleWiFiConnectedTransmissionPowerChange() {
        if (wifiConnectedTransmissionPower === "") return;

        try {
            const response = await updateConfig({
                wifiConnectedTransmissionPower: getWifiTransmissionPowerValue(
                    wifiConnectedTransmissionPower,
                ),
            });
            wifiConnectedTransmissionPower = getWifiTransmissionPowerName(
                response.wifiConnectedTransmissionPower,
            );
            $notification = {
                severity: "normal",
                message: `Set connected WiFi power to ${wifiConnectedTransmissionPower}`,
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

    <SelectSetting
        name="WiFi Idle Transmission Power"
        bind:value={wifiIdleTransmissionPower}
        description="Maximum WiFi transmission power while the clock is not connected to another access point."
        options={wifiTransmissionPowerOptions}
        onchange={handleWiFiIdleTransmissionPowerChange}
    ></SelectSetting>

    <SelectSetting
        name="WiFi Connected Transmission Power"
        bind:value={wifiConnectedTransmissionPower}
        description="Maximum WiFi transmission power while the clock is connected to another access point."
        options={wifiTransmissionPowerOptions}
        onchange={handleWiFiConnectedTransmissionPowerChange}
    ></SelectSetting>

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
