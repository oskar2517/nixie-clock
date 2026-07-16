<script>
    import SettingGroup from "./settings/SettingGroup.svelte";
    import SwitchSetting from "./settings/SwitchSetting.svelte";
    import TextInputSetting from "./settings/TextInputSetting.svelte";
</script>

<SettingGroup title="Advanced">
    <TextInputSetting name="NTP Server" value="pool.ntp.org"></TextInputSetting>

    <TextInputSetting
        name="NTP Synchronization Frequency"
        value="30"
        description="How often, in minutes, the current time is fetched from the configured NTP server."
        type="number"
    ></TextInputSetting>

    <SwitchSetting
        name="Healing Mode"
        description="When enabled, the clock will cycle through all digits on every tube. This may heal cathode poisoning."
        value={false}
    ></SwitchSetting>
</SettingGroup>
