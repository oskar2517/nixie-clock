<script lang="ts">
    import Setting from "./Setting.svelte";

    interface Props {
        name: string;
        description?: string;
        value: string;
        type?: "text" | "password" | "number" | "time";
        disabled?: boolean;
        onchange?: () => void;
    }

    let {
        name,
        description,
        value = $bindable(),
        type = "text",
        disabled = false,
        onchange,
    }: Props = $props();
</script>

<Setting {name} {description} {disabled}>
    <input class="text-input-setting" {type} placeholder={name} bind:value {onchange} />
</Setting>

<style>
    .text-input-setting {
        all: unset;
        width: 100%;
        background-color: #313131;
        padding: 5px 10px;
        box-sizing: border-box;
        border: solid 1px #444444;
        border-radius: 5px;
    }
</style>
