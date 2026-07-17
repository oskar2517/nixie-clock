<script lang="ts">
    interface Props {
        message: string;
        error?: boolean;
    }

    const { message, error = false }: Props = $props();
</script>

<div class="result" class:error>{message}</div>

<style>
    .result {
        color: #3FBEB0;
    }

    .error {
        color: #b31d28;
    }
</style>
