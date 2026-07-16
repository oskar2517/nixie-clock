<script>
    import SettingGroup from "./settings/SettingGroup.svelte";
    import SwitchSetting from "./settings/SwitchSetting.svelte";
    import TextInputSetting from "./settings/TextInputSetting.svelte";

    let timer = $state(false);
</script>

<SettingGroup title="Timer">
    <SwitchSetting
        name="Timer"
        bind:value={timer}
        description="Turns the Nixie tubes off during a specified time interval (e.g., at night)."
    ></SwitchSetting>

    <TextInputSetting
        name="Tubes Off Time"
        value=""
        type="time"
        disabled={!timer}
    ></TextInputSetting>

    <TextInputSetting
        name="Tubes On Time"
        value=""
        type="time"
        disabled={!timer}
    ></TextInputSetting>
</SettingGroup>
