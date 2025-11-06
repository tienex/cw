# LibElf - ELF Binary Format Library

## Overview

LibElf provides comprehensive support for parsing and creating ELF (Executable and Linkable Format) binaries, the standard binary format for Unix and Unix-like systems.

## Supported Formats

- **ELF32**: 32-bit ELF binaries
- **ELF64**: 64-bit ELF binaries
- **FatELF**: Multi-architecture ELF containers
- **x32 ABI**: ILP32 data model on x86-64 (32-bit pointers with 64-bit instructions)

## OS/ABI Support (18 variants)

| ABI Identifier | Description | Since |
|----------------|-------------|-------|
| ELFOSABI_SYSV | System V / Generic Unix | 1986 |
| ELFOSABI_HPUX | HP-UX | 1986 |
| ELFOSABI_NETBSD | NetBSD | 1993 |
| ELFOSABI_GNU/LINUX | GNU/Linux | 1991 |
| ELFOSABI_SOLARIS | Solaris | 1992 |
| ELFOSABI_AIX | IBM AIX | 1986 |
| ELFOSABI_IRIX | SGI IRIX | 1988 |
| ELFOSABI_FREEBSD | FreeBSD | 1993 |
| ELFOSABI_TRU64 | Tru64 UNIX (Digital UNIX/OSF/1) | 1992 |
| ELFOSABI_MODESTO | Novell Modesto | 2000s |
| ELFOSABI_OPENBSD | OpenBSD | 1995 |
| ELFOSABI_OPENVMS | OpenVMS | 2003 |
| ELFOSABI_NSK | HP Non-Stop Kernel | 1970s |
| ELFOSABI_AROS | AROS Research Operating System | 1995 |
| ELFOSABI_FENIXOS | FenixOS | 2000s |
| ELFOSABI_CLOUDABI | CloudABI | 2015 |
| ELFOSABI_ARM | ARM EABI | 2003 |
| ELFOSABI_STANDALONE | Standalone/Embedded | Various |

## Machine Types (100+ architectures)

### Major Production Architectures
- **EM_386** (3): Intel 80386 and later (i386, i486, Pentium, etc.)
- **EM_X86_64** (62): AMD/Intel x86-64 (x64, x86_64)
- **EM_ARM** (40): ARM 32-bit (ARMv4-ARMv7)
- **EM_AARCH64** (183): ARM 64-bit (ARMv8-A and later)
- **EM_PPC** (20): PowerPC 32-bit
- **EM_PPC64** (21): PowerPC 64-bit
- **EM_MIPS** (8): MIPS RS3000 and later
- **EM_SPARC** (2): SPARC 32-bit
- **EM_SPARCV9** (43): SPARC v9 64-bit
- **EM_RISCV** (243): RISC-V
- **EM_IA_64** (50): Intel Itanium IA-64
- **EM_S390** (22): IBM System/390 and z/Architecture
- **EM_ALPHA** (41): DEC Alpha
- **EM_PARISC** (15): HP PA-RISC
- **EM_SH** (42): Hitachi SuperH
- **EM_LOONGARCH** (258): LoongArch
- **EM_BPF** (247): Linux BPF (Berkeley Packet Filter)

### Historical and Embedded Architectures
- **EM_M32** (1): AT&T WE 32100
- **EM_68K** (4): Motorola 68000
- **EM_88K** (5): Motorola 88000
- **EM_860** (7): Intel 80860
- **EM_MMIX** (80): Donald Knuth's MMIX educational architecture
- **EM_VAX** (75): DEC VAX
- **EM_PDP10** (64): DEC PDP-10
- **EM_PDP11** (65): DEC PDP-11
- **EM_AVR** (83): Atmel AVR 8-bit microcontroller
- **EM_XTENSA** (94): Tensilica Xtensa
- **EM_OPENRISC** (92): OpenRISC
- **EM_CSKY** (252): C-SKY
- **EM_NS32K** (97): National Semiconductor 32000
- **EM_TRICORE** (44): Siemens TriCore
- **EM_ARC** (45): Argonaut RISC Core
- **EM_V850** (87): NEC V850
- **EM_M32R** (88): Renesas M32R
- **EM_CRIS** (76): Axis CRIS
- **EM_SPU** (23): Sony/Toshiba/IBM Cell SPU

Plus 70+ additional historical, embedded, and specialized architectures.

## ELF Note Types

### Core Dump Notes
Used in PT_NOTE segments of core dumps to save process state:

