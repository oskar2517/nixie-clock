<script lang="ts">
    import { onMount } from "svelte";
    import { getTimerConfig, setTimer, setTimerInterval } from "../../api";
    import { notification } from "./common/notification_store";
    import SettingGroup from "./settings/SettingGroup.svelte";
    import SwitchSetting from "./settings/SwitchSetting.svelte";
    import TextInputSetting from "./settings/TextInputSetting.svelte";

    let timer = $state(false);

    let tubesOffTime = $state("");
    let tubesOnTime = $state("");

    function splitTime(time: string): number[] {
        return time.split(":").map((n) => parseInt(n));
    }

    async function handleIntervalChange() {
        if (!tubesOffTime || !tubesOnTime) return;

        const [tubesOffHours, tubesOffMinutes] = splitTime(tubesOffTime);
        const [tubesOnHours, tubesOnMinutes] = splitTime(tubesOnTime);

        try {
            await setTimerInterval(
                tubesOffHours,
                tubesOffMinutes,
                tubesOnHours,
                tubesOnMinutes,
            );
            $notification = {
                severity: "normal",
                message: `Set timer interval to ${tubesOffTime} - ${tubesOnTime}.`,
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: err.toString(),
            };
        }
    }

    async function handleTimerEnabledChange() {
        try {
            await setTimer(timer);
            $notification = {
                severity: "normal",
                message: timer ? "Enabled timer" : "Disabled timer",
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: err.toString(),
            };
        }
    }

    onMount(async () => {
        const config = await getTimerConfig();

        timer = config.timer;
        tubesOffTime = `${config.tubesOffHours}:${config.tubesOffMinutes}`;
        tubesOffTime = `${config.tubesOnHours}:${config.tubesOnMinutes}`;
    });
</script>

<SettingGroup title="Timer">
    <SwitchSetting
        name="Timer"
        bind:value={timer}
        description="Turns the Nixie tubes off during a specified time interval (e.g., at night)."
        onchange={handleTimerEnabledChange}
    ></SwitchSetting>

    <TextInputSetting
        name="Tubes Off Time"
        bind:value={tubesOffTime}
        type="time"
        onchange={handleIntervalChange}
    ></TextInputSetting>

    <TextInputSetting
        name="Tubes On Time"
        bind:value={tubesOnTime}
        type="time"
        onchange={handleIntervalChange}
    ></TextInputSetting>
</SettingGroup>
