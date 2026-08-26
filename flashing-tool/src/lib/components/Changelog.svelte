<script lang="ts">
    import Category from "./Category.svelte";

    interface Props {
        changelog: string[];
    }

    const { changelog }: Props = $props();
</script>

<Category title="Changes">
    <ul class="changes">
        {#each changelog as c}
            <li class="change">{c}</li>
        {/each}
    </ul>
</Category>

<style>
    .changes {
        max-height: 400px;
        overflow: auto;
    }

    .change {
        white-space: pre-wrap;
        word-break: break-word;
        margin: 5px 0px 5px 20px;
    }
</style>
