const DEFAULT_WASM_URL = "/littlefs.async.wasm";
const MAX_ATTRIBUTE_TAGS = 256;
const DEFAULT_BUFFER_SIZE = 8192;
const READ_ONLY_ERROR = -5;

export enum LittleFsInfoType {
    RegularFile = 1,
    Directory = 2,
}

export enum LittleFsOpenFlags {
    ReadOnly = 1,
    WriteOnly = 2,
    ReadWrite = 3,
    Create = 0x0100,
    Exclusive = 0x0200,
    Truncate = 0x0400,
    Append = 0x0800,
}

export interface LittleFsMountOptions {
    readSize: number;
    cacheSize: number;
    lookaheadSize: number;
    blockSize: number;
    blockCount: number;
}

export interface LittleFsDirEntry {
    type: LittleFsInfoType | number;
    size: number;
    name: string;
}

export type LittleFsReadCallback = (
    offset: number,
    target: ArrayBuffer,
    ptr: number,
    size: number,
) => void | Promise<void>;

export type LittleFsProgCallback = (
    offset: number,
    source: ArrayBuffer,
    ptr: number,
    size: number,
) => number | void;

export type LittleFsEraseCallback = (offset: number, size: number) => number | void;
export type LittleFsSyncCallback = () => number | void;

export interface CreateLittleFsImageReaderOptions {
    read: LittleFsReadCallback;
    prog?: LittleFsProgCallback;
    erase?: LittleFsEraseCallback;
    sync?: LittleFsSyncCallback;
    wasmUrl?: string | URL;
}

export interface ImageReaderOptions extends Pick<CreateLittleFsImageReaderOptions, "wasmUrl"> {
    partitionOffset?: number;
}

export interface LittleFsImageReader {
    formatImage(options: LittleFsMountOptions): Promise<void>;
    openImage(options: LittleFsMountOptions): Promise<void>;
    unmount(): void;
    openDir(path: string): Promise<void>;
    readDir(): Promise<LittleFsDirEntry | null>;
    closeDir(): void;
    listDir(path?: string): Promise<LittleFsDirEntry[]>;
    openFile(path: string, flags?: LittleFsOpenFlags | number): Promise<void>;
    readFileChunk(): Promise<Uint8Array>;
    writeFileChunk(data: Uint8Array | ArrayBuffer): Promise<number>;
    syncFile(): Promise<void>;
    truncateOpenFile(size: number): Promise<void>;
    closeFile(): void;
    readFile(path: string): Promise<Uint8Array>;
    readTextFile(path: string, encoding?: string): Promise<string>;
    writeFile(path: string, data: Uint8Array | ArrayBuffer, flags?: number): Promise<void>;
    writeTextFile(path: string, text: string, encoding?: string, flags?: number): Promise<void>;
    truncateFile(path: string, size: number): Promise<void>;
    remove(path: string): Promise<void>;
    rename(oldPath: string, newPath: string): Promise<void>;
    mkdir(path: string): Promise<void>;
    traverse(callback: (block: number) => void): Promise<void>;
    countAttributes(path: string): Promise<number>;
    listAttributes(path: string): Promise<Array<[tag: number, size: number]>>;
    readAttribute(path: string, tag: number): Promise<Uint8Array>;
    setAttribute(path: string, tag: number, data: Uint8Array | ArrayBuffer): Promise<void>;
    removeAttribute(path: string, tag: number): Promise<void>;
}

