# Binutils Format Support Research

This document catalogs all binary formats supported by GNU Binutils, organized by type, architecture, era, and system.

## Overview

GNU Binutils supports a wide range of object file formats, executable formats, and archive formats across multiple architectures and operating systems. This comprehensive list is derived from the BFD (Binary File Descriptor) library which forms the core of binutils.

---

## 1. ELF (Executable and Linkable Format)

**Era:** 1990s - Present
**Primary Systems:** Unix, Linux, BSD, Solaris, IRIX
**Type:** Modern universal format

### Variants

| Format | Bit Width | Endianness | Description |
|--------|-----------|------------|-------------|
| elf32-i386 | 32-bit | Little | Intel x86 ELF |
| elf32-iamcu | 32-bit | Little | Intel MCU ELF |
| elf64-x86-64 | 64-bit | Little | AMD64/x86-64 ELF |
| elf32-little | 32-bit | Little | Generic 32-bit LE ELF |
| elf32-big | 32-bit | Big | Generic 32-bit BE ELF |
| elf64-little | 64-bit | Little | Generic 64-bit LE ELF |
| elf64-big | 64-bit | Big | Generic 64-bit BE ELF |

### Architecture-Specific ELF Variants

#### ARM
- elf32-littlearm
- elf32-bigarm
- elf64-littleaarch64
- elf64-bigaarch64

#### MIPS
- elf32-tradbigmips
- elf32-tradlittlemips
- elf64-tradbigmips
- elf64-tradlittlemips
- elf32-nbigmips
- elf32-nlittlemips
- elf64-bigmips
- elf64-littlemips

#### PowerPC
- elf32-powerpc
- elf32-powerpcle
- elf64-powerpc
- elf64-powerpcle
- elf32-ppc
- elf64-ppc

#### SPARC
- elf32-sparc
- elf64-sparc

#### RISC-V
- elf32-littleriscv
- elf64-littleriscv
- elf64-bigriscv

#### Alpha
- elf64-alpha

#### IA-64
- elf64-ia64-little
- elf64-ia64-big

#### S/390
- elf32-s390
- elf64-s390

#### m68k
- elf32-m68k

#### SH (SuperH)
- elf32-sh
- elf32-shl
- elf64-sh64
- elf64-sh64l

#### HPPA
- elf32-hppa-linux
- elf64-hppa-linux

#### AVR
- elf32-avr

#### CR16
- elf32-cr16

#### CRIS
- elf32-cris

#### D10V, D30V
- elf32-d10v
- elf32-d30v

#### OpenRISC
- elf32-or1k
- elf32-or1knd

#### Xtensa
- elf32-xtensa-le
- elf32-xtensa-be

---

## 2. COFF (Common Object File Format)

**Era:** 1980s - Present
**Primary Systems:** Unix System V, Windows, DOS extenders
**Type:** Traditional Unix/legacy format

### Standard COFF Variants

| Format | Architecture | System | Description |
|--------|--------------|--------|-------------|
| coff-i386 | x86 | DOS/Windows | Intel 386 COFF |
| coff-x86-64 | x86-64 | Windows | AMD64 COFF |
| coff-arm | ARM | Various | ARM COFF |
| coff-go32 | x86 | DOS | GO32 DOS extender |
| coff-go32-exe | x86 | DOS | GO32 executable |

### PE/COFF (Portable Executable)

**Era:** 1993 - Present
**System:** Windows NT family

| Format | Bit Width | Description |
|--------|-----------|-------------|
| pe-i386 | 32-bit | Windows x86 executable |
| pei-i386 | 32-bit | Windows x86 PE image |
| pe-x86-64 | 64-bit | Windows x64 executable |
| pei-x86-64 | 64-bit | Windows x64 PE image |
| pe-arm | 32-bit | Windows ARM executable |
| pei-arm | 32-bit | Windows ARM PE image |
| pe-arm64 | 64-bit | Windows ARM64 executable |
| pei-arm64 | 64-bit | Windows ARM64 PE image |

### Extended COFF Variants

#### XCOFF (Extended COFF - AIX)
**Era:** 1990s - Present
**System:** IBM AIX

- aixcoff-rs6000
- aixcoff64-rs6000

#### ECOFF (Extended COFF - MIPS/Alpha)
**Era:** 1980s - 2000s
**Systems:** MIPS RISC/OS, Ultrix, Tru64

- ecoff-bigmips
- ecoff-littlemips
- ecoff-biglittlemips

### TE (Terse Executable)
**Era:** 2000s - Present
**System:** UEFI firmware

- efi-app-ia32
- efi-app-x86_64
- efi-app-ia64
- efi-app-aarch64

---

