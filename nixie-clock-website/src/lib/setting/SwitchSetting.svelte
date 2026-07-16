<script lang="ts">
    interface Props {
        name: string;
        state: boolean;
        disabled?: boolean;
        onchange?: () => void;
    }

    let {
        name,
        state = $bindable(),
        disabled = false,
        onchange,
    }: Props = $props();

    function handleToggle() {
        state = !state;

        onchange?.();
    }
</script>

<!-- svelte-ignore a11y_click_events_have_key_events -->
<!-- svelte-ignore a11y_no_static_element_interactions -->
<div class="switch-setting" class:disabled onclick={handleToggle}>
    <div class="switch">
        <div class="state" class:active={state}>On</div>
        <div class="state" class:active={!state}>Off</div>
    </div>

    <div class="name">{name}</div>
</div>

<style>
    .switch-setting {
        display: flex;
        column-gap: 10px;
        align-items: center;
    }

    .switch-setting.disabled {
        pointer-events: none;
        opacity: 0.5;
    }

    .switch {
        display: grid;
        grid-template-columns: 1fr 1fr;
        border: solid 1px #444444;
        border-radius: 5px;
        overflow: hidden;
    }

    .state {
        text-align: center;
        padding: 5px 10px;
    }

    .state.active {
        background-color: #0078d4;
    }
</style>
