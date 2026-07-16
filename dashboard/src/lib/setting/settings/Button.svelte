<script lang="ts">
    interface Props {
        name: string;
        disabled?: boolean;
        busy?: boolean;
        onclick?: () => void;
    }

    const { name, disabled = false, busy = false, onclick }: Props = $props();
</script>

<button class:disabled class:busy type="button" onclick={() => onclick?.()}
    >{name}</button
>

<style>
    button {
        all: unset;
        background-color: #0078d4;
        border-radius: 5px;
        padding: 5px 10px;
        text-align: center;
        color: white;
        cursor: pointer;
        position: relative;
    }

    button:hover {
        background-color: #026cbd;
    }

    button.disabled {
        pointer-events: none;
        opacity: 0.5;
    }

    .busy::after {
        content: "";
        display: block;
        height: 20px;
        width: 20px;
        background-image: url("../../../assets/icon/spinner.svg");
        background-repeat: no-repeat;
        background-size: 20px;
        position: absolute;
        right: 10px;
        top: 50%;
        transform: translateY(-50%);
        animation: ease spin 1s forwards infinite;
    }

    @keyframes spin {
        from {
            transform: translateY(-50%) rotate(0deg);
        }

        to {
            transform: translateY(-50%) rotate(360deg);
        }
    }
</style>
