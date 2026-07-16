<script>
    import NixieClock from "./lib/nixie/NixieClock.svelte";
    import Button from "./lib/setting/Button.svelte";
    import SettingGroup from "./lib/setting/SettingGroup.svelte";
    import SwitchSetting from "./lib/setting/SwitchSetting.svelte";
    import TextInputSetting from "./lib/setting/TextInputSetting.svelte";

    let wifi = $state(false);
</script>

<div class="main">
    <NixieClock />

    <SettingGroup title="Connectivity">
        <SwitchSetting name="WiFi Enabled" bind:state={wifi}></SwitchSetting>
        <TextInputSetting name="SSID" value="" disabled={!wifi}></TextInputSetting>
        <TextInputSetting name="Password" value="" disabled={!wifi} type="password"
        ></TextInputSetting>
        <Button name="Test Credentials" disabled={!wifi}></Button>
    </SettingGroup>
    <SettingGroup title="Date & Time"></SettingGroup>
    <SettingGroup title="Anti Cathode Poisoning"></SettingGroup>
    <SettingGroup title="Night Mode"></SettingGroup>
</div>

<style>
    .main {
        display: flex;
        flex-direction: column;
        align-items: center;
    }
</style>
