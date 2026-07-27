const fs = require('fs')
const path = require('path')

const projectRoot = path.resolve(__dirname, '..')
const cliBinDir = path.join(
  projectRoot,
  'node_modules',
  '@lightningjs',
  'cli',
  'node_modules',
  '.bin'
)
const rollupEntrypoint = path.join(projectRoot, 'node_modules', 'rollup', 'dist', 'bin', 'rollup')
const nodeExecutable = process.execPath

if (!fs.existsSync(rollupEntrypoint)) {
  console.error('Missing rollup entrypoint at ' + rollupEntrypoint)
  process.exit(1)
}

fs.mkdirSync(cliBinDir, { recursive: true })

const posixNodeExecutable = nodeExecutable.replace(/\\/g, '/')
const posixRollupEntrypoint = rollupEntrypoint.replace(/\\/g, '/')

const shShim = [
  '#!/bin/sh',
  'exec "' + posixNodeExecutable + '" "' + posixRollupEntrypoint + '" "$@"',
  '',
].join('\n')

const cmdShim = [
  '@ECHO off',
  '"' + nodeExecutable + '" "' + rollupEntrypoint + '" %*',
  '',
].join('\r\n')

const ps1NodeExecutable = nodeExecutable.replace(/'/g, "''")
const ps1RollupEntrypoint = rollupEntrypoint.replace(/'/g, "''")
const ps1Shim = [
  "& '" + ps1NodeExecutable + "' '" + ps1RollupEntrypoint + "' @args",
  '',
].join('\r\n')

const writeShim = (filePath, content, mode) => {
  if (fs.existsSync(filePath) && fs.readFileSync(filePath, 'utf8') === content) return
  fs.writeFileSync(filePath, content, 'utf8')
  if (mode) fs.chmodSync(filePath, mode)
}

writeShim(path.join(cliBinDir, 'rollup'), shShim, 0o755)
writeShim(path.join(cliBinDir, 'rollup.cmd'), cmdShim)
writeShim(path.join(cliBinDir, 'rollup.ps1'), ps1Shim)

console.log('Ensured Lightning CLI rollup shim at ' + cliBinDir)