interface LittleFsWasmExports {
    memory: WebAssembly.Memory;
    get_asyncify_info(): number;
    prepare_asyncify_unwind(): void;
    configure(
        readSize: number,
        cacheSize: number,
        lookaheadSize: number,
        blockSize: number,
        blockCount: number,
    ): number;
    get_lfs_image(): number;
    get_lfs_path_buffer(): number;
    get_lfs_path_buffer_2(): number;
    get_lfs_dir(): number;
    get_lfs_info(): number;
    get_lfs_info_type(): number;
    get_lfs_info_size(): number;
    get_lfs_info_name(): number;
    get_lfs_info_name_length(): number;
    get_lfs_file_config(): number;
    get_lfs_file(): number;
    get_file_read_buffer(): number;
    get_file_write_buffer(): number;
    do_lfs_traverse(): number;
    get_attr_sizes(): number;
    list_lfs_attr(): number;
    lfs_getattr(img: number, path: number, tag: number, buffer: number, size: number): number;
    lfs_format(img: number, config: number): number;
    lfs_mount(img: number, config: number): number;
    lfs_remove(img: number, path: number): number;
    lfs_rename(img: number, oldPath: number, newPath: number): number;
    lfs_setattr(img: number, path: number, tag: number, buffer: number, size: number): number;
    lfs_removeattr(img: number, path: number, tag: number): number;
    lfs_file_opencfg(
        img: number,
        file: number,
        path: number,
        flags: number,
        config: number,
    ): number;
    lfs_file_close(img: number, file: number): number;
    lfs_file_sync(img: number, file: number): number;
    lfs_file_read(img: number, file: number, buffer: number, size: number): number;
    lfs_file_write(img: number, file: number, buffer: number, size: number): number;
    lfs_file_seek(img: number, file: number, offset: number, whence: number): number;
    lfs_file_truncate(img: number, file: number, size: number): number;
    lfs_mkdir(img: number, path: number): number;
    lfs_dir_open(img: number, dir: number, path: number): number;
    lfs_dir_close(img: number, dir: number): number;
    lfs_dir_read(img: number, dir: number, info: number): number;
    asyncify_start_unwind(data: number): void;
    asyncify_stop_unwind(): void;
    asyncify_start_rewind(data: number): void;
    asyncify_stop_rewind(): void;
    asyncify_get_state(): number;
}

type RewindCallback = (error?: unknown) => void;

function asExports(instance: WebAssembly.Instance): LittleFsWasmExports {
    return instance.exports as unknown as LittleFsWasmExports;
}

function checkReturn(ret: number, message: string): number {
    if (ret < 0) {
        throw new Error(`${message}: ${ret}`);
    }

    return ret;
}

function copyFromMemory(memory: WebAssembly.Memory, ptr: number, size: number): Uint8Array {
    return new Uint8Array(memory.buffer, ptr, size).slice();
}

function concatBytes(chunks: Uint8Array[]): Uint8Array {
    const size = chunks.reduce((total, chunk) => total + chunk.byteLength, 0);
    const out = new Uint8Array(size);
    let offset = 0;

    for (const chunk of chunks) {
        out.set(chunk, offset);
        offset += chunk.byteLength;
    }

    return out;
}

function asBytes(data: Uint8Array | ArrayBuffer): Uint8Array {
    return data instanceof Uint8Array ? data : new Uint8Array(data);
}

async function loadWasm(url: string | URL): Promise<ArrayBuffer> {
    const response = await fetch(url);

    if (!response.ok) {
        throw new Error(`Failed to load LittleFS WASM from ${url.toString()}: ${response.status}`);
    }

    return response.arrayBuffer();
}

function fillRandom(memory: WebAssembly.Memory, ptr: number, size: number): number {
    const out = new Uint8Array(memory.buffer, ptr, size);

    if (globalThis.crypto?.getRandomValues !== undefined) {
        for (let offset = 0; offset < out.byteLength; offset += 65536) {
            globalThis.crypto.getRandomValues(out.subarray(offset, offset + 65536));
        }
    } else {
        for (let offset = 0; offset < out.byteLength; offset += 1) {
            out[offset] = Math.floor(Math.random() * 256);
        }
    }

    return 0;
}

function defaultReadOnlyProg(): number {
    return READ_ONLY_ERROR;
}

function defaultReadOnlyErase(): number {
    return READ_ONLY_ERROR;
}

function defaultReadOnlySync(): number {
    return 0;
}

