<script lang="ts">
    interface Props {
        name: string;
        disabled?: boolean;
        onclick?: () => void;
    }

    const { name, disabled = false, onclick }: Props = $props();
</script>

<button class:disabled type="button" onclick={() => onclick?.()}>{name}</button>

<style>
    button {
        all: unset;
        background-color: #0078d4;
        border-radius: 5px;
        padding: 5px 10px;
        text-align: center;
        color: white;
    }

    button:hover {
        background-color: #026cbd;
    }

    button.disabled {
        pointer-events: none;
        opacity: .5;
    }
</style>
