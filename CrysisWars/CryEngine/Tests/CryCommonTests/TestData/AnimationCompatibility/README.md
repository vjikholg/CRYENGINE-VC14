# Animation Asset Compatibility

The compatibility pipeline discovers every `.chr`, `.cga`, `.anm`, `.caf`, and
`.cgf` file beneath a supplied asset root. Assets remain external to the source
tree and are never modified.

Format-only validation:

```text
CryCommonTests.exe --animation-asset-format <asset-root> [report.tsv]
```

Format and runtime validation:

```text
CryCommonTests.exe --animation-assets <runtime-dir> <game-root> <asset-root> [report.tsv]
```

For MWLL, `game-root` is the testbed root and `asset-root` is normally
`<game-root>\Mods\MWLL\Game`. The runtime directory must contain the newly built
CrySystem, CryAnimation, and Cry3DEngine modules together with their required
runtime dependencies.

The TSV report contains one format row and one runtime row per discovered asset.
CHR and CGA files are loaded as character instances, CGF files as static objects,
and CAF/ANM files through CryAnimation's controller/timing chunk loader. The
runtime phase executes in a child process and writes the currently processed
asset to `<report.tsv>.current`, allowing a crash to be associated with a file.

Each asset interface is explicitly released after its probe. The child does not
perform full CrySystem teardown because the compatibility runtime can contain
unported VC8 modules whose unload behavior is outside this asset test. After the
report is flushed, process termination provides isolation and operating-system
resource cleanup. Full engine shutdown must be validated separately once the
remaining runtime modules have been rebuilt.