function validateRange(size: number, offset: number, length: number, label: string): void {
    const end = offset + length;

    if (offset < 0 || end > size) {
        throw new Error(
            `LittleFS ${label} is outside the supplied image: offset=${offset}, size=${length}, imageSize=${size}`,
        );
    }
}

export function createReadCallbackFromImage(
    image: Uint8Array | ArrayBuffer,
    partitionOffset = 0,
): LittleFsReadCallback {
    const bytes = asBytes(image);

    return (offset, target, ptr, size) => {
        const sourceOffset = partitionOffset + offset;
        validateRange(bytes.byteLength, sourceOffset, size, "read");
        new Uint8Array(target, ptr, size).set(bytes.subarray(sourceOffset, sourceOffset + size));
    };
}

export function createProgCallbackFromImage(
    image: Uint8Array | ArrayBuffer,
    partitionOffset = 0,
): LittleFsProgCallback {
    const bytes = asBytes(image);

    return (offset, source, ptr, size) => {
        const targetOffset = partitionOffset + offset;
        validateRange(bytes.byteLength, targetOffset, size, "program");

        const sourceBytes = new Uint8Array(source, ptr, size);

        for (let i = 0; i < sourceBytes.byteLength; i += 1) {
            if ((bytes[targetOffset + i] & sourceBytes[i]) !== sourceBytes[i]) {
                return READ_ONLY_ERROR;
            }
        }

        for (let i = 0; i < sourceBytes.byteLength; i += 1) {
            bytes[targetOffset + i] &= sourceBytes[i];
        }

        return 0;
    };
}

export function createEraseCallbackFromImage(
    image: Uint8Array | ArrayBuffer,
    partitionOffset = 0,
): LittleFsEraseCallback {
    const bytes = asBytes(image);

    return (offset, size) => {
        const targetOffset = partitionOffset + offset;
        validateRange(bytes.byteLength, targetOffset, size, "erase");
        bytes.fill(0xff, targetOffset, targetOffset + size);
        return 0;
    };
}

export function createReadCallbackFromBlob(
    blob: Blob,
    partitionOffset = 0,
): LittleFsReadCallback {
    return async (offset, target, ptr, size) => {
        const sourceOffset = partitionOffset + offset;
        const end = sourceOffset + size;

        if (sourceOffset < 0 || end > blob.size) {
            throw new Error(
                `LittleFS read is outside the supplied blob: offset=${sourceOffset}, size=${size}, blobSize=${blob.size}`,
            );
        }

        const bytes = new Uint8Array(await blob.slice(sourceOffset, end).arrayBuffer());
        new Uint8Array(target, ptr, size).set(bytes);
    };
}

