<script lang="ts">
    interface Props {
        progress: number;
        text: string;
    }

    const { progress, text }: Props = $props();
</script>

<div class="progress" style="--progress: {progress};">
    ({Math.floor(progress * 100)}%) {text}
</div>

<style>
    .progress {
        background-image: linear-gradient(
            to right,
            #0078d4 0 50%,
            #181818 50% 100%
        );
        border: solid 1px #444444;
        color: white;
        padding: 10px;
        margin: 15px 0;
        background-size: 200% 100%;
        background-position: calc(100% - var(--progress) * 100%) 0;
        transition: ease background-position 0.2s;
    }
</style>
