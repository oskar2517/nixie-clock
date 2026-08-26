<script lang="ts">
    import Setting from "./Setting.svelte";

    interface Props {
        options: string[];
        value: string;
        name: string;
        description?: string;
        disabled?: boolean;
        onchange?: () => void;
    }

    let {
        options,
        name,
        value = $bindable(),
        onchange,
        description,
        disabled,
    }: Props = $props();
</script>

<Setting {name} {description} {disabled}>
    <select class="select-setting" bind:value {onchange}>
        {#each options as o}
            <option value={o}>{o}</option>
        {/each}
    </select>
</Setting>

<style>
    .select-setting {
        all: unset;
        width: 100%;
        background-color: #313131;
        padding: 5px 10px;
        box-sizing: border-box;
        border: solid 1px #444444;
        border-radius: 5px;
        background-image: url("../../assets/icon/arrow.svg");
        background-repeat: no-repeat;
        background-size: 20px;
        background-position: 99% center;
        cursor: pointer;
    }
</style>
