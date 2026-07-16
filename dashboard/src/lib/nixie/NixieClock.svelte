<script lang="ts">
    import { onMount } from "svelte";
    import Tube from "./Tube.svelte";

    let digits: number[] = $state([]);

    function convertTimePart(p: number) {
        if (p < 10) {
            return [0, p];
        }

        return [Math.floor(p / 10), p % 10];
    }

    function updateTime() {
                const date = new Date();
        const hours = convertTimePart(date.getHours());
        const minutes = convertTimePart(date.getMinutes());
        const seconds = convertTimePart(date.getSeconds());

        digits = [...hours, ...minutes, ...seconds];
    }

    setInterval(updateTime, 500);

    onMount(updateTime);
</script>

<div class="display">
{#each digits as digit}
    <Tube {digit} />
    {/each}
</div>

<style>
    .display {
        margin-top: 40px;
        margin-bottom: 20px;
    }
</style>
