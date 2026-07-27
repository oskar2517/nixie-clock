<script lang="ts">
    import NixieClock from "./lib/nixie/NixieClock.svelte";
    import Advanced from "./lib/setting/Advanced.svelte";
    import Display from "./lib/setting/Display.svelte";
    import Connectivity from "./lib/setting/Connectivity.svelte";
    import TimeDate from "./lib/setting/TimeDate.svelte";
    import Timer from "./lib/setting/Timer.svelte";
    import Notification from "./lib/setting/common/Notification.svelte";
    import { onMount } from "svelte";
    import { getFirmware } from "./api";
    import Diagnostics from "./lib/setting/Diagnostics.svelte";

    let firmwareVersion = $state("");

    onMount(async () => {
        firmwareVersion = (await getFirmware()).version;
    });
</script>

<div class="main">
    <NixieClock />

    <p>Running firmware version {firmwareVersion}</p>

    <Connectivity />

    <TimeDate />

    <Display />

    <Timer />

    <Advanced />

    <Diagnostics />

    <Notification />
</div>

<style>
    .main {
        display: flex;
        flex-direction: column;
        align-items: center;
    }
</style>
