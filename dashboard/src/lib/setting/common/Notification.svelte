<script lang="ts">
    import { fly } from "svelte/transition";
    import { notification } from "./notification_store";

    let timeout = $state(-1);

    notification.subscribe(() => {
        if (timeout !== -1) {
            clearTimeout(timeout);
            timeout = -1;
        }

        timeout = setTimeout(() => {
            $notification = null;
        }, 3000);
    });
</script>

{#if $notification !== null}
    <div class="notification" class:error={$notification.severity === "error"} transition:fly={{y: 20, duration: 200}}>
        {$notification.message}
    </div>
{/if}

<style>
    .notification {
        position: fixed;
        bottom: 0;
        left: 0;
        width: 100vh;
        color: white;
        background-color: #159D83;
        width: 100vw;
        padding: 30px 20px;
    }

    .error {
        background-color: #b31d28;
    }
</style>
