# MatPLC - Coruscant Release

> **NOTE**: This is the **Coruscant release** of MatPLC (version 0.0.6), originally from https://mat.sourceforge.net/, updated to build on modern Linux systems.

## Overview

MatPLC (Matrix PLC) is an open-source Programmable Logic Controller (PLC) software developed around 2002-2007, based on the IEC 61131-3 standard for PLC programming languages.

This is a **modernized fork** of the Coruscant release (May 2006) that compiles and runs on modern Linux systems (tested on Linux Mint 21+ with GCC 13).

## What is Working

- **Core library**: `libmatplc.so` / `libmatplc.a`
- **Main executable**: `tools/run/matplc`
- **DSP logic module**: `logic/dsp/dsp`
- **IEC 61131-3 compiler**: `logic/iec/iec2cc` (IL/ST compilation)
- **PLC5 emulator**: Basic compilation (logic/plc5)

## Prerequisites

### Ubuntu/Debian/Linux Mint

```bash
# Core build tools
sudo apt-get install build-essential libtool autoconf automake

# For GTK2 HMI support (optional)
sudo apt-get install libgtk2.0-dev libglade2-dev

# For additional features
sudo apt-get install libmodbus-dev comedi-dev
```

## Building

### Build All Available Modules

```bash
# Build core and main components
make
```

### Build Individual Components

```bash
# Core library only
make -C lib

# Main executable only
make -C tools/run

# DSP logic module
make -C logic/dsp

# IEC compiler
make -C logic/iec

# GTK2 HMI (requires libgtk2.0-dev)
make -C mmi/hmi_gtk2
```

### Quick Test

```bash
# Main PLC runtime
./tools/run/matplc --help

# DSP module
./logic/dsp/dsp --help

# IEC compiler
./logic/iec/iec2cc
```

## Project Structure

```
mat-coruscant/
├── lib/              - Core library (plc, gmm, synch, etc.)
├── tools/run/        - Main matplc executable
├── logic/
│   ├── dsp/         - DSP/filter logic module (WORKS)
│   ├── iec/         - IEC 61131-3 compiler (WORKS)
│   ├── ladder_lib/  - Ladder logic (INCOMPLETE SOURCE)
│   ├── plc5/        - PLC5 emulator
│   └── il/          - IL compiler
├── io/               - I/O modules (modbus, parport, etc.)
├── mmi/              - Human-Machine Interface modules
│   ├── hmi_gtk/     - GTK1 HMI (deprecated)
│   ├── hmi_gtk2/    - GTK2 HMI (requires libgtk2.0-dev)
│   └── curses/      - Curses-based HMI
├── comm/             - Communication libraries
├── service/          - Services (email, matd)
└── demo/             - Example applications
```

## Known Issues

1. **Ladder Logic (ladder_lib)**: The `core.cpp` file is incomplete in this release - the core class implementation is missing from the source code. This appears to be a gap in the original Coruscant release.

2. **GTK1**: The original GTK1 HMI is no longer supported on modern systems.

3. **GTK2**: Requires `libgtk2.0-dev` and `libglade2-dev` packages.

4. **Timer API**: POSIX timer API changed - some casting may be needed for `timer_t`.

## History

This code is based on the **Coruscant release** (version 0.0.6) from May 2006, originally hosted at:
- http://mat.sourceforge.net/
- Later moved to BerliOS

The code is licensed under the GPL v2 (see COPYING file).

## Contributing

This is a historical preservation project. To contribute:

1. Fix remaining build issues
2. Find/complete missing source code (especially ladder_lib/core.cpp)
3. Update documentation
4. Add modern build system support (CMake, etc.)

## Acknowledgments

- Original authors: Mario de Sousa, Hugh Jack, Juan Carlos Orozco, and many contributors
- The IEC compiler was based on the CANOPEN project's compiler
- Named after planets from Star Wars (releases: Alderaan, Bespin, Coruscant, Dagobah, Endor)
