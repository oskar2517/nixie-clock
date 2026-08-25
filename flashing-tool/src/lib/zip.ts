import { BlobReader, BlobWriter, ZipReader } from "@zip.js/zip.js";
import type { LittleFsImageReader } from "./littlefs";

export async function extractZipToLittleFs(basePath: string, data: Blob, lfs: LittleFsImageReader) {
    const zipFileReader = new BlobReader(data);
    const zipReader = new ZipReader(zipFileReader);
    const normalizedBasePath = `/${basePath.split("/").filter(Boolean).join("/")}`;

    const entries = await zipReader.getEntries();

    for (const e of entries) {
        if (e.directory) continue;

        const normalizedFilename = e.filename.split("/").filter(Boolean).join("/");
        const filename = `${normalizedBasePath}/${normalizedFilename}`;

        const parts = filename.split("/").filter(Boolean);
        let current = "";

        for (let i = 0; i < parts.length - 1; i++) {
            current += `/${parts[i]}`;

            try {
                await lfs.mkdir(current);
            } catch (err) { }
        }

        const fileData = new BlobWriter();
        const fileBlob = await e.getData(fileData);

        await lfs.writeFile(filename, await fileBlob.arrayBuffer());
    }

    await zipReader.close();
}
