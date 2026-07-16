<script lang="ts">
    import type { Snippet } from "svelte";

    interface Props {
        title: string;
        children?: Snippet;
    }

    const { title, children }: Props = $props();

    let expanded = $state(false);
</script>

<div class="setting-group">
    <button class="title" onclick={() => (expanded = !expanded)}>{title}</button
    >
    {#if expanded}
        <div class="settings">
            {@render children?.()}
        </div>
    {/if}
</div>

<style>
    .setting-group {
        background-color: #181818;
        width: 700px;
        max-width: calc(100vw - 30px);
        margin: 10px 0;
        border: solid 1px #444444;
    }

    .title {
        all: unset;
        box-sizing: border-box;
        padding: 10px;
        font-weight: 700;
        width: 100%;
        background-image: url("../../../assets/icon/expand.svg");
        background-repeat: no-repeat;
        background-size: 20px;
        background-position: 99% center;
        cursor: pointer;
    }

    .settings {
        border-top: solid 1px #444444;
        padding: 10px;
        display: flex;
        flex-direction: column;
        gap: 20px;
    }

    .settings > :global(* + *) {
        position: relative;
    }

    .settings > :global(* + *)::before {
        content: "";
        position: absolute;
        top: -10px;
        left: 0;
        right: 0;
        border-top: solid 1px #444444;
    }
</style>
