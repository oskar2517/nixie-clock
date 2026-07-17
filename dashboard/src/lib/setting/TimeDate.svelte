<script lang="ts">
    import ct from "countries-and-timezones";

    import SelectSetting from "./settings/SelectSetting.svelte";
    import SettingGroup from "./settings/SettingGroup.svelte";
    import Button from "./settings/Button.svelte";
    import SwitchSetting from "./settings/SwitchSetting.svelte";
    import Result from "./common/Result.svelte";
    import { syncTime } from "../../api";

    const timezones = Object.keys(ct.getAllTimezones());

    let result = $state("");
    let error = $state(false);

    async function handleTimeSync() {
        try {
            const timestamp = Math.floor(Date.now() / 1000);

            await syncTime(timestamp);
            error = false;
            result = "Successfully set time";
        } catch (err: any) {
            error = true;
            result = err;
        }
    }
</script>

<SettingGroup title="Time & Date">
    <SelectSetting name="Timezone" value={timezones[0]} options={timezones}
    ></SelectSetting>

    <SelectSetting name="Time Display Format" options={["12-hour", "24-hour"]} value="24-hour"></SelectSetting>

    <SwitchSetting
        name="Set Time Automatically"
        description="Automatically set and update the clock using internet time (requires WiFi connection)."
        value={true}
    ></SwitchSetting>

    <Button name="Synchronize Time Now" onclick={handleTimeSync}></Button>

    {#if result !== ""}
        <Result {error} message={result}></Result>
    {/if}
</SettingGroup>
