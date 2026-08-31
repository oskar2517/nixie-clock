export interface FirmwareRelease {
    name: string;
    changes: string[];
    migration?: string;
    assets: {
        dashboard: string;
        firmware: {
            esp32s3: string;
        };
    };
}

const REPO = "oskar2517/nixie-clock-firmware-releases";
const BRANCH = "main";
const RAW_BASE = `https://raw.githubusercontent.com/${REPO}/${BRANCH}`;

function rawUrl(path: string): string {
    const normalizedPath = path.replace(/^\/+/, "");
    return `${RAW_BASE}/${encodeURI(normalizedPath)}`;
}

export async function fetchReleases(): Promise<FirmwareRelease[]> {
    const response = await fetch(rawUrl("manifest.json"));

    if (!response.ok) {
        throw new Error(`Failed to fetch firmware manifest (${response.status})`);
    }

    return await response.json();
}

export async function downloadFirmware(path: string): Promise<Uint8Array<ArrayBuffer>> {
    const response = await fetch(rawUrl(path));

    if (!response.ok) {
        throw new Error(`Failed to download firmware (${response.status})`);
    }

    return new Uint8Array(await response.arrayBuffer());
}

export async function fetchMigration(release: FirmwareRelease): Promise<string | undefined> {
    if (release.migration === undefined) return undefined;

    const response = await fetch(rawUrl(release.migration));

    if (!response.ok) {
        throw new Error(`Failed to fetch migration for release ${release.name} (${response.status})`);
    }

    return await response.text();
}