- **NT_PRSTATUS** (1): Process status (registers, signal info)
- **NT_FPREGSET** (2): Floating-point registers
- **NT_PRPSINFO** (3): Process info (PID, command name, etc.)
- **NT_AUXV** (6): Auxiliary vector
- **NT_SIGINFO** (0x53494749): Signal information
- **NT_FILE** (0x46494c45): Mapped files list

### Architecture-Specific Register Notes

#### x86/x64
- **NT_386_TLS** (0x200): Thread-local storage slots
- **NT_386_IOPERM** (0x201): I/O permission bitmap
- **NT_X86_XSTATE** (0x202): Extended state (XSAVE: AVX, AVX-512, AMX)
- **NT_PRXFPREG** (0x46e62b7f): x87 FPU state (fxsave format)

#### PowerPC
- **NT_PPC_VMX** (0x100): Altivec/VMX vector registers
- **NT_PPC_SPE** (0x101): SPE (Signal Processing Engine)
- **NT_PPC_VSX** (0x102): VSX (Vector Scalar Extension)
- **NT_PPC_TAR** (0x103): Target Address Register
- **NT_PPC_PPR** (0x104): Program Priority Register
- **NT_PPC_DSCR** (0x105): Data Stream Control Register
- **NT_PPC_EBB** (0x106): Event-Based Branch registers
- **NT_PPC_PMU** (0x107): Performance Monitor Unit
- **NT_PPC_TM_CGPR** (0x108): Transactional Memory checkpointed GPRs
- **NT_PPC_TM_CFPR** (0x109): TM checkpointed FPRs
- **NT_PPC_TM_CVMX** (0x10a): TM checkpointed VMX
- **NT_PPC_TM_CVSX** (0x10b): TM checkpointed VSX
- **NT_PPC_TM_SPR** (0x10c): TM special purpose registers

#### ARM
- **NT_ARM_VFP** (0x400): VFP/NEON registers
- **NT_ARM_TLS** (0x401): Thread-local storage
- **NT_ARM_HW_BREAK** (0x402): Hardware breakpoints
- **NT_ARM_HW_WATCH** (0x403): Hardware watchpoints
- **NT_ARM_SVE** (0x405): Scalable Vector Extension registers
- **NT_ARM_PAC_MASK** (0x406): Pointer authentication code masks
- **NT_ARM_PACA_KEYS** (0x407): Pointer auth address keys
- **NT_ARM_PACG_KEYS** (0x408): Pointer auth generic keys
- **NT_ARM_TAGGED_ADDR_CTRL** (0x409): Tagged address control
- **NT_ARM_PAC_ENABLED_KEYS** (0x40a): Enabled pointer auth keys
- **NT_ARM_SSVE** (0x40b): Streaming SVE registers
- **NT_ARM_ZA** (0x40c): SME ZA matrix array register
- **NT_ARM_ZT** (0x40d): SME2 ZT lookup table registers

#### S/390 (IBM Z)
- **NT_S390_HIGH_GPRS** (0x300): Upper halves of 64-bit GPRs
- **NT_S390_TIMER** (0x301): CPU timer
- **NT_S390_TODCMP** (0x302): TOD comparator
- **NT_S390_TODPREG** (0x303): TOD programmable register
- **NT_S390_CTRS** (0x304): Control registers
- **NT_S390_PREFIX** (0x305): Prefix register
- **NT_S390_LAST_BREAK** (0x306): Breaking event address
- **NT_S390_SYSTEM_CALL** (0x307): System call restart data
- **NT_S390_TDB** (0x308): Transaction diagnostic block
- **NT_S390_VXRS_LOW** (0x309): Vector registers 0-15 lower half
- **NT_S390_VXRS_HIGH** (0x30a): Vector registers 16-31

#### RISC-V
- **NT_RISCV_CSR** (0x900): Control and status registers

### GNU Toolchain Notes

- **NT_GNU_ABI_TAG** (1): Operating system ABI tag
- **NT_GNU_HWCAP** (2): Hardware capabilities
- **NT_GNU_BUILD_ID** (3): Unique build identifier (SHA1/MD5)
- **NT_GNU_GOLD_VERSION** (4): Gold linker version
- **NT_GNU_PROPERTY_TYPE_0** (5): Program properties

## Dynamic Linking Support

### Core Dynamic Tags (34 standard entries)

