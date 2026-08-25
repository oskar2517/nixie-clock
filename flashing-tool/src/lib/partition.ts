export type EspPartition = {
    name: string;
    type: number;
    subtype: number;
    offset: number;
    size: number;
    flags: number;
};

export const PARTITION_TABLE_OFFSET = 0x8000;
export const PARTITION_TABLE_SIZE = 0xC00;
const ENTRY_SIZE = 32;

export function parseEspPartitions(
    flashDump: Uint8Array,
): EspPartition[] {
    const entries: EspPartition[] = [];
    const view = new DataView(
        flashDump.buffer,
        flashDump.byteOffset,
        flashDump.byteLength,
    );

    for (let pos = 0; pos < PARTITION_TABLE_SIZE; pos += ENTRY_SIZE) {
        const entry = flashDump.subarray(pos, pos + ENTRY_SIZE);

        if (entry.every((byte) => byte === 0xff)) break;

        // MD5 marker row: 0xeb 0xeb ... checksum. Usually followed by 0xff padding.
        if (entry[0] === 0xeb && entry[1] === 0xeb) break;

        // Partition entry magic bytes.
        if (entry[0] !== 0xaa || entry[1] !== 0x50) {
            throw new Error(`Invalid partition entry magic at 0x${pos.toString(16)}`);
        }

        const nameBytes = entry.subarray(12, 28);
        const zero = nameBytes.indexOf(0);
        const name = new TextDecoder().decode(
            zero === -1 ? nameBytes : nameBytes.subarray(0, zero),
        );

        entries.push({
            type: entry[2],
            subtype: entry[3],
            offset: view.getUint32(pos + 4, true),
            size: view.getUint32(pos + 8, true),
            name,
            flags: view.getUint32(pos + 28, true),
        });
    }

    return entries;
}