<script lang="ts">
    import type { Snippet } from "svelte";

    interface Props {
        children: Snippet;
    }

    const {children}: Props = $props();
</script>

<span>{@render children()}</span>

<style>
    span {
        color: #e8ae04;
    }
</style>