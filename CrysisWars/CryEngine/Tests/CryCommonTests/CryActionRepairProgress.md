# NW04, AN07 and OS01 repair — 2026-11-06

Scope: the three requested families. Changes are local and uncommitted. Other
historical CryAction failures were not repaired or revalidated in this pass.

## Production changes

- **NW04:** DynArray traversal and diagnostic indices use the container's signed
  index type. Successful replay erases without skipping adjacent entries;
  deferred or unsuccessful records advance once and remain available for retry.
  Invalid operation records, null records and the unrepresentable negative
  reference INT_MIN are rejected. BreakablePlane validates recorded ranges before
  accessing chunk IDs, bounds appended counts to int, and checks spawn failure.
- **NW04 physics dependency:** grid generation and fragmentation previously called
  CRT srand while drawing from the physics Mersenne Twister. A fresh-geometry
  replay reproduced different fragments for the same recorded seed. Both grid
  entry points now seed the generator they actually consume. The existing -1
  no-reseed behavior is preserved. No wire member, width or field name changed.
- **AN07:** the graph displays the selected query ID, rather than the address of
  the query array. IDs retain their unsigned values, factory/RMI pointers use %p,
  and label construction is bounded. Text containing literal percent signs is
  passed as data to the renderer and system warning owner. The missing-extension
  diagnostic also exposed GameWarning formatting its text a second time; its
  call to the system now uses a literal %s format.
- **OS01:** a shared Windows boundary module closes registry handles on every
  opened path, writes the REG_SZ terminator, accepts legacy unterminated values
  only when the 32-byte read buffer has append room, and rejects oversized or
  incorrectly typed values. ViewNote checks temporary-path failure and append
  room, quotes the editor's filename, and releases failure-path resources. Its
  implementation moved from the header to a corresponding source file. Reference
  ownership and registration synchronization protect an early editor-exit
  callback; process/thread handles and note buffers are released during cleanup.

## Coverage

| Family | Cases | What executes |
| --- | ---: | --- |
| NW04 | 13 | Actual replay with 0/1/3 records; exact joint parameters; all-fail/mixed retries; sentinel, frame and signed-reference gates; captured dump output; malformed records; real recording owner and production script-table serializers; fresh real two-part CryPhysics joints; real plane fragmentation and loading with recorded seeds/chunk IDs and 0/1/3 preserved prefix entries. |
| AN07 | 6 | Actual graph DebugDisplay with 0/1/3 inputs and unsigned IDs above INT_MAX; literal percent signs; real factory pointer; actual missing-extension RMI Sync on a real local entity and captured emitted diagnostics. |
| OS01 | 7 | Actual framework registry read/write through a test-owned HKCU key; missing/normal/legacy/malformed/type/length cases; persisted terminator and 32-byte raw API read; 128 repeated read/write pairs with unchanged process handle count; actual GetTempPath and controlled path-capacity/failure cases without an editor. |

The old break-record, break-physics and break-chunks aliases have been replaced
with the named owners and dependencies. They no longer repeat generic playback.
The full manifest now has **240 cases across all 33 planned family labels**.
Representation does not mean complete planned integration, and the complete
240-case matrix was not run in this pass.

Final focused validation is recorded in the six family TSV/evidence pairs below.
The helper also runs the existing 97-test default suite before each family.

| Lane | NW04 | AN07 | OS01 |
| --- | --- | --- | --- |
| x64 Profile | 13 PASS | 6 PASS | 7 PASS |
| x64 Profile + ASan | 13 PASS | 6 PASS | 7 PASS |

Each lane has 26 passing focused cases and 552 successful assertions, with zero
failures, crashes, timeouts, blocked cases, sanitizer diagnoses or unexpected
terminations. All six helper invocations also passed the 97-test default suite.
The normal CryAction, CrySystem and CryPhysics DLL hashes match their pre-run
values. A whitespace/error check of the working diff passed.

## Evidence and reproduction

- [Frozen final evidence archive](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-NW04-AN07-OS01-fixed-20261106) preserves all six reports, evidence records, case logs and final build logs. All six recorded source-hash lists were checked against the final source.
- Profile: [NW04](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-regression-Profile-NW04.tsv), [AN07](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-regression-Profile-AN07.tsv), [OS01](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-regression-Profile-OS01.tsv).
- ASan: [NW04](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-regression-AsanProfile-NW04.tsv), [AN07](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-regression-AsanProfile-AN07.tsv), [OS01](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-regression-AsanProfile-OS01.tsv).
- Each report has an adjacent -evidence.json containing source/runtime hashes,
  statuses, prerequisites, sanitizer counts and production DLL hashes. Per-case
  logs retain assertion and diagnostic observations.
- [Intermediate diagnostics](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-NW04-AN07-OS01-diagnostics-20261106) retain the failing percent-text RMI check, the fixture object-manager teardown crash, and the fresh-geometry replay failure before correcting physics seeding. These are separate from final passing results.

Run the [regression helper](/g:/projects/CRYENGINE-VC14/CrysisWars/CryEngine/Tests/CryCommonTests/RunCryActionRegression.ps1)
with -Family NW04, AN07 or OS01, adding -Asan for the sanitizer lane. Builds use
isolated directories and leave the normal CryAction, CrySystem and CryPhysics
DLLs unchanged. The staged physics dependency is rebuilt in Profile for both
lanes so the repaired seed path is exercised. ASan instruments the host,
CryAction and CrySystem, including pool-shadow lifetime hooks; CryPhysics and
the other engine dependencies remain uninstrumented.

## Remaining integration and compatibility limits

- The physical-joint fixture performs a real structural break and feeds its
  controlled event through the actual recording owner; it does not prove an
  autonomous solver event callback or remote transport. Script serializers are
  exercised, not a live CryNetwork packet exchange or multiplayer session.
- The corrected seed intentionally replaces order-dependent fragmentation with
  seed-repeatable fragmentation. Cross-version save replay and mixed old/new
  engine participants remain unverified; unchanged wire fields alone do not
  prove that compatibility.
- Full graph overlay visuals, character animation, and a playable game boot are
  still integration work. Registry/path tests neither access the real CD-key
  location nor launch Notepad. The ViewNote callback/process lifecycle changes
  compile in both lanes but were not exercised with a real editor or a separate
  signaled-event lifecycle test.
- Normal production DLLs were preserved, so these source repairs are not yet
  installed into a shipping game runtime. No whole-engine leak or shutdown claim
  is made. The earlier normal CrySystem PDB recovery issue remains documented in
  the [UI progress report](/g:/projects/CRYENGINE-VC14/CrysisWars/CryEngine/Tests/CryCommonTests/CryActionUIProgress.md).

Recommended next step: rerun the complete 240-case matrix to establish a current
failure inventory, then address the remaining reproduced failures in family-sized
changes. Preserve the separate shipping save/network and visual acceptance work.
