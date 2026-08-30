# DXT compatibility fixtures

These DDS files are selected from the local MWLL testbed and cover DXT1, DXT3,
and DXT5. The matching `.nvdxt.rgba` files are top-mip RGBA output captured by
`LegacyNvDxtOracle.exe`, built with the original VC8 x64 compiler and bundled
`nvDXTlibMTDLL.vc8.x64.lib`.

The normal VS2022 tests never load or link the VC8 library. They decode each DDS
top mip through the bundled Squish implementation and compare every output byte
against the captured nvDXT result.

DXT1 and DXT3 currently match byte-for-byte. The selected DXT5 texture differs
by at most one least-significant bit in any RGBA channel because the decoders use
slightly different integer interpolation rounding. The test rejects any larger
difference.

Fixture provenance:

- `mwll_tankcolor_dxt1.dds`: `Props/all/propanetank/tankcolor.dds`
- `mwll_dot_dxt3.dds`: `Effects/textures/dds/dot.dds`
- `mwll_light_color_dxt5.dds`: `Props/all/lightpole/light_color.dds`
