import { rename } from 'node:fs/promises'
import { resolve } from 'node:path'
import { defineConfig, type Plugin } from 'vite'
import { svelte } from '@sveltejs/vite-plugin-svelte'

function emitIndexHtm(): Plugin {
    let outDir = ''

    return {
        name: 'emit-index-htm',
        apply: 'build',
        configResolved(config) {
            outDir = resolve(config.root, config.build.outDir)
        },
        async closeBundle() {
            await rename(resolve(outDir, 'index.html'), resolve(outDir, 'index.htm'))
        },
    }
}

// https://vite.dev/config/
export default defineConfig({
    plugins: [svelte(), emitIndexHtm()],
    build: {
        outDir: '../software/data/dashboard',
        emptyOutDir: true,
    }
})
