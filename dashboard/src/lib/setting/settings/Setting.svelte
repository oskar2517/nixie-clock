<script lang="ts">
    import type { Snippet } from "svelte";

    interface Props {
        name: string;
        description?: string;
        disabled?: boolean;
        children: Snippet;
    }

    const { name, description, disabled = false, children }: Props = $props();
</script>

<div class="setting" class:disabled>
    <div class="name">{name}</div>
    {#if description}
        <div class="description">{description}</div>
    {/if}
    {@render children()}
</div>

<style>
    .disabled {
        pointer-events: none;
        opacity: 0.5;
    }

    .name {
        margin-bottom: 5px;
    }

    .description {
        color: #979797;
        font-size: 14px;
        margin-bottom: 5px;
    }
</style>
