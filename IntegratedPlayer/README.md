# IPALauncher

C++17 launcher application that plays a manifest URL through `PlayerInstanceAAMP`.

## Structure

- `source/` - application source files

## Build

`IPALauncher` expects the sibling `lib32-aamp` project to be present and buildable.

```bash
cmake -S . -B build
cmake --build build
./build/source/IPALauncher <manifest-url>
```