| Tag | Name | Description |
|-----|------|-------------|
| DT_NULL (0) | End | End of dynamic array |
| DT_NEEDED (1) | Needed library | Name of needed shared library |
| DT_PLTRELSZ (2) | PLT size | Size in bytes of PLT relocations |
| DT_PLTGOT (3) | PLT/GOT | Address of PLT and/or GOT |
| DT_HASH (4) | Hash table | Address of symbol hash table |
| DT_STRTAB (5) | String table | Address of string table |
| DT_SYMTAB (6) | Symbol table | Address of symbol table |
| DT_RELA (7) | RELA relocs | Address of relocation table (addend) |
| DT_RELASZ (8) | RELA size | Total size of RELA relocations |
| DT_RELAENT (9) | RELA entry | Size of one RELA entry |
| DT_STRSZ (10) | String size | Size of string table |
| DT_SYMENT (11) | Symbol entry | Size of one symbol entry |
| DT_INIT (12) | Init function | Address of initialization function |
| DT_FINI (13) | Fini function | Address of termination function |
| DT_SONAME (14) | Shared object name | String table offset of SO name |
| DT_RPATH (15) | Library path | String table offset of search path (deprecated) |
| DT_SYMBOLIC (16) | Symbolic | Start symbol search here |
| DT_REL (17) | REL relocs | Address of REL relocation table |
| DT_RELSZ (18) | REL size | Total size of REL relocations |
| DT_RELENT (19) | REL entry | Size of one REL entry |
| DT_PLTREL (20) | PLT reloc type | Type of relocation in PLT |
| DT_DEBUG (21) | Debug | For debugging |
| DT_TEXTREL (22) | Text reloc | Relocations might modify .text |
| DT_JMPREL (23) | Jump reloc | Address of PLT relocation entries |
| DT_BIND_NOW (24) | Bind now | Process all relocations at load time |
| DT_INIT_ARRAY (25) | Init array | Address of array of init functions |
| DT_FINI_ARRAY (26) | Fini array | Address of array of termination functions |
| DT_INIT_ARRAYSZ (27) | Init array size | Size in bytes of DT_INIT_ARRAY |
| DT_FINI_ARRAYSZ (28) | Fini array size | Size in bytes of DT_FINI_ARRAY |
| DT_RUNPATH (29) | Run path | String table offset of search path |
| DT_FLAGS (30) | Flags | Flag values |
| DT_PREINIT_ARRAY (32) | Preinit array | Address of preinit function array |
| DT_PREINIT_ARRAYSZ (33) | Preinit size | Size in bytes of preinit array |
| DT_SYMTAB_SHNDX (34) | Symbol indices | Address of extended section indices |

### GNU Extensions (20+ entries)

- **DT_GNU_HASH** (0x6ffffef5): GNU-style hash table (faster than SysV hash)
- **DT_GNU_PRELINKED** (0x6ffffdf5): Prelinking timestamp
- **DT_VERSYM** (0x6ffffff0): Version symbol table
- **DT_VERDEF** (0x6ffffffc): Version definition table
- **DT_VERDEFNUM** (0x6ffffffd): Number of version definitions
- **DT_VERNEED** (0x6ffffffe): Version dependency table
- **DT_VERNEEDNUM** (0x6fffffff): Number of version dependencies
- **DT_TLSDESC_PLT** (0x6ffffef6): PLT entry for TLS descriptor
- **DT_TLSDESC_GOT** (0x6ffffef7): GOT entry for TLS descriptor
- **DT_RELACOUNT** (0x6ffffff9): Count of RELATIVE relocations
- **DT_RELCOUNT** (0x6ffffffa): Count of RELATIVE relocations
- **DT_FLAGS_1** (0x6ffffffb): Extended state flags

### Dynamic Flags

**DF_* (DT_FLAGS values):**
- **DF_ORIGIN** (0x1): Object may use $ORIGIN in RPATH/RUNPATH
- **DF_SYMBOLIC** (0x2): Symbol resolution starts in this object
- **DF_TEXTREL** (0x4): Contains relocations to .text section
- **DF_BIND_NOW** (0x8): Non-lazy binding (resolve all symbols at load)
- **DF_STATIC_TLS** (0x10): Uses static thread-local storage model

