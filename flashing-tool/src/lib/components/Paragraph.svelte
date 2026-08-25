<script lang="ts">
    import type { Snippet } from "svelte";

    interface Props {
        children: Snippet;
        highlight?: boolean;
    }

    const { children, highlight = false }: Props = $props();
</script>

<p class:highlight>{@render children()}</p>

<style>
    p {
        margin: 15px 0;
    }

    .highlight {
        color: #e8ae04;
    }
</style>
