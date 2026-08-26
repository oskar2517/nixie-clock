<script lang="ts">
    import type { IEspLoaderTerminal } from "esptool-js";
    import ButtonPrimary from "./lib/components/ButtonPrimary.svelte";
    import Heading from "./lib/components/Heading.svelte";
    import Paragraph from "./lib/components/Paragraph.svelte";
    import Log from "./lib/components/Log.svelte";
    import {
        connectToEsp,
        dumpPartition,
        flashPartition,
        mountLittleFs,
        readClockConfig,
        readInstalledFirmwareVersion,
        readPartitionTable,
        type ClockConfig,
        type EspConnection,
    } from "./lib/esp-interface";
    import Progress from "./lib/components/Progress.svelte";
    import E from "./lib/components/E.svelte";
    import {
        downloadFirmware,
        fetchReleases,
        type FirmwareRelease,
    } from "./lib/github";
    import SelectSetting from "./lib/components/SelectSetting.svelte";
    import type { EspPartition } from "./lib/partition";
    import {
        removeFileRecursively,
        type LittleFsImageReader,
    } from "./lib/littlefs";
    import { extractZipToLittleFs } from "./lib/zip";
    import TextInputSetting from "./lib/components/TextInputSetting.svelte";
    import Changelog from "./lib/components/Changelog.svelte";
    import Hr from "./lib/components/Hr.svelte";

    interface TProgress {
        progress: number;
        text: string;
    }

    interface ClockInfo {
        chipName: string;
        installedFirmwareVersion: string;
    }

    const WIFI_AP_PASSWORD_REGEX = "^[\\x20-\\x7E]+$";

    let log: Log;
    let littleFsData: Uint8Array<ArrayBufferLike> | undefined =
        $state(undefined);
    let lfs: LittleFsImageReader | undefined = $state(undefined);
    let connection: EspConnection | undefined = $state(undefined);
    let terminal: IEspLoaderTerminal | undefined = $state(undefined);
    let error = $state("");
    let updateStarted = $state(false);
    let progress: TProgress | undefined = $state(undefined);
    let clockInfo: ClockInfo | undefined = $state(undefined);
    let readyToInstall = $state(false);
    let availableReleases: FirmwareRelease[] = $state([]);
    let selectedRelease: string = $state("");
    let partitionTable: EspPartition[] = $state([]);
    let finished = $state(false);
    let wifiApPassword = $state("");
    let clockConfig: ClockConfig | undefined = $state(undefined);

    async function beginUpdate() {
        terminal = {
            clean() {
                log.clear();
            },
            writeLine(data: string) {
                log.writeLine(data);
            },
            write(data: string) {
                log.write(data);
            },
        };

        try {
            connection = await connectToEsp(terminal);
            const { chipName, loader } = connection;

            updateStarted = true;

            progress = {
                progress: 0,
                text: "Preparing...",
            };

            partitionTable = await readPartitionTable(loader, terminal);

            const littleFsPartition = partitionTable.find(
                (e) => e.name === "spiffs",
            );

            if (!littleFsPartition) {
                throw new Error("Could not find little-fs partition!");
            }

            terminal.writeLine(
                `Found little-fs partition at 0x${littleFsPartition.offset.toString(16)}`,
            );

            littleFsData = await dumpPartition(
                loader,
                terminal,
                littleFsPartition,
                (packet, prog, totalSize) => {
                    progress = {
                        progress: prog / totalSize,
                        text: "Dumping little-fs...",
                    };
                },
            );

            progress = undefined;

            lfs = await mountLittleFs(
                terminal,
                littleFsPartition,
                littleFsData,
            );

            terminal.writeLine("Reading clock config...");
            clockConfig = await readClockConfig(lfs);
            wifiApPassword = clockConfig.wifiApPassword ?? "";

            const installedFirmwareVersion =
                await readInstalledFirmwareVersion(lfs);

            terminal.writeLine(
                `Detected installed firmware version: ${installedFirmwareVersion ?? "unknown"}`,
            );

            clockInfo = {
                chipName,
                installedFirmwareVersion: installedFirmwareVersion ?? "unknown",
            };

            availableReleases = await fetchReleases();
            if (availableReleases.length === 0) {
                throw new Error("No firmware releases found in manifest");
            }

            selectedRelease = availableReleases[0].name;
            readyToInstall = true;
        } catch (err: any) {
            error = err.toString();
        }
    }

    async function installRelease() {
        if (
            terminal === undefined ||
            connection === undefined ||
            lfs === undefined ||
            littleFsData === undefined ||
            wifiApPassword.length < 8 ||
            clockConfig === undefined
        )
            return;

        readyToInstall = false;

        progress = {
            progress: 0,
            text: "Preparing...",
        };

        try {
            const { loader } = connection;
            const relevantRelease = availableReleases.find(
                (r) => r.name === selectedRelease,
            );

            if (!relevantRelease) {
                throw new Error(
                    `Selected release not found: ${selectedRelease}`,
                );
            }

            terminal.writeLine(
                `Downloading firmware release ${selectedRelease}...`,
            );

            // TODO: Don't use hardcoded esp32s3 release..
            const relevantFirmware = relevantRelease.assets.firmware.esp32s3;

            if (!relevantFirmware) throw new Error("Firmware not found!");

            const appPartitions = partitionTable.filter((e) => e.type === 0x00);

            if (appPartitions.length === 0) {
                throw new Error("Failed to detect app partition");
            }

            if (appPartitions.length > 1) {
                terminal.writeLine(
                    `Found ${appPartitions.length} app partitions; flashing ${appPartitions[0].name}`,
                );
            }

            const appPartition = appPartitions[0];

            const firmwareBytes = await downloadFirmware(relevantFirmware);

            if (firmwareBytes.length > appPartition.size) {
                throw new Error(
                    `Firmware is too large: ${firmwareBytes.length} bytes, app partition ${appPartition.name} is ${appPartition.size} bytes`,
                );
            }

            terminal.writeLine("Downloading dashboard...");
            const dashboardBytes = await downloadFirmware(
                relevantRelease.assets.dashboard,
            );

            terminal.writeLine("Patching little-fs...");
            terminal.writeLine("Patching config...");
            clockConfig.wifiApPassword = wifiApPassword;
            await lfs.writeFile(
                "/config.json",
                new TextEncoder().encode(JSON.stringify(clockConfig)),
            );

            terminal.writeLine("Patching dashboard...");

            terminal.writeLine("Removing old dashboard...");
            await removeFileRecursively(lfs, "/dashboard");

            const dashboardBlob = new Blob([
                dashboardBytes.buffer.slice(
                    dashboardBytes.byteOffset,
                    dashboardBytes.byteOffset + dashboardBytes.byteLength,
                ),
            ]);

            terminal.writeLine("Extracting dashboard archive to little-fs...");
            await extractZipToLittleFs("/dashboard", dashboardBlob, lfs);
            terminal.writeLine("Extracted dashboard successfully.");

            const littleFsPartition = partitionTable.find(
                (e) => e.name === "spiffs",
            );

            if (!littleFsPartition) {
                throw new Error("Could not find little-fs partition!");
            }

            progress = {
                progress: 0,
                text: "Preparing...",
            };

            await flashPartition(
                loader,
                terminal,
                appPartition,
                firmwareBytes,
                (fileIndex, written, total) => {
                    progress = {
                        progress: written / total,
                        text: "Flashing firmware...",
                    };
                },
            );

            await flashPartition(
                loader,
                terminal,
                littleFsPartition,
                littleFsData,
                (fileIndex, written, total) => {
                    progress = {
                        progress: written / total,
                        text: "Flashing patched little-fs...",
                    };
                },
            );

            progress = undefined;
            finished = true;
        } catch (err: any) {
            error = err.toString();
        }
    }
