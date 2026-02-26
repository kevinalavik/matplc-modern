# MatPLC - MATрица Programmable Logic Controller

> **NOTE**: This is a historical version of the MatPLC project, updated to build on modern Linux systems. For the latest version, check [matplc/matplc](https://github.com/matplc/matplc).

## Overview

MatPLC (Matrix PLC) is an open-source Programmable Logic Controller (PLC) software. It was originally developed around 2002-2007 and is based on the IEC 61131-3 standard for PLC programming languages.

This is a **modernized fork** that compiles and runs on modern Linux systems (tested on Linux Mint 21+ with GCC 13).

## What is Working

- **Core library**: `libmatplc.so` / `libmatplc.a`
- **Main executable**: `tools/run/matplc`
- **DSP logic module**: `logic/dsp/dsp`
- **IEC 61131-3 compiler**: `logic/iec/iec2cc` (basic IL/ST compilation)

## What Needs Work

The following modules have build issues due to their age (~20 years old code):

- **Ladder Logic**: Uses obsolete Borland `stream.h` headers
- **Modbus I/O**: May have compatibility issues
- **Various HMI modules**: GTK1/GTK2 dependencies no longer available

## Building

### Prerequisites

```bash
# Ubuntu/Debian/Linux Mint
sudo apt-get install build-essential libtool autoconf automake
```

### Build

```bash
# The main binaries
make

# Just the core library
make -C lib

# Just the main executable
make -C tools/run
```

### Quick Test

```bash
# The main PLC runtime
./tools/run/matplc --help

# DSP module
./logic/dsp/dsp --help
```

## Project Structure

```
mat-coruscant/
├── lib/              - Core library (plc, gmm, synch, etc.)
├── tools/run/        - Main matplc executable
├── logic/
│   ├── dsp/         - DSP/filter logic module (WORKS)
│   ├── iec/         - IEC 61131-3 compiler (WORKS)
│   ├── ladder_lib/  - Ladder logic (needs work)
│   └── il/          - IL compiler
├── io/               - I/O modules (modbus, parport, etc.)
├── mmi/              - Human-Machine Interface modules
├── comm/             - Communication libraries
├── service/          - Services (email, matd)
└── demo/             - Example applications
```

## History

This project was originally hosted on SourceForge and later on BerliOS. The original project page was at: http://mat.sourceforge.net/

The code is licensed under the GPL v2 (see COPYING file).

## Contributing

This is a historical preservation project. If you'd like to contribute:

1. Fix remaining build issues (see issues)
2. Update documentation
3. Add modern build system support (CMake, etc.)

## Known Issues

1. **stream.h not found** - Ladder lib uses old Borland headers
2. **iostream.h deprecated** - Many files use old C++ headers
3. **extern "C" issues** - Some files mix C and C++ incorrectly
4. **GTK1/GTK2 unavailable** - HMI modules need porting
5. **timer_t type changes** - POSIX timer API changes require casting

## Acknowledgments

- Original authors: Mario de Sousa, Hugh Jack, and many contributors
- The IEC compiler was based on the CANOPEN project's compiler