## 3. a.out Format Family

**Era:** 1970s - 1990s
**Primary Systems:** Early Unix, BSD, SunOS
**Type:** Historical Unix format

### Standard a.out

| Format | System | Description |
|--------|--------|-------------|
| a.out-i386-linux | Linux | x86 Linux a.out |
| a.out-arm-linux | Linux | ARM Linux a.out |
| a.out-m68k-linux | Linux | m68k Linux a.out |
| a.out-sparc-linux | Linux | SPARC Linux a.out |
| aout0-big | Various | Old a.out big-endian |
| aout0-little | Various | Old a.out little-endian |

### BSD a.out Variants

- bout (b.out - early BSD)
- newsos3 (Sony NEWS)
- pc532-mach
- pdp11

### SunOS a.out

- sunos-big
- sunos-little

### NetBSD a.out

- aout-ns32k-netbsd
- aout-sparc-netbsd
- aout-m68k-netbsd

---

## 4. Mach-O (Mach Object)

**Era:** 1980s - Present
**Primary System:** NeXTSTEP, macOS, iOS
**Type:** Modern Apple format

### Mach-O Variants

| Format | Bit Width | Architecture | Description |
|--------|-----------|--------------|-------------|
| mach-o-i386 | 32-bit | x86 | Intel 32-bit Mach-O |
| mach-o-x86-64 | 64-bit | x86-64 | Intel 64-bit Mach-O |
| mach-o-arm | 32-bit | ARM | ARM 32-bit Mach-O |
| mach-o-arm64 | 64-bit | ARM64 | ARM 64-bit Mach-O |
| mach-o-ppc | 32-bit | PowerPC | PowerPC Mach-O |
| mach-o-ppc64 | 64-bit | PowerPC64 | PowerPC 64-bit Mach-O |

### Universal Binaries (Fat Mach-O)

- mach-o-fat (multi-architecture containers)

---

## 5. OMF and Related Formats

**Era:** 1980s - 2000s
**Primary Systems:** DOS, OS/2, Windows 16-bit
**Type:** Intel/Microsoft object formats

### OMF (Object Module Format)

- omf-386 (Intel OMF-386)
- omf-86 (Intel OMF-86)

### DOS/Windows Executables

| Format | Era | System | Description |
|--------|-----|--------|-------------|
| MZ | 1981+ | DOS | DOS executable |
| NE | 1985+ | Windows 3.x, OS/2 | New Executable (16-bit) |
| LE | 1992+ | OS/2, Win9x VxD | Linear Executable |
| LX | 1994+ | OS/2 | Linear Executable Extended |

### Xenix Formats

- x.out (Xenix executable - NOT COFF variant)
- coff-Intel-little (Xenix COFF)
- coff-Intel-big (Xenix COFF BE)

---

## 6. Specialized and Embedded Formats

### Motorola S-Records

- srec (Motorola S-record)
- symbolsrec (S-record with symbols)

### Intel Hex

- ihex (Intel HEX format)
- tekhex (Tektronix extended HEX)

### Verilog Hex

- verilog (Verilog memory format)

### Binary/Raw

- binary (raw binary)
- rawbin (raw binary data)

---

## 7. Archive Formats

### Unix Archives

- archive (traditional Unix ar)
- aixcoff-archive (AIX COFF archive)

### Windows Libraries

- pe-i386.lib
- pe-x86-64.lib

---

## 8. Platform-Specific Formats

### HPPA (HP PA-RISC)

**Era:** 1990s - 2000s
**System:** HP-UX

- som (System Object Module)
- som-big
- som-little

### VAX

**Era:** 1970s - 1990s
**System:** VMS, BSD

- vax-bsd
- vms-alpha
- vms-vax

### PDP-11

**Era:** 1970s - 1980s
**System:** Unix v6/v7

- pdp11 (PDP-11 a.out)

### Tandem

- tandem-coff

### TI COFF

- tic30-coff
- tic4x-coff
- tic54x-coff
- tic80-coff

---

## 9. Real-Time and Embedded Systems

### VxWorks

- vxworks

### RTEMS

- rtems-elf

### Bare Metal

- elf32-little (bare metal)
- elf64-little (bare metal)

---

## 10. Special Purpose Formats

### Plugin Format

- plugin (BFD plugin interface)

### Core Dumps

- elf32-*-core
- elf64-*-core
- aix5coff64-core

### Debug Formats

- coff-stgo32 (DJGPP debug)
- pe-*-debug

---

## Format Support by Architecture

### x86 (Intel 32-bit)

- ELF: elf32-i386
- PE/COFF: pe-i386, pei-i386
- COFF: coff-i386
- a.out: a.out-i386-linux
- Mach-O: mach-o-i386

