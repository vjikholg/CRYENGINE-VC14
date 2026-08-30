# Legacy nvDXT oracle

This utility is intentionally built with the Visual C++ 2005 x64 compiler. It
links the bundled `nvDXTlibMTDLL.vc8.x64.lib`, decodes the selected DDS fixtures,
and writes top-mip RGBA reference files for the VS2022 tests.

Run `GenerateGoldenOutputs.cmd` only when intentionally refreshing the nvDXT
baselines. The oracle executable and object file are written under `BinTemp` and
must not be committed.

The normal `CryCommonTests` executable does not load this utility or link any VC8
code. It consumes only the generated `.nvdxt.rgba` files.
