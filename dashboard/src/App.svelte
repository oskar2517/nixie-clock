<script lang="ts">
    import NixieClock from "./lib/nixie/NixieClock.svelte";
    import Advanced from "./lib/setting/Advanced.svelte";
    import Display from "./lib/setting/Display.svelte";
    import Connectivity from "./lib/setting/Connectivity.svelte";
    import TimeDate from "./lib/setting/TimeDate.svelte";
    import Timer from "./lib/setting/Timer.svelte";
    import Notification from "./lib/setting/common/Notification.svelte";
</script>

<div class="main">
    <NixieClock />

    <Connectivity />

    <TimeDate />

    <Display />

    <Timer />

    <Advanced />

    <Notification />
</div>

<style>
    .main {
        display: flex;
        flex-direction: column;
        align-items: center;
    }
</style>
