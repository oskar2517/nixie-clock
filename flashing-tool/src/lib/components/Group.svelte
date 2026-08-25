<script lang="ts">
    import type { Snippet } from "svelte";

    interface Props {
        title?: string;
        disabled?: boolean;
        children: Snippet;
    }

    const { title, disabled = false, children }: Props = $props();
</script>

<div class="group" class:disabled>
    {#if title}
        <div class="title">{title}</div>
    {/if}

    <div class="content">
        {@render children()}
    </div>
</div>

<style>
    .group {
        border: 1px solid #444;
    }

    .disabled {
        pointer-events: none;
        opacity: 0.5;
    }

    .title {
        padding: 10px;
        border-bottom: 1px solid #444;
    }

    .content {
        padding: 10px;
    }
</style>
