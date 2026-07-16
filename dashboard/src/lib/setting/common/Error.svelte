<script lang="ts">
    interface Props {
        message: string;
    }

    const { message }: Props = $props();
</script>

<div class="error">{message}</div>

<style>
    .error {
        color: #b31d28;
    }
</style>