export async function createLittleFsImageReader(
    opts: CreateLittleFsImageReaderOptions,
): Promise<LittleFsImageReader> {
    const {
        read,
        prog = defaultReadOnlyProg,
        erase = defaultReadOnlyErase,
        sync = defaultReadOnlySync,
        wasmUrl = DEFAULT_WASM_URL,
    } = opts;

    let rewindCallback: RewindCallback | undefined;
    let traverseCallback: ((block: number) => void) | undefined;
    let exports: LittleFsWasmExports | undefined;
    let importError: unknown;

    function onAsyncRewind(callback: RewindCallback): void {
        if (rewindCallback !== undefined) {
            throw new Error("Another LittleFS async callback is already registered");
        }

        rewindCallback = callback;
    }

    function callBlockImport(fn: () => number | void): number {
        try {
            return fn() ?? 0;
        } catch (error) {
            importError = error;
            return READ_ONLY_ERROR;
        }
    }

    const bytes = await loadWasm(wasmUrl);
    const { instance } = await WebAssembly.instantiate(bytes, {
        env: {
            traverseCallback(_: number, block: number): number {
                return callBlockImport(() => {
                    traverseCallback?.(block);
                });
            },
            readFileRange(offset: number, ptr: number, size: number): void {
                if (exports === undefined) {
                    throw new Error("LittleFS WASM is not initialized");
                }

                if (exports.asyncify_get_state() !== 0) {
                    exports.asyncify_stop_rewind();
                    return;
                }

                const ret = read(offset, exports.memory.buffer, ptr, size);

                if (ret === undefined) {
                    rewindCallback = undefined;
                    return;
                }

                exports.prepare_asyncify_unwind();
                const dataAddr = exports.get_asyncify_info();
                exports.asyncify_start_unwind(dataAddr);

                ret.then(
                    () => {
                        if (exports === undefined) {
                            throw new Error("LittleFS WASM is not initialized");
                        }

                        exports.asyncify_start_rewind(dataAddr);
                        const callback = rewindCallback;
                        rewindCallback = undefined;
                        callback?.();
                    },
                    (error: unknown) => {
                        const callback = rewindCallback;
                        rewindCallback = undefined;
                        callback?.(error);
                    },
                );
            },
            progFileRange(offset: number, ptr: number, size: number): number {
                if (exports === undefined) {
                    return READ_ONLY_ERROR;
                }

                return callBlockImport(() => prog(offset, exports!.memory.buffer, ptr, size));
            },
            eraseFileRange(offset: number, size: number): number {
                return callBlockImport(() => erase(offset, size));
            },
            syncFile(): number {
                return callBlockImport(sync);
            },
        },
        wasi_snapshot_preview1: {
            random_get(ptr: number, size: number): number {
                if (exports === undefined) {
                    return READ_ONLY_ERROR;
                }

                return callBlockImport(() => fillRandom(exports!.memory, ptr, size));
            },
        },
    });

    exports = asExports(instance);

    const img = exports.get_lfs_image();
    const dir = exports.get_lfs_dir();
    const file = exports.get_lfs_file();
    const fileConfig = exports.get_lfs_file_config();
    const info = exports.get_lfs_info();
    const pathBuf = exports.get_lfs_path_buffer();
    const pathBuf2 = exports.get_lfs_path_buffer_2();
    const pathBufSize = pathBuf2 - pathBuf;
    const fileReadBuffer = exports.get_file_read_buffer();
    const fileWriteBuffer = exports.get_file_write_buffer();
    const attrSizes = exports.get_attr_sizes();
    const readBufferSize = fileWriteBuffer > fileReadBuffer
        ? fileWriteBuffer - fileReadBuffer
        : DEFAULT_BUFFER_SIZE;
    const writeBufferSize = attrSizes > fileWriteBuffer
        ? attrSizes - fileWriteBuffer
        : DEFAULT_BUFFER_SIZE;
    const textEncoder = new TextEncoder();
    const textDecoder = new TextDecoder();

    if (pathBufSize <= 1) {
        throw new Error("LittleFS WASM exposed invalid path buffers");
    }

    function callAsync(fn: () => number): Promise<number> {
        importError = undefined;
        const ret = fn();

        if (exports?.asyncify_get_state() === 0) {
            if (importError !== undefined) {
                return Promise.reject(importError);
            }

            return Promise.resolve(ret);
        }

        if (ret !== 0) {
            throw new Error(`Async LittleFS operation should have returned 0, but got ${ret}`);
        }

        return new Promise((resolve, reject) => {
            onAsyncRewind((error) => {
                if (error !== undefined) {
                    reject(error);
                    return;
                }

                callAsync(fn).then(resolve, reject);
            });
        });
    }

    function writePathBuffer(path: string, target = pathBuf): void {
        const out = new Uint8Array(exports!.memory.buffer, target, pathBufSize);
        const encoded = textEncoder.encode(path);

        if (encoded.byteLength >= pathBufSize) {
            throw new Error(`LittleFS path is too long: ${path}`);
        }

        out.fill(0);
        out.set(encoded);
    }

    function configureImage(options: LittleFsMountOptions): number {
        return checkReturn(
            exports!.configure(
                options.readSize,
                options.cacheSize,
                options.lookaheadSize,
                options.blockSize,
                options.blockCount,
            ),
            "configure",
        );
    }

    async function formatImage(options: LittleFsMountOptions): Promise<void> {
        const config = configureImage(options);
        checkReturn(await callAsync(() => exports!.lfs_format(img, config)), "lfs_format");
    }

    async function openImage(options: LittleFsMountOptions): Promise<void> {
        const config = configureImage(options);
        checkReturn(await callAsync(() => exports!.lfs_mount(img, config)), "lfs_mount");
    }

    function unmount(): void {
        // The current WASM wrapper does not export lfs_unmount.
    }

    async function openDir(path: string): Promise<void> {
        writePathBuffer(path);
        checkReturn(await callAsync(() => exports!.lfs_dir_open(img, dir, pathBuf)), "lfs_dir_open");
    }

    async function readDir(): Promise<LittleFsDirEntry | null> {
        const ret = checkReturn(
            await callAsync(() => exports!.lfs_dir_read(img, dir, info)),
            "lfs_dir_read",
        );

        if (ret === 0) {
            return null;
        }

        const nameLength = exports!.get_lfs_info_name_length();

        return {
            type: exports!.get_lfs_info_type(),
            size: exports!.get_lfs_info_size(),
            name: textDecoder.decode(
                new Uint8Array(
                    exports!.memory.buffer,
                    exports!.get_lfs_info_name(),
                    nameLength,
                ),
            ),
        };
    }

    function closeDir(): void {
        checkReturn(exports!.lfs_dir_close(img, dir), "lfs_dir_close");
    }

    async function listDir(path = "/"): Promise<LittleFsDirEntry[]> {
        await openDir(path);

        try {
            const entries: LittleFsDirEntry[] = [];

            for (;;) {
                const entry = await readDir();

                if (entry === null) {
                    return entries;
                }

                if (entry.name !== "." && entry.name !== "..") {
                    entries.push(entry);
                }
            }
        } finally {
            closeDir();
        }
    }

    async function openFile(
        path: string,
        flags: LittleFsOpenFlags | number = LittleFsOpenFlags.ReadOnly,
    ): Promise<void> {
        writePathBuffer(path);
        checkReturn(
            await callAsync(() => exports!.lfs_file_opencfg(img, file, pathBuf, flags, fileConfig)),
            "lfs_file_open",
        );
    }

    async function readFileChunk(): Promise<Uint8Array> {
        const n = checkReturn(
            await callAsync(() => exports!.lfs_file_read(img, file, fileReadBuffer, readBufferSize)),
            "lfs_file_read",
        );

        return copyFromMemory(exports!.memory, fileReadBuffer, n);
    }

    async function writeFileChunk(data: Uint8Array | ArrayBuffer): Promise<number> {
        const bytes = asBytes(data);
        let written = 0;

        while (written < bytes.byteLength) {
            const n = Math.min(writeBufferSize, bytes.byteLength - written);
            new Uint8Array(exports!.memory.buffer, fileWriteBuffer, n).set(
                bytes.subarray(written, written + n),
            );

            const ret = checkReturn(
                await callAsync(() => exports!.lfs_file_write(img, file, fileWriteBuffer, n)),
                "lfs_file_write",
            );

            if (ret === 0 && n > 0) {
                throw new Error("lfs_file_write wrote 0 bytes before the input was exhausted");
            }

            written += ret;
        }

        return written;
    }

    async function syncFile(): Promise<void> {
        checkReturn(await callAsync(() => exports!.lfs_file_sync(img, file)), "lfs_file_sync");
    }

    async function truncateOpenFile(size: number): Promise<void> {
        checkReturn(
            await callAsync(() => exports!.lfs_file_truncate(img, file, size)),
            "lfs_file_truncate",
        );
    }

    function closeFile(): void {
        checkReturn(exports!.lfs_file_close(img, file), "lfs_file_close");
    }

    async function readFile(path: string): Promise<Uint8Array> {
        await openFile(path);

        try {
            const chunks: Uint8Array[] = [];

            for (;;) {
                const chunk = await readFileChunk();

                if (chunk.byteLength === 0) {
                    return concatBytes(chunks);
                }

                chunks.push(chunk);
            }
        } finally {
            closeFile();
        }
    }

    async function readTextFile(path: string, encoding = "utf-8"): Promise<string> {
        return new TextDecoder(encoding).decode(await readFile(path));
    }

    async function writeFile(
        path: string,
        data: Uint8Array | ArrayBuffer,
        flags = LittleFsOpenFlags.WriteOnly | LittleFsOpenFlags.Create | LittleFsOpenFlags.Truncate,
    ): Promise<void> {
        await openFile(path, flags);

        try {
            await writeFileChunk(data);
            await syncFile();
        } finally {
            closeFile();
        }
    }

    async function writeTextFile(
        path: string,
        text: string,
        encoding = "utf-8",
        flags = LittleFsOpenFlags.WriteOnly | LittleFsOpenFlags.Create | LittleFsOpenFlags.Truncate,
    ): Promise<void> {
        if (encoding.toLowerCase() !== "utf-8" && encoding.toLowerCase() !== "utf8") {
            throw new Error(`TextEncoder only supports utf-8, got ${encoding}`);
        }

        await writeFile(path, new TextEncoder().encode(text), flags);
    }

    async function truncateFile(path: string, size: number): Promise<void> {
        await openFile(path, LittleFsOpenFlags.ReadWrite);

        try {
            await truncateOpenFile(size);
            await syncFile();
        } finally {
            closeFile();
        }
    }

    async function remove(path: string): Promise<void> {
        writePathBuffer(path);
        checkReturn(await callAsync(() => exports!.lfs_remove(img, pathBuf)), "lfs_remove");
    }

    async function rename(oldPath: string, newPath: string): Promise<void> {
        writePathBuffer(oldPath, pathBuf);
        writePathBuffer(newPath, pathBuf2);
        checkReturn(
            await callAsync(() => exports!.lfs_rename(img, pathBuf, pathBuf2)),
            "lfs_rename",
        );
    }

    async function mkdir(path: string): Promise<void> {
        writePathBuffer(path);
        checkReturn(await callAsync(() => exports!.lfs_mkdir(img, pathBuf)), "lfs_mkdir");
    }

    async function traverse(callback: (block: number) => void): Promise<void> {
        traverseCallback = callback;

        try {
            checkReturn(await callAsync(() => exports!.do_lfs_traverse()), "lfs_traverse");
        } finally {
            traverseCallback = undefined;
        }
    }

    async function countAttributes(path: string): Promise<number> {
        writePathBuffer(path);
        return checkReturn(await callAsync(() => exports!.list_lfs_attr()), "list_lfs_attr");
    }

    async function listAttributes(path: string): Promise<Array<[tag: number, size: number]>> {
        const n = await countAttributes(path);
        const sizes = new Uint16Array(exports!.memory.buffer, attrSizes, MAX_ATTRIBUTE_TAGS);
        const attrs: Array<[tag: number, size: number]> = [];

        for (let tag = 0; tag < MAX_ATTRIBUTE_TAGS; tag += 1) {
            if (sizes[tag] !== 0xffff) {
                attrs.push([tag, sizes[tag]]);
            }
        }

        if (attrs.length !== n) {
            throw new Error("Reported LittleFS attribute count does not match the attribute table");
        }

        return attrs;
    }

    async function readAttribute(path: string, tag: number): Promise<Uint8Array> {
        writePathBuffer(path);

        const size = checkReturn(
            await callAsync(() =>
                exports!.lfs_getattr(img, pathBuf, tag, fileReadBuffer, readBufferSize),
            ),
            "lfs_getattr",
        );

        return copyFromMemory(exports!.memory, fileReadBuffer, size);
    }

    async function setAttribute(
        path: string,
        tag: number,
        data: Uint8Array | ArrayBuffer,
    ): Promise<void> {
        const bytes = asBytes(data);

        if (bytes.byteLength > writeBufferSize) {
            throw new Error(`LittleFS attribute is too large: ${bytes.byteLength}`);
        }

        writePathBuffer(path);
        new Uint8Array(exports!.memory.buffer, fileWriteBuffer, bytes.byteLength).set(bytes);
        checkReturn(
            await callAsync(() =>
                exports!.lfs_setattr(img, pathBuf, tag, fileWriteBuffer, bytes.byteLength),
            ),
            "lfs_setattr",
        );
    }

    async function removeAttribute(path: string, tag: number): Promise<void> {
        writePathBuffer(path);
        checkReturn(
            await callAsync(() => exports!.lfs_removeattr(img, pathBuf, tag)),
            "lfs_removeattr",
        );
    }

    return {
        formatImage,
        openImage,
        unmount,
        openDir,
        readDir,
        closeDir,
        listDir,
        openFile,
        readFileChunk,
        writeFileChunk,
        syncFile,
        truncateOpenFile,
        closeFile,
        readFile,
        readTextFile,
        writeFile,
        writeTextFile,
        truncateFile,
        remove,
        rename,
        mkdir,
        traverse,
        countAttributes,
        listAttributes,
        readAttribute,
        setAttribute,
        removeAttribute,
    };
}

