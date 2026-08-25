<script lang="ts">
    import { tick } from "svelte";
    import Category from "./Category.svelte";

    let log = $state<string[]>([]);
    let messagesElement: HTMLDivElement | undefined = $state(undefined);

    function isScrolledToBottom() {
        if (!messagesElement) return true;

        const distanceFromBottom =
            messagesElement.scrollHeight -
            messagesElement.scrollTop -
            messagesElement.clientHeight;

        return distanceFromBottom <= 4;
    }

    async function scrollToBottomIfNeeded(shouldScroll: boolean) {
        if (!shouldScroll) return;

        await tick();

        if (!messagesElement) return;

        messagesElement.scrollTop = messagesElement.scrollHeight;
    }

    export function clear() {
        log = [];
    }

    export async function write(message: string) {
        const shouldScroll = isScrolledToBottom();
        log = [...log, message];
        await scrollToBottomIfNeeded(shouldScroll);
    }

    export async function writeLine(message: string) {
        await write(message);
    }
</script>

{#if log.length > 0}
    <Category title="Log" expanded={true}>
        <div class="messages" bind:this={messagesElement}>
            {#each log as message}
                <div class="message">{message}</div>
            {/each}
        </div>
    </Category>
{/if}

<style>
    .messages {
        max-height: 400px;
        overflow: auto;
    }

    .message {
        white-space: pre-wrap;
        word-break: break-word;
    }
</style>
