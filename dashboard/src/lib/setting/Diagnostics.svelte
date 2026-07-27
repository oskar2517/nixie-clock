<script lang="ts">
    import { onMount } from "svelte";
    import { getDisganostics, type DiagnosticsResponse } from "../../api";
    import SettingGroup from "./settings/SettingGroup.svelte";
    import Group from "./common/Group.svelte";

    let diagnostics: DiagnosticsResponse = $state({});

    onMount(async () => {
        diagnostics = await getDisganostics();

        console.log(diagnostics);
    });
</script>

<SettingGroup title="Diagnostics">
    {#each Object.keys(diagnostics) as groupTitle}
        <Group title={groupTitle}>
            {#each Object.keys(diagnostics[groupTitle] as any) as name}
                <div class="diagnostic">
                    <div class="name">{name}</div>
                    <div class="value">
                        {(diagnostics[groupTitle] as any)[name]}
                    </div>
                </div>
            {/each}
        </Group>
    {/each}
</SettingGroup>

<style>
    .diagnostic {
        display: flex;
        justify-content: space-between;
    }

    .value {
        color: #979797;
    }
</style>
