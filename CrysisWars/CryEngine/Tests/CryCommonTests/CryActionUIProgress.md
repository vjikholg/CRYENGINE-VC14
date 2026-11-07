# CryAction UI repair and validation — 2026-11-06

Scope: UI01/UI02 and their ASan allocation-owner prerequisite. NW04, AN07 and
OS01 remain outside this pass. Other historical CryAction defects were not repaired
or revalidated here. All changes are local and uncommitted.

## Repairs

- CUISystem batches whole lines, triangles and quads before either its uint16
  index domain, the renderer's GetDynVBSize capacity, or the backend's 32,768-index
  buffer is exceeded. Oversized circle fans split at complete triangles and repeat
  the center/shared edge. Fewer than three slices emit no geometry.
- SetBlend flushes the old state before recording the new one; Flush2D submits the
  stored state. Begin2D flushes prior geometry before changing dimensions and
  restores the standard UI blending default.
- ToWideString uses mbrtowc's consumed byte count and a fresh mbstate_t in the
  selected CRT locale. No implicit UTF-8 code-page change was introduced. The
  explicit overflow policy is a complete-character prefix of at most 2047 wchar_t
  units plus NUL. Invalid/incomplete input encountered before that limit returns
  empty, including any previously converted prefix. Null returns empty. The pointer
  still borrows the static buffer until the next call; no concurrent-use guarantee
  or API return-length field was added. Supplementary Unicode/surrogate handling
  beyond the selected CRT mbrtowc contract is not claimed.
- The local UI script is mounted at Game/ with temporary pak priority and an
  explicit identity marker. The fixture restores the locale, pak priority and
  environment views and checks balanced input subscription teardown.

CUISystem is an internal owner, not a published interface. Its private layout now
includes blend state. Public signatures, renderer index types, file formats and
network formats are unchanged.

## ASan findings and allocation lifetime

The old UI01 reports stopped in index-vector capture/reallocation and console
command removal. CryAction's annotated STL containers shared allocation pools
owned by an uninstrumented CrySystem. Returned pool slots could retain stale
container shadow poison when reused by an unrelated allocation.

The ASan lane now stages an instrumented CrySystem as well as CryAction and the
host. CryMemoryManager unpoisons a newly allocated slot before initializing its
header and poisons released user storage while keeping intrusive pool metadata
accessible. These hooks compile only under __SANITIZE_ADDRESS__. Vector
annotations and ordinary ASan checks remain enabled; no suppression was added.
ui-renderer-limit additionally verifies released-pointer poison and reuse of a
previously poisoned pool slot through the real module allocation wrappers.
Other dependencies remain uninstrumented; this is not complete-engine memory
safety validation.

The first CrySystem diagnostic build exposed an output-path problem: OutDir was
reset by imported build targets despite TargetPath retaining the requested stage.
Its failed link removed the normal CrySystem DLL; a later attempt also wrote an
ASan DLL there. The original DLL was restored from the staged original and its
SHA-256 matched the saved pre-repair evidence. The dedicated build now explicitly
uses TargetPath/TargetDir for DLL/PDB output and checks both normal DLL hashes.
The normal CrySystem PDB was touched by those intermediate attempts; its original
symbols need recovery/regeneration before debugging the preserved normal DLL.

## Requirement-to-case mapping

