<script lang="ts">
    interface Props {
        name: string;
        value: string;
        type?: "text" | "password";
                disabled?: boolean;
        onchange?: () => void;
    }

    let {
        name,
        value = $bindable(),
        type = "text",
        disabled = false,
        onchange,
    }: Props = $props();
</script>

<div class="text-input-setting" class:disabled>
    <div class="name">{name}</div>
    <input class="input" {type} placeholder={name} bind:value {onchange} />
</div>

<style>
    .text-input-setting.disabled {
        pointer-events: none;
        opacity: .5;
    }

    .name {
        margin-bottom: 2px;
    }

    .input {
        all: unset;
        width: 100%;
        background-color: #313131;
        padding: 5px 10px;
        box-sizing: border-box;
        border: solid 1px #444444;
        border-radius: 5px;
    }
</style>