**DF_1_* (DT_FLAGS_1 values - 25+ flags):**
- **DF_1_NOW** (0x1): Complete relocation processing at load time
- **DF_1_GLOBAL** (0x2): Set RTLD_GLOBAL flag
- **DF_1_GROUP** (0x4): Set RTLD_GROUP flag
- **DF_1_NODELETE** (0x8): Cannot be deleted from a process
- **DF_1_LOADFLTR** (0x10): Immediate filtee processing
- **DF_1_INITFIRST** (0x20): Initialize before all others
- **DF_1_NOOPEN** (0x40): Cannot be dlopened
- **DF_1_ORIGIN** (0x80): $ORIGIN processing required
- **DF_1_DIRECT** (0x100): Direct binding enabled
- **DF_1_INTERPOSE** (0x400): Object is an interposer
- **DF_1_NODEFLIB** (0x800): Ignore default library search path
- **DF_1_PIE** (0x8000000): Position Independent Executable
- **DF_1_SINGLETON** (0x2000000): Singleton object (only one instance)
- **DF_1_STUB** (0x4000000): Stub object

## Sources and References

### Official Specifications
- **System V ABI** (1986-present): Original ELF specification
  - Generic ABI: https://refspecs.linuxfoundation.org/elf/gabi4+/
  - AMD64 ABI: https://refspecs.linuxfoundation.org/elf/x86_64-abi-0.99.pdf
  - i386 ABI: https://refspecs.linuxfoundation.org/elf/abi386-4.pdf
- **ARM ABI**: https://github.com/ARM-software/abi-aa
- **RISC-V psABI**: https://github.com/riscv-non-isa/riscv-elf-psabi-doc
- **PowerPC64 ABI**: https://openpowerfoundation.org/specifications/
- **SPARC ABI**: https://sparc.org/standards/

### Kernel Sources
- **Linux kernel** (include/uapi/linux/elf.h): Note types, core dump structures
  - Repository: https://kernel.org/
- **FreeBSD** (sys/elf_common.h): BSD-specific extensions
- **NetBSD** (sys/exec_elf.h): NetBSD ELF extensions
- **OpenBSD** (sys/exec_elf.h): OpenBSD ELF extensions

### Toolchain Sources
- **GNU binutils** (include/elf/*.h): Machine types, relocations, dynamic tags
  - Repository: https://sourceware.org/git/binutils-gdb.git
- **glibc** (elf/elf.h): Dynamic linking definitions
  - Repository: https://sourceware.org/git/glibc.git
- **LLVM** (include/llvm/BinaryFormat/ELF.h): Modern ELF definitions
  - Repository: https://github.com/llvm/llvm-project

### Historical References
- **Tool Interface Standard (TIS) ELF Specification** (1995)
- **SCO System V ABI** (1996)
- **Solaris Linker and Libraries Guide**: Oracle documentation
- **IBM AIX documentation**: Power ABI

### Books
- *Linkers and Loaders* by John R. Levine (1999)
- *ELF-64 Object File Format* by HP/Intel/SCO (1998)

## Relocation Types

LibElf includes comprehensive relocation type definitions for all supported architectures:

- **x86**: 44 relocation types (R_386_*)
- **x86-64**: 43 relocation types (R_X86_64_*)
- **ARM**: 131 relocation types (R_ARM_*)
- **AArch64**: 48 relocation types (R_AARCH64_*)
- **PowerPC**: 96 (32-bit) + 153 (64-bit) types
- **MIPS**: 51 relocation types
- **SPARC**: 80 relocation types
- **RISC-V**: 58 relocation types
- **IA-64**: 80 relocation types
- **Alpha**: 41 relocation types
- **M68K**: 43 relocation types
- **S/390**: 61 relocation types
- **VAX**: 13 relocation types
- **SH (SuperH)**: 38 relocation types
- **AVR**: 34 relocation types
- **Xtensa**: 57 relocation types
- **OpenRISC**: 35 relocation types

See LibElf.c source for complete relocation type listings.

## Integration with CoreContext.h

LibElf note types directly map to the NT-style CONTEXT structures defined in CoreContext.h, enabling:
- Reading core dump notes into universal CONTEXT structures
- Writing CONTEXT structures back to core dump notes
- Converting between Linux/BSD core dumps and Windows minidumps
- Cross-platform debugging and analysis

## Version History

- **v1.0** (2025): Initial comprehensive implementation
  - Complete ELF32/ELF64 support
  - 100+ machine types
  - Full relocation support for 18 architectures
  - Comprehensive note type definitions
  - Complete dynamic linking support
  - x32 ABI support

## License

MIT License - See SPDX-License-Identifier in source files.
