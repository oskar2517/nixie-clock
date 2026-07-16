<script>
    import NixieClock from "./lib/nixie/NixieClock.svelte";
    import AntiCathodePoisoning from "./lib/setting/AntiCathodePoisoning.svelte";
    import Connectivity from "./lib/setting/Connectivity.svelte";
    import Button from "./lib/setting/settings/Button.svelte";
    import SelectSetting from "./lib/setting/settings/SelectSetting.svelte";
    import SettingGroup from "./lib/setting/settings/SettingGroup.svelte";
    import TextInputSetting from "./lib/setting/settings/TextInputSetting.svelte";
    import TimeDate from "./lib/setting/TimeDate.svelte";
    import Timer from "./lib/setting/Timer.svelte";
</script>

<div class="main">
    <NixieClock />

    <Connectivity />

    <TimeDate />

    <AntiCathodePoisoning />

    <Timer />
</div>

<style>
    .main {
        display: flex;
        flex-direction: column;
        align-items: center;
    }
</style>
