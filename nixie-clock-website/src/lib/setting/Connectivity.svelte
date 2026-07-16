<script lang="ts">
    import Button from "./settings/Button.svelte";
    import SettingGroup from "./settings/SettingGroup.svelte";
    import TextInputSetting from "./settings/TextInputSetting.svelte";
</script>

<SettingGroup title="Connectivity">
    <TextInputSetting name="SSID" value=""></TextInputSetting>
    <TextInputSetting name="Password" value="" type="password"
    ></TextInputSetting>
    <Button name="Connect"></Button>
</SettingGroup>
