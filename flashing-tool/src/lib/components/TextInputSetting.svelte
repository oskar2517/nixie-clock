<script lang="ts">
    import Setting from "./Setting.svelte";
    import eyeIcon from "../../assets/icon/eye.svg";

    interface Props {
        name: string;
        description?: string;
        disabled?: boolean;
        value: string;
        type?: "text" | "password" | "number" | "time";
        minLength?: number;
        maxLength?: number;
        step?: number;
        pattern?: string;
        onchange?: () => void;
    }

    let {
        name,
        value = $bindable(),
        description,
        disabled,
        type = "text",
        minLength,
        maxLength,
        step,
        pattern,
        onchange,
    }: Props = $props();

    const originalType = type;

    function revealPassword() {
        if (originalType !== "password") return;

        if (type === "text") {
            type = "password";
        } else {
            type = "text";
        }
    }
</script>

<Setting {name} {disabled} {description}>
    <div class="wrapper">
        <input
            class="text-input-setting"
            {type}
            minlength={minLength}
            maxlength={maxLength}
            {step}
            placeholder={name}
            bind:value
            {onchange}
            {pattern}
        />
        {#if originalType === "password"}
            <button
                type="button"
                class="button-reveal"
                onclick={revealPassword}
            >
                <img src={eyeIcon} alt="reveal" />
            </button>
        {/if}
    </div>
</Setting>

<style>
    .wrapper {
        position: relative;
    }

    .text-input-setting {
        all: unset;
        width: 100%;
        background-color: #313131;
        padding: 5px 10px;
        box-sizing: border-box;
        border: solid 1px #444444;
        border-radius: 5px;
    }

    .text-input-setting:invalid {
        border-color: #C63C37;
    }

    .button-reveal {
        all: unset;
        position: absolute;
        right: 10px;
        top: 50%;
        transform: translateY(-50%);
        cursor: pointer;
    }

    .button-reveal img {
        height: 28px;
        opacity: 0.7;
    }
</style>
