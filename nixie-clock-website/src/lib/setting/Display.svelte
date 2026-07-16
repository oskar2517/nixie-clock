<script>
    import SelectSetting from "./settings/SelectSetting.svelte";
    import SettingGroup from "./settings/SettingGroup.svelte";
</script>

<SettingGroup title="Display">
    <SelectSetting
        name="Seconds Neons Pattern"
        options={["Blink", "Toggle", "Disabled"]}
        value="Blink"
        description="Behavior of the neon bulbs indicating seconds."
    ></SelectSetting>

    <SelectSetting
        name="Anti Cathode Poisoning Routine"
        options={["Slot machine", "From left ro right"]}
        value="Slot machine"
        description="Animation style of the anti cathode poisoning routine."
    ></SelectSetting>
</SettingGroup>