export async function createLittleFsImageReaderFromImage(
    image: Uint8Array | ArrayBuffer,
    mountOptions: LittleFsMountOptions,
    options: ImageReaderOptions = {},
): Promise<LittleFsImageReader> {
    const reader = await createLittleFsImageReader({
        wasmUrl: options.wasmUrl,
        read: createReadCallbackFromImage(image, options.partitionOffset),
        prog: createProgCallbackFromImage(image, options.partitionOffset),
        erase: createEraseCallbackFromImage(image, options.partitionOffset),
    });

    await reader.openImage(mountOptions);
    return reader;
}

export async function createLittleFsImageReaderFromBlob(
    blob: Blob,
    mountOptions: LittleFsMountOptions,
    options: ImageReaderOptions = {},
): Promise<LittleFsImageReader> {
    const reader = await createLittleFsImageReader({
        wasmUrl: options.wasmUrl,
        read: createReadCallbackFromBlob(blob, options.partitionOffset),
    });

    await reader.openImage(mountOptions);
    return reader;
}

export async function createFormattedLittleFsImage(
    imageSize: number,
    mountOptions: LittleFsMountOptions,
    options: Pick<CreateLittleFsImageReaderOptions, "wasmUrl"> = {},
): Promise<{ image: Uint8Array; reader: LittleFsImageReader }> {
    const image = new Uint8Array(imageSize);
    image.fill(0xff);

    const reader = await createLittleFsImageReader({
        wasmUrl: options.wasmUrl,
        read: createReadCallbackFromImage(image),
        prog: createProgCallbackFromImage(image),
        erase: createEraseCallbackFromImage(image),
    });

    await reader.formatImage(mountOptions);
    await reader.openImage(mountOptions);

    return { image, reader };
}

export async function removeFileRecursively(
    lfs: LittleFsImageReader,
    path: string,
): Promise<void> {
    try {
        const entries = await lfs.listDir(path);
        const basePath = path.replace(/\/+$/, "");

        for (const entry of entries) {
            await removeFileRecursively(lfs, `${basePath}/${entry.name}`);
        }
    } catch (error) {
        const message = error instanceof Error ? error.message : String(error);

        if (!message.includes("lfs_dir_open: -20") && !message.includes("lfs_dir_open: -2")) {
            throw error;
        }
    }

    try {
        await lfs.remove(path);
    } catch (error) {
        const message = error instanceof Error ? error.message : String(error);

        if (!message.includes("lfs_remove: -2")) {
            throw error;
        }
    }
}
