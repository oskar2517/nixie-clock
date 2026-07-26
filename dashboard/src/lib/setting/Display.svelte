<script lang="ts">
    import { onMount } from "svelte";
    import SelectSetting from "./settings/SelectSetting.svelte";
    import SettingGroup from "./settings/SettingGroup.svelte";
    import { getConfig, updateConfig } from "../../api";
    import { notification } from "./common/notification_store";

    const neonsModes = new Map([
        ["Disabled", 0],
        ["Blink", 1],
        ["Toggle", 2],
    ]);

    let neonsMode = $state("");

    async function handleNeonsModeChange() {
        let neonsModeNumber = neonsModeNameToNumber(neonsMode);

        try {
            const response = await updateConfig({
                neonsMode: neonsModeNumber,
            });
            neonsMode = neonsModeNumberToName(response.neonsMode);
            $notification = {
                severity: "normal",
                message: `Set neons mode to ${neonsMode}`,
            };
        } catch (err: any) {
            $notification = {
                severity: "error",
                message: err.toString(),
            };
        }
    }

    function neonsModeNumberToName(n: number): string {
        return [...neonsModes].find(([_, value]) => value === n)!![0];
    }

    function neonsModeNameToNumber(s: string): number {
        return neonsModes.get(s)!!;
    }

    onMount(async () => {
        const config = await getConfig();
        neonsMode = neonsModeNumberToName(config.neonsMode);
    });
</script>

<SettingGroup title="Display">
    <SelectSetting
        name="Seconds Neons Pattern"
        options={["Blink", "Toggle", "Disabled"]}
        bind:value={neonsMode}
        onchange={handleNeonsModeChange}
        description="Behavior of the neon bulbs indicating seconds."
    ></SelectSetting>

    <SelectSetting
        name="Anti Cathode Poisoning Routine"
        options={["Slot machine", "From left ro right"]}
        value="Slot machine"
        description="Animation style of the anti cathode poisoning routine."
    ></SelectSetting>
</SettingGroup>
