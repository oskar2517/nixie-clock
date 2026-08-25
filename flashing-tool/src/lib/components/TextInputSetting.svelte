<script lang="ts">
    interface Props {
        name: string;
        value: string;
        type?: "text" | "password" | "number" | "time";
        min?: number;
        max?: number;
        step?: number;
        onchange?: () => void;
    }

    let {
        name,
        value = $bindable(),
        type = "text",
        min,
        max,
        step,
        onchange,
    }: Props = $props();
</script>

<div class="setting">
    <div class="name">
        {name}
    </div>
    <input
        class="text-input-setting"
        {type}
        {min}
        {max}
        {step}
        placeholder={name}
        bind:value
        {onchange}
    />
</div>

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

    .name {
        margin-bottom: 5px;
    }

    .setting {
        margin: 15px 0;
    }
</style>