### x86-64 (AMD64)

- ELF: elf64-x86-64
- PE/COFF: pe-x86-64, pei-x86-64
- COFF: coff-x86-64
- Mach-O: mach-o-x86-64

### ARM 32-bit

- ELF: elf32-littlearm, elf32-bigarm
- PE/COFF: pe-arm, pei-arm
- COFF: coff-arm
- Mach-O: mach-o-arm
- a.out: a.out-arm-linux

### ARM 64-bit (AArch64)

- ELF: elf64-littleaarch64, elf64-bigaarch64
- PE/COFF: pe-arm64, pei-arm64
- Mach-O: mach-o-arm64

### MIPS

- ELF: elf32-tradbigmips, elf32-tradlittlemips, elf64-bigmips, elf64-littlemips
- ECOFF: ecoff-bigmips, ecoff-littlemips

### PowerPC

- ELF: elf32-powerpc, elf64-powerpc
- XCOFF: aixcoff-rs6000, aixcoff64-rs6000
- Mach-O: mach-o-ppc, mach-o-ppc64

### SPARC

- ELF: elf32-sparc, elf64-sparc
- a.out: sunos-big, aout-sparc-netbsd

### RISC-V

- ELF: elf32-littleriscv, elf64-littleriscv

### Alpha

- ELF: elf64-alpha
- ECOFF: ecoff-littlealpha
- VMS: vms-alpha

---

## Historical Timeline

### 1970s

- a.out format introduced (Unix v1-v6)
- COFF development begins (Unix System III)

### 1980s

- COFF standardized (System V Release 3)
- MZ format (1981 - MS-DOS)
- OMF-86 introduced
- NE format (1985 - Windows 1.0)
- Mach-O created for NeXTSTEP (1988)

### 1990s

- ELF specification published (1992)
- PE/COFF introduced (1993 - Windows NT 3.1)
- LE/LX formats (OS/2)
- Linux adopts ELF (1995)
- XCOFF64 for 64-bit AIX
- Fat Mach-O for PowerPC/x86 transition

### 2000s

- ELF becomes dominant on Unix-like systems
- TE format for UEFI
- PE+ (PE32+) for 64-bit Windows
- Mach-O becomes primary format for macOS/iOS

### 2010s-2020s

- ARM64 support across all major formats
- RISC-V ELF support
- BigObj COFF for large objects
- Universal binaries for Apple Silicon transition

---

## Format Characteristics Comparison

| Format | Relocatable | Executable | Shared Library | Core Dump | Multi-Arch |
|--------|-------------|------------|----------------|-----------|------------|
| ELF | ✓ | ✓ | ✓ | ✓ | ✗ (FatELF unofficial) |
| PE/COFF | ✓ | ✓ | ✓ | ✗ | ✗ |
| Mach-O | ✓ | ✓ | ✓ | ✓ | ✓ (Fat) |
| a.out | ✓ | ✓ | Limited | ✓ | ✗ |
| COFF | ✓ | ✓ | ✗ | ✗ | ✗ |
| XCOFF | ✓ | ✓ | ✓ | ✓ | ✗ |
| OMF | ✓ | ✗ | ✗ | ✗ | ✗ |

---

## Binutils Tool Support

### objdump

Supports all formats listed for disassembly and analysis.

### objcopy

Can convert between compatible formats:
- ELF ↔ binary
- PE ↔ ELF (limited)
- a.out → ELF
- srec/ihex ↔ binary

### nm

Symbol table extraction for all formats with symbol tables.

### ar

Archive creation/manipulation:
- Traditional Unix archives
- Windows .lib files
- AIX big archives

### ranlib

Index generation for archives (all archive formats).

### strip

Debug information removal (all formats with debug sections).

---

## Notes

1. **Multi-Architecture Support**: Only Mach-O (Fat) and unofficial FatELF support multiple architectures in a single file natively.

2. **Debug Information**: Most formats support DWARF debug information; PE/COFF can use CodeView or DWARF.

3. **Dynamic Linking**: ELF, PE/COFF, and Mach-O have full dynamic linking support. a.out has limited support.

4. **Platform Migration**: Many systems have migrated:
   - Linux: a.out → ELF (1995)
   - Solaris: a.out → ELF (1990s)
   - macOS: Mach-O throughout
   - Windows: PE/COFF throughout

5. **Embedded Systems**: Tend to use ELF or raw binary formats.

---

## References

- GNU Binutils documentation
- BFD library source code
- System V ABI specifications
- PE/COFF specification (Microsoft)
- Mach-O programming topics (Apple)
- ELF specification (Tool Interface Standard)
