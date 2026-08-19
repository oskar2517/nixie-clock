<script lang="ts">
    import { onMount } from "svelte";
    import SelectSetting from "./settings/SelectSetting.svelte";
    import SettingGroup from "./settings/SettingGroup.svelte";
    import { getConfig, runAcpRoutine, updateConfig } from "../../api";
    import {
        notification,
        notificationErrorMessage,
    } from "./common/notification_store";
    import Button from "./settings/Button.svelte";
    import SwitchSetting from "./settings/SwitchSetting.svelte";

    const ACP_CYCLE_THROUGH = "<Cycle through>";

    const neonsModes = ["Disabled", "Blink", "Toggle"];
    const acpRoutines = [
        "Basic",
        "Sweep",
        "Additive",
        "Slot Machine",
        "Centroid",
    ];

    let neonsMode = $state("");
    let acpRoutine = $state("");
    let digitCrossFade = $state(false);

    async function handleNeonsModeChange() {
        let neonsModeNumber = neonsModes.indexOf(neonsMode);

        try {
            const response = await updateConfig({
                neonsMode: neonsModeNumber,
            });
            neonsMode = neonsModes[response.neonsMode];
            $notification = {
                severity: "normal",
                message: `Set neons to ${neonsMode.toLowerCase()}`,
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: notificationErrorMessage(err),
            };
        }
    }

    async function handleacpRoutineChange() {
        let acpRoutineNumber = acpRoutines.indexOf(acpRoutine);

        try {
            const response = await updateConfig({
                acpRoutine: acpRoutineNumber,
            });
            acpRoutine = acpRoutines[response.acpRoutine] ?? ACP_CYCLE_THROUGH;
            $notification = {
                severity: "normal",
                message: `Set ACP to ${acpRoutine.replace(/[<>]/g, "").toLowerCase()}`,
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: notificationErrorMessage(err),
            };
        }
    }

    async function handleCrossFadeChange() {
        try {
            const response = await updateConfig({
                digitCrossFade,
            });
            digitCrossFade = response.digitCrossFade;
            $notification = {
                severity: "normal",
                message: `${digitCrossFade ? "Enabled" : "Disabled"} digit crossfade`,
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: notificationErrorMessage(err),
            };
        }
    }

    async function handleRunAcpRoutineClick() {
        await runAcpRoutine();
    }

    onMount(async () => {
        const config = await getConfig();
        neonsMode = neonsModes[config.neonsMode];
        acpRoutine = acpRoutines[config.acpRoutine] ?? ACP_CYCLE_THROUGH;
        digitCrossFade = config.digitCrossFade;
    });
</script>

<SettingGroup title="Display">
    <SelectSetting
        name="Seconds Neons Pattern"
        options={neonsModes}
        bind:value={neonsMode}
        onchange={handleNeonsModeChange}
        description="Behavior of the neon bulbs indicating seconds."
    ></SelectSetting>

    <SwitchSetting
        name="Digit Crossfade"
        bind:value={digitCrossFade}
        onchange={handleCrossFadeChange}
        description="When enabled, a displayed digit will not be updated immediatly but the previous one fades out and the next one fades in."
    ></SwitchSetting>

    <SelectSetting
        name="Anti Cathode Poisoning Routine"
        options={[ACP_CYCLE_THROUGH, ...acpRoutines]}
        bind:value={acpRoutine}
        onchange={handleacpRoutineChange}
        description="Animation style of the anti cathode poisoning routine."
    ></SelectSetting>

    <Button name="Run Selected Routine Now" onclick={handleRunAcpRoutineClick}
    ></Button>
</SettingGroup>
