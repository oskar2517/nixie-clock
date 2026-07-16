<script lang="ts">
    // Based on https://codepen.io/lucitribal/pen/bRzKEv

    interface Props {
        digit: number;
    }

    const { digit }: Props = $props();
</script>

<div class="tube">
    <div class="letter">{digit}</div>
</div>

<style>
    .tube {
        display: inline-block;
        position: relative;
        height: 80px;
        width: 50px;
        border-radius: 20px 20px 0 0;
        padding-top: 8px;
        margin: 0px 5px 16px 0;
        box-shadow:
            inset 0px 1px 3px 2px rgba(90, 0, 0, 0.9),
            inset 0px 2px 2px 3px rgba(230, 220, 0, 1),
            inset 0px 4px 4px 3px rgba(255, 0, 0, 0.9);
    }

    .tube:nth-child(2n) {
        margin-right: 20px;
    }

    .tube:last-child {
        margin-right: 0;
    }

    .tube::after {
        content: "";
        display: block;
        position: absolute;
        top: 100%;
        width: 100%;
        height: 16px;
        background-color: gray;
        border-radius: 0 0 35% 35%;
        background-image: radial-gradient(
            40% 100%,
            #ea4 0%,
            #f93 15%,
            #caa 50%,
            #778 75%,
            #345 100%
        );
    }

    .letter {
        line-height: 70px;
        font-size: 65px;
        text-align: center;
        transition:
            ease-out color 0.5s,
            ease-out text-shadow 0.5s;
        color: transparent;
        text-shadow:
            0 0 1px rgba(254, 252, 124, 1),
            0 0 3px rgba(255, 217, 54, 0.7),
            0 0 5px rgba(255, 0, 0, 1),
            0 0 14px rgba(255, 0, 0, 1),
            0 0 17px rgba(255, 0, 0, 1),
            0 0 20px rgba(255, 0, 0, 1),
            0 5px 25px rgba(0, 0, 255, 0.9);
    }
</style>
