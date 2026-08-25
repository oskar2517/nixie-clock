<script lang="ts">
    import type { Snippet } from "svelte";

    interface Props {
        children: Snippet;
    }

    const { children }: Props = $props();
</script>

<h1>{@render children()}</h1>

<style>
    h1 {
        color: var(--highlight-color);
        margin-top: 50px;
        font-weight: 700;
        margin-bottom: 20px;
        font-size: 40px;
    }
</style>
