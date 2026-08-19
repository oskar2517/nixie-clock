<script lang="ts">
    import ct from "countries-and-timezones";

    import SelectSetting from "./settings/SelectSetting.svelte";
    import SettingGroup from "./settings/SettingGroup.svelte";
    import Button from "./settings/Button.svelte";
    import SwitchSetting from "./settings/SwitchSetting.svelte";
    import {
        getConfig,
        syncTime,
        updateConfig,
    } from "../../api";
    import { convertIanaToPosix } from "../../util/posix_ts";
    import { onMount } from "svelte";
    import {
        notification,
        notificationErrorMessage,
    } from "./common/notification_store";

    const timezones = Object.keys(ct.getAllTimezones());

    let timezoneIana = $state("");
    let timeDisplayFormat = $state("");
    let automaticTime = $state(false);

    async function handleTimeSync() {
        try {
            const timestamp = Math.floor(Date.now() / 1000);

            await syncTime(timestamp);
            $notification = {
                severity: "normal",
                message: "Set time",
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: notificationErrorMessage(err),
            };
        }
    }

    async function handleTimezoneChange() {
        if (timezoneIana === "") return;

        const timezonePosix = convertIanaToPosix(timezoneIana);

        if (!timezonePosix) {
            $notification = {
                severity: "error",
                message: "Unsupported timezone",
            };
            return;
        }

        try {
            const response = await updateConfig({
                timezonePosix,
                timezoneIana,
            });
            timezoneIana = response.timezoneIana;
            $notification = {
                severity: "normal",
                message: `Set timezone to ${timezoneIana}`,
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: notificationErrorMessage(err),
            };
        }
    }

    async function handleTimeDisplayFormatChange() {
        if (timeDisplayFormat === "") return;

        try {
            const format = timeDisplayFormat === "24-hour" ? 24 : 12;
            const response = await updateConfig({
                timeDisplayFormat: format,
            });
            timeDisplayFormat =
                response.timeDisplayFormat === 24 ? "24-hour" : "12-hour";
            $notification = {
                severity: "normal",
                message: `Set time format to ${timeDisplayFormat}`,
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: notificationErrorMessage(err),
            };
        }
    }

    async function handleAutomaticTimeChange() {
        try {
            const response = await updateConfig({
                automaticTime,
            });
            automaticTime = response.automaticTime;
            $notification = {
                severity: "normal",
                message: automaticTime
                    ? "Enabled automatic time"
                    : "Disabled automatic time",
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: notificationErrorMessage(err),
            };
        }
    }

    onMount(async () => {
        const config = await getConfig();
        timezoneIana = config.timezoneIana;
        timeDisplayFormat =
            config.timeDisplayFormat === 24 ? "24-hour" : "12-hour";
        automaticTime = config.automaticTime;
    });
</script>

<SettingGroup title="Time & Date">
    <SelectSetting
        name="Timezone"
        bind:value={timezoneIana}
        onchange={handleTimezoneChange}
        options={timezones}
    ></SelectSetting>

    <SelectSetting
        name="Time Display Format"
        options={["12-hour", "24-hour"]}
        bind:value={timeDisplayFormat}
        onchange={handleTimeDisplayFormatChange}
    ></SelectSetting>

    <SwitchSetting
        name="Set Time Automatically"
        description="Automatically set and update the clock using internet time (requires WiFi connection)."
        bind:value={automaticTime}
        onchange={handleAutomaticTimeChange}
    ></SwitchSetting>

    <Button name="Synchronize Time Now" onclick={handleTimeSync}></Button>
</SettingGroup>