</script>

<div class="main">
    <Heading>Nixie Clock Update Tool</Heading>
    <Paragraph
        >This website allows you to automatically update the firmware of your
        Nixie clock.</Paragraph
    >
    <Paragraph
        >Connect your clock to this computer via USB and follow the instructions
        below.</Paragraph
    >

    <Hr></Hr>

    {#if error !== ""}
        <Paragraph highlight={true}>{error}</Paragraph>
        <Hr></Hr>
    {/if}

    {#if clockInfo !== undefined}
        <Paragraph><E>Chip Name:</E> {clockInfo.chipName}</Paragraph>
        <Paragraph
            ><E>Current Firmware Version:</E>
            {clockInfo.installedFirmwareVersion}</Paragraph
        >
        <Hr></Hr>
    {/if}

    {#if progress !== undefined}
        <Progress progress={progress.progress} text={progress.text}></Progress>
    {/if}

    {#if !updateStarted}
        <ButtonPrimary name="Begin Update" onclick={beginUpdate}
        ></ButtonPrimary>
    {/if}

    {#if readyToInstall}
        <SelectSetting
            name="Release"
            description="This is the software version that will be installed. It is recommended to always use the latest version."
            options={availableReleases.map((r) => r.name)}
            bind:value={selectedRelease}
        ></SelectSetting>

        <Changelog
            changelog={availableReleases.find((r) => r.name === selectedRelease)
                ?.changes ?? []}
        ></Changelog>

        <TextInputSetting
            name="WiFi AP Password"
            minLength={8}
            maxLength={32}
            pattern={WIFI_AP_PASSWORD_REGEX}
            bind:value={wifiApPassword}
            description="This is the password that will be required to connect to the clock's configuration WiFi network."
            type="password"
        ></TextInputSetting>

        <ButtonPrimary
            disabled={wifiApPassword.length < 8 ||
                !new RegExp(WIFI_AP_PASSWORD_REGEX).test(wifiApPassword)}
            name="Install Selected Release"
            onclick={installRelease}
        ></ButtonPrimary>
    {/if}

    {#if finished}
        <Paragraph
            >Successfully installed release <E>{selectedRelease}</E>.</Paragraph
        >
    {/if}

    <Log bind:this={log}></Log>
</div>

<style>
    .main {
        margin: 0 auto;
        width: 900px;
        max-width: 98vw;
        margin-bottom: 50px;
    }
</style>
