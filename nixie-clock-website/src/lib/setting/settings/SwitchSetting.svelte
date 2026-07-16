<script lang="ts">
    import Setting from "./Setting.svelte";

    interface Props {
        name: string;
        description?: string;
        value: boolean;
        disabled?: boolean;
        onchange?: () => void;
    }

    let {
        name,
        description,
        value = $bindable(),
        disabled,
        onchange,
    }: Props = $props();

    function handleToggle() {
        value = !value;

        onchange?.();
    }
</script>

<Setting {name} {description} {disabled}>
    <!-- svelte-ignore a11y_click_events_have_key_events -->
    <!-- svelte-ignore a11y_no_static_element_interactions -->
    <div class="switch-setting" onclick={handleToggle}>
        <div class="switch">
            <div class="state" class:active={value}>On</div>
            <div class="state" class:active={!value}>Off</div>
        </div>
    </div>
</Setting>

<style>
    .switch-setting {
        display: flex;
        column-gap: 10px;
        align-items: center;
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