| Requirement | Cases / evidence |
| --- | --- |
| Empty drawing and independent primitive vertices, indices, color/UV and topology | ui-empty, ui-line, ui-tri, ui-quad, ui-circle |
| Mixed primitive order and texture changes | ui-mixed, ui-texture |
| Scissor clipping and font clipping state | ui-scissor; original line-clipping path preserved |
| Blend transition ordering and submitted state | ui-blend |
| Full accumulated uint16 domain, wrapped-but-in-bounds associations | ui-index-last, ui-index-cross-line, ui-index-cross-tri, ui-index-cross-quad, ui-index-cross-circle; totals and semantic associations across split submissions |
| Renderer capacity and absent buffer | ui-renderer-limit; four-vertex capacity and zero capacity |
| One oversized circle and invalid counts | ui-circle-large (70,000 triangles with ordered segment joins), ui-circle-invalid (-1/0/1/2 slices) |
| Actual precache position/count consumer | ui-precache-empty, ui-precache-single, ui-precache-three |
| Offscreen image | ui-offscreen is implemented but BLOCKED: D3D10 ITexture::LockData is unimplemented. The existing framebuffer API reads the backbuffer, not this target. Allocation/draw attempt does not establish visual compatibility. |
| Empty/ASCII/2047/2048/2049 text and bounded termination | text-empty, text-ascii, text-2047, text-2048, text-2049; ASan global-buffer checking |
| Explicit multibyte locales and boundary | text-multibyte, text-multibyte-boundary (.UTF8); text-dbcs (.932), text-legacy-byte (.1252) |
| Invalid input, stale prefixes, repeated calls, null | text-invalid, text-invalid-middle, text-repeated, text-null |
| Production conversion to font consumer; input unchanged; borrowed lifetime | text-font and other completed text cases capture actual DrawText output; text-repeated checks next-call invalidation and independent owned copy |

Remaining integration: offscreen readback/scene verification and rendering text
through the actual font/device path. The recording font proves call contents and
ownership, not rasterized font appearance. A full playable framework and original
shipping-runtime image comparison are not established by these component tests.

## Reproduction and evidence

Final focused results (mutually exclusive status columns):

| Lane | Family | PASS | FAIL | CRASH | TIMEOUT | BLOCKED | ASan diagnoses | Unexpected terminations |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Profile | UI01 | 20 | 0 | 0 | 0 | 1 | 0 | 0 |
| Profile | UI02 | 14 | 0 | 0 | 0 | 0 | 0 | 0 |
| Profile + ASan | UI01 | 20 | 0 | 0 | 0 | 1 | 0 | 0 |
| Profile + ASan | UI02 | 14 | 0 | 0 | 0 | 0 | 0 | 0 |

Before repair: Profile UI01 12 PASS/5 FAIL/1 BLOCKED and UI02 6 PASS/4 FAIL.
ASan UI01 8 PASS/9 FAIL/1 BLOCKED with nine diagnoses; UI02 6 PASS/4 FAIL
with three diagnoses. Seven additional bounded cases were added. All nine
functional failing cases and all twelve sanitizer terminations are now resolved
in the focused runs. Each helper's default suite passes 97 cases/zero failures.
Normal CryAction.dll SHA-256 remains
E9FA2C5B4CFCF13174D82221167FF3F820B9B7FA50B0B4D526194318D6F5A707;
normal CrySystem.dll remains
C0D0B3EF36DB449855AB9063F24613AA64A15586401BF1D5EAB5426EF13F86D3.
The normal DLLs are preserved baseline binaries; these source fixes are compiled
into the isolated validation stage, not deployed to the normal runtime.

Final reports/evidence are preserved in
[the validated UI archive](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-UI-fixed-20261106).

Run each family sequentially within a lane; the build helper stages dependencies
and cannot overwrite DLLs held by an active child in the same stage:

```powershell
& 'G:/projects/CRYENGINE-VC14/CrysisWars/CryEngine/Tests/CryCommonTests/RunCryActionRegression.ps1' -Family UI01
& 'G:/projects/CRYENGINE-VC14/CrysisWars/CryEngine/Tests/CryCommonTests/RunCryActionRegression.ps1' -Family UI02
& 'G:/projects/CRYENGINE-VC14/CrysisWars/CryEngine/Tests/CryCommonTests/RunCryActionRegression.ps1' -Asan -Family UI01
& 'G:/projects/CRYENGINE-VC14/CrysisWars/CryEngine/Tests/CryCommonTests/RunCryActionRegression.ps1' -Asan -Family UI02
```

Reports and evidence JSONs use the suffixes Profile-UI01, Profile-UI02,
AsanProfile-UI01 and AsanProfile-UI02 under the workspace BinTemp directory.
ui-offscreen remains BLOCKED rather than passing or being removed. The default
97-case unit suite also runs in every helper invocation.

Before-fix reports, evidence JSONs, and original UI sources are preserved in
[the repair baseline](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-UI-before-fixes-20261106).
The prior full 153-case baseline and its unrelated red cases remain historical
evidence; the complete expanded CryAction manifest has not been rerun in this pass.
