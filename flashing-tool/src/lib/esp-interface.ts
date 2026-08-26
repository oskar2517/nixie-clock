import { ESPLoader, Transport, type FlashOptions, type IEspLoaderTerminal, type LoaderOptions } from "esptool-js";
import { parseEspPartitions, PARTITION_TABLE_OFFSET, PARTITION_TABLE_SIZE, type EspPartition } from "./partition";
import { createLittleFsImageReaderFromImage, type LittleFsImageReader } from "./littlefs";

const BAUDRATE = 115200;
const DEBUG_LOGGING = false;
const ENABLE_TRACING = false;

export interface EspConnection {
    loader: ESPLoader;
    chipName: string;
}

export interface ClockConfig {
    wifiApPassword: string;
}

type DumpProgress = (packet: Uint8Array<ArrayBufferLike>, progress: number, totalSize: number) => void;

type FlashProgress = (fileIndex: number, written: number, total: number) => void;

export async function connectToEsp(terminal: IEspLoaderTerminal): Promise<EspConnection> {
    const port = await navigator.serial.requestPort();
    const transport = new Transport(port, true);

    const loaderOptions: LoaderOptions = {
        transport,
        baudrate: BAUDRATE,
        terminal,
        debugLogging: DEBUG_LOGGING,
        enableTracing: ENABLE_TRACING,
    };

    const esploader = new ESPLoader(loaderOptions);

    try {
        const chipName = await esploader.main();

        return {
            loader: esploader,
            chipName
        };
    } catch (error) {
        throw new Error(`Failed to connect: ${error}`);
    }
}

export async function readPartitionTable(
    loader: ESPLoader,
    terminal: IEspLoaderTerminal,
    progressCb?: DumpProgress
): Promise<EspPartition[]> {
    terminal.writeLine("Reading partition table...");

    const partitionTableData = await loader.readFlash(
        PARTITION_TABLE_OFFSET,
        PARTITION_TABLE_SIZE,
        (packet, progress, totalSize) => {
            terminal.writeLine(`Read: ${progress}/${totalSize} bytes`);

            progressCb?.(packet, progress, totalSize);
        },
    );

    terminal.writeLine("Successfully read partition table.");

    const partitionTable = parseEspPartitions(partitionTableData);

    terminal.writeLine(`Parsed partition table: ${JSON.stringify(partitionTable)}`);

    return partitionTable;
}

export async function dumpPartition(
    loader: ESPLoader,
    terminal: IEspLoaderTerminal,
    partition: EspPartition,
    progressCb?: DumpProgress
): Promise<Uint8Array<ArrayBufferLike>> {
    terminal.writeLine(`Dumping partition ${partition.name} (0x${partition.offset.toString(16)})...`);

    const data = await loader.readFlash(
        partition.offset,
        partition.size,
        (packet, progress, totalSize) => {
            terminal.writeLine(`Read: ${progress}/${totalSize} bytes`);

            progressCb?.(packet, progress, totalSize);
        },
    );

    terminal.writeLine(`Successfully dumped partition ${partition.name}.`);

    return data;
}

export async function flashPartition(
    loader: ESPLoader,
    terminal: IEspLoaderTerminal,
    partition: EspPartition,
    data: Uint8Array<ArrayBufferLike>,
    progressCb?: FlashProgress
) {
    terminal.writeLine(`Flashing partition ${partition.name} (0x${partition.offset.toString(16)})...`);

    const flashOptions: FlashOptions = {
        fileArray: [{ data, address: partition.offset }],
        flashMode: "qio",
        flashFreq: "80m",
        flashSize: "4MB",
        eraseAll: false,
        compress: true,
        reportProgress: (fileIndex, written, total) => {
            terminal.writeLine(`Flashed: ${written}/${total} bytes`);

            progressCb?.(fileIndex, written, total);
        },
    };

    await loader.writeFlash(flashOptions);

    terminal.writeLine(`Successfully flashed partition ${partition.name}.`);
}

export async function mountLittleFs(
    terminal: IEspLoaderTerminal,
    partition: EspPartition,
    data: Uint8Array<ArrayBufferLike>
): Promise<LittleFsImageReader> {
    terminal.writeLine("Mounting little-fs...")

    const blockSize = 4096;

    const lfs = await createLittleFsImageReaderFromImage(
        data,
        {
            readSize: 16,
            cacheSize: 256,
            lookaheadSize: 16,
            blockSize,
            blockCount: partition.size / blockSize,
        },
        {
            partitionOffset: 0,
        },
    );

    terminal.writeLine("little-fs mounted successfully.");

    return lfs;
}

export async function readInstalledFirmwareVersion(lfs: LittleFsImageReader): Promise<string | null> {
    try {
        const installedFirmwareVersion = await lfs.readFile("VERSION");
        return new TextDecoder().decode(installedFirmwareVersion);
    } catch (err) {
        return null;
    }
}

export async function readClockConfig(lfs: LittleFsImageReader): Promise<ClockConfig> {
    const config = JSON.parse(
        new TextDecoder().decode(await lfs.readFile("/config.json")),
    );

    return config;
}