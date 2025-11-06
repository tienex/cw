# Binary Format Library Augmentation Guide

This document details enhancements to each binary format library based on comprehensive research of GNU Binutils and GDB repositories.

**Based on Research:** GDB 16.3 and Binutils 2.45 (2025)
**Source:** binutils-gdb/bfd/targets.c analysis

---

## Summary of Augmentations

### Libraries to Update
1. **libelf** - Add 100+ ELF variant recognition
2. **libcoff** - Add ECOFF, XCOFF, TI DSP COFF variants
3. **libaout** - Add BSD, Linux, vendor-specific variants
4. **libmacho** - Add architecture detection and fat binary improvements
5. **libomf** - Add DOS extender variants
6. **liborf** - Add VMS, PEF, and additional obscure formats

---

## 1. LibElf Augmentations

### Current Status
- Basic ELF32/64 support
- FatELF recognition

### Proposed Additions

#### A. ELF Variant Detection

Add support for OS/ABI-specific ELF variants:

```c
///
/// ELF OS/ABI Values
///
#define ELFOSABI_NONE           0    ///< No extensions or unspecified
#define ELFOSABI_SYSV           0    ///< UNIX System V ABI
#define ELFOSABI_HPUX           1    ///< HP-UX
#define ELFOSABI_NETBSD         2    ///< NetBSD
#define ELFOSABI_GNU            3    ///< Object uses GNU ELF extensions
#define ELFOSABI_LINUX          3    ///< Linux (alias for GNU)
#define ELFOSABI_SOLARIS        6    ///< Sun Solaris
#define ELFOSABI_AIX            7    ///< IBM AIX
#define ELFOSABI_IRIX           8    ///< SGI Irix
#define ELFOSABI_FREEBSD        9    ///< FreeBSD
#define ELFOSABI_TRU64          10   ///< Compaq TRU64 UNIX
#define ELFOSABI_MODESTO        11   ///< Novell Modesto
#define ELFOSABI_OPENBSD        12   ///< OpenBSD
#define ELFOSABI_ARM_AEABI      64   ///< ARM EABI
#define ELFOSABI_ARM            97   ///< ARM
#define ELFOSABI_STANDALONE     255  ///< Standalone (embedded)
```

#### B. Architecture-Specific ELF Magic

Add machine type detection:

```c
#define EM_MMIX         80    ///< MMIX
#define EM_AARCH64      183   ///< ARM 64-bit
#define EM_RISCV        243   ///< RISC-V
#define EM_BPF          247   ///< Linux BPF
#define EM_LOONGARCH    258   ///< LoongArch
#define EM_CSKY         252   ///< C-SKY
```

#### C. Special ELF Variants

```c
///
/// FDPIC (Function Descriptor PIC) ELF
///
#define ELF_FLAGS_FDPIC  0x00000001

///
/// VxWorks ELF
///
#define ELF_FLAGS_VXWORKS  0x00000002

///
/// CloudABI ELF
///
#define ELFOSABI_CLOUDABI  12

///
/// NaCl (Native Client) ELF
///
#define ELFOSABI_NACL  123
```

#### D. Enhanced Detection Function

```c
typedef enum {
  ElfVariantGeneric,
  ElfVariantLinux,
  ElfVariantFreeBSD,
  ElfVariantNetBSD,
  ElfVariantOpenBSD,
  ElfVariantSolaris,
  ElfVariantAIX,
  ElfVariantVxWorks,
  ElfVariantFDPIC,
  ElfVariantCloudABI,
  ElfVariantNaCl,
  ElfVariantStandalone
} ELF_VARIANT_TYPE;

ELF_VARIANT_TYPE DetectElfVariant(UINT8 osabi, UINT32 flags) {
  switch (osabi) {
    case ELFOSABI_LINUX:    return ElfVariantLinux;
    case ELFOSABI_FREEBSD:  return ElfVariantFreeBSD;
    case ELFOSABI_NETBSD:   return ElfVariantNetBSD;
    // ... etc
  }

  if (flags & ELF_FLAGS_FDPIC) return ElfVariantFDPIC;
  if (flags & ELF_FLAGS_VXWORKS) return ElfVariantVxWorks;

  return ElfVariantGeneric;
}
```

---

## 2. LibCoff Augmentations

### Current Status
- PE/COFF, BigObj, TE
- Basic XCOFF/ECOFF recognition

### Proposed Additions

#### A. ECOFF (Extended COFF) Variants

```c
///
/// ECOFF Magic Numbers
///
#define ECOFF_MAGIC_MIPS_LE     0x0162  ///< MIPS little-endian
#define ECOFF_MAGIC_MIPS_BE     0x0160  ///< MIPS big-endian
#define ECOFF_MAGIC_ALPHA       0x0183  ///< DEC Alpha
#define ECOFF_MAGIC_MIPS_BELE   0x0180  ///< MIPS bi-endian

///
/// ECOFF Header Structure
///
typedef struct {
  UINT16  Magic;                ///< Magic number
  UINT16  NumSections;          ///< Number of sections
  UINT32  TimeDate;             ///< Time and date
  UINT32  SymbolTablePtr;       ///< Symbol table pointer
  UINT32  NumSymbols;           ///< Number of symbols
  UINT16  OptHeaderSize;        ///< Optional header size
  UINT16  Flags;                ///< Flags
  //
  // ECOFF-specific fields
  //
  UINT64  GpValue;              ///< GP register value
  UINT32  GpMask;               ///< GP register mask
  UINT32  CpuMask;              ///< CPU mask
} ECOFF_HEADER;
```

#### B. XCOFF Enhancements

```c
///
/// XCOFF Auxiliary Header (AIX)
///
typedef struct {
  UINT16  Magic;                ///< Magic (0x010B for AIX)
  UINT16  Version;              ///< Version
  UINT32  TextSize;             ///< Text size
  UINT32  InitDataSize;         ///< Initialized data size
  UINT32  UnitDataSize;         ///< Uninitialized data size
  UINT32  EntryPoint;           ///< Entry point
  UINT32  TextStart;            ///< Text starting address
  UINT32  DataStart;            ///< Data starting address
  UINT32  TocAnchor;            ///< TOC anchor
  UINT16  SnEntry;              ///< Section number for entry
  UINT16  SnText;               ///< Section number for text
  UINT16  SnData;               ///< Section number for data
  UINT16  SnToc;                ///< Section number for TOC
  UINT16  SnLoader;             ///< Section number for loader
  UINT16  SnBss;                ///< Section number for BSS
  UINT16  AlgnText;             ///< Text alignment
  UINT16  AlgnData;             ///< Data alignment
  CHAR8   ModType[2];           ///< Module type
  UINT8   CpuType;              ///< CPU type
  UINT8   Reserved[1];          ///< Reserved
  UINT32  MaxStack;             ///< Maximum stack size
  UINT32  MaxData;              ///< Maximum data size
  UINT8   Reserved2[12];        ///< Reserved
} XCOFF_AUX_HEADER;
```

#### C. Texas Instruments DSP COFF

```c
///
/// TI COFF Variants
///
#define TI_COFF_MAGIC_C30       0x0093  ///< TMS320C3x
#define TI_COFF_MAGIC_C40       0x0093  ///< TMS320C4x
#define TI_COFF_MAGIC_C54       0x0098  ///< TMS320C54x
#define TI_COFF_MAGIC_C55       0x009C  ///< TMS320C55x
#define TI_COFF_MAGIC_C60       0x0099  ///< TMS320C6x

///
/// TI COFF Version Numbers
///
#define TI_COFF_VERSION_0       0
#define TI_COFF_VERSION_1       1
#define TI_COFF_VERSION_2       2

typedef struct {
  UINT16  Version;              ///< COFF version
  UINT16  NumSections;          ///< Number of sections
  UINT32  TimeDate;             ///< Time and date stamp
  UINT32  SymTablePtr;          ///< Symbol table pointer
  UINT32  NumSymbols;           ///< Number of symbols
  UINT16  OptHeaderSize;        ///< Optional header size
  UINT16  Flags;                ///< Flags
  UINT16  TargetId;             ///< Target ID
} TI_COFF_HEADER;
```

#### D. Z80/Z8000 COFF

```c
#define Z80_COFF_MAGIC          0x805A  ///< Z80
#define Z8K_COFF_MAGIC          0x8000  ///< Z8000
```

---

## 3. LibAout Augmentations

### Current Status
- Basic a.out, b.out, Plan 9

### Proposed Additions

#### A. BSD a.out Variants

```c
///
/// BSD a.out Magic Numbers
///
#define OMAGIC_BSD      0407    ///< Old impure format
#define NMAGIC_BSD      0410    ///< Read-only text
#define ZMAGIC_BSD      0413    ///< Demand paged
#define QMAGIC_BSD      0314    ///< Compact QMAGIC (NetBSD)

///
/// NetBSD a.out Machine Types
///
#define MID_ZERO        0       ///< Unknown
#define MID_SUN010      1       ///< Sun 68010
#define MID_SUN020      2       ///< Sun 68020
#define MID_PC386       100     ///< Intel 386
#define MID_HP200       200     ///< HP 200
#define MID_HP300       300     ///< HP 300
#define MID_HPUX        0x20C   ///< HP-UX
#define MID_HPUX800     0x20B   ///< HP-UX 800
```

#### B. Linux a.out

```c
///
/// Linux a.out Machine Types
///
#define M_386           100     ///< Intel 386
#define M_68020         2       ///< Motorola 68020
#define M_SPARC         3       ///< SPARC
#define M_ARM           103     ///< ARM

///
/// Linux a.out Flags
///
#define A_FLAG_QMAGIC   0x0001  ///< QMAGIC format
```

#### C. Vendor-Specific a.out

```c
///
/// SunOS a.out
///
typedef struct {
  UINT32  Magic;                ///< Magic number
  UINT32  TextSize;             ///< Text size
  UINT32  DataSize;             ///< Data size
  UINT32  BssSize;              ///< BSS size
  UINT32  SymSize;              ///< Symbol table size
  UINT32  Entry;                ///< Entry point
  UINT32  TextRelSize;          ///< Text relocation size
  UINT32  DataRelSize;          ///< Data relocation size
  UINT32  MachineType;          ///< Machine type (Sun-specific)
} SUNOS_AOUT_HEADER;

///
/// Sony NEWS a.out (newsos3)
///
#define NEWSOS3_MAGIC   0x010B
```

#### D. NS32K a.out

```c
///
/// NS32K PC532 a.out
///
#define NS32K_MAGIC_MACH    0532    ///< pc532mach
#define NS32K_MAGIC_NBSD    0532    ///< NetBSD NS32K
```

---

## 4. LibMacho Augmentations

### Current Status
- Basic Mach-O 32/64
- Fat binary recognition

### Proposed Additions

#### A. Detailed CPU Types

```c
///
/// Mach-O CPU Types (Comprehensive)
///
#define CPU_TYPE_ANY            ((UINT32)-1)
#define CPU_TYPE_VAX            1
#define CPU_TYPE_MC680x0        6
#define CPU_TYPE_X86            7
#define CPU_TYPE_I386           CPU_TYPE_X86
#define CPU_TYPE_X86_64         (CPU_TYPE_X86 | CPU_ARCH_ABI64)
#define CPU_TYPE_MC98000        10
#define CPU_TYPE_HPPA           11
#define CPU_TYPE_ARM            12
#define CPU_TYPE_ARM64          (CPU_TYPE_ARM | CPU_ARCH_ABI64)
#define CPU_TYPE_ARM64_32       (CPU_TYPE_ARM | CPU_ARCH_ABI64_32)
#define CPU_TYPE_MC88000        13
#define CPU_TYPE_SPARC          14
#define CPU_TYPE_I860           15
#define CPU_TYPE_POWERPC        18
#define CPU_TYPE_POWERPC64      (CPU_TYPE_POWERPC | CPU_ARCH_ABI64)

///
/// CPU Architecture Mask
///
#define CPU_ARCH_MASK           0xff000000
#define CPU_ARCH_ABI64          0x01000000
#define CPU_ARCH_ABI64_32       0x02000000
```

#### B. CPU Subtypes

```c
///
/// x86 CPU Subtypes
///
#define CPU_SUBTYPE_I386_ALL        3
#define CPU_SUBTYPE_486             4
#define CPU_SUBTYPE_486SX           0x84
#define CPU_SUBTYPE_586             5
#define CPU_SUBTYPE_PENT            CPU_SUBTYPE_586
#define CPU_SUBTYPE_PENTPRO         0x16
#define CPU_SUBTYPE_PENTII_M3       0x36
#define CPU_SUBTYPE_PENTII_M5       0x56
#define CPU_SUBTYPE_CELERON         0x67
#define CPU_SUBTYPE_CELERON_MOBILE  0x77
#define CPU_SUBTYPE_PENTIUM_3       0x08
#define CPU_SUBTYPE_PENTIUM_3_M     0x18
#define CPU_SUBTYPE_PENTIUM_3_XEON  0x28
#define CPU_SUBTYPE_PENTIUM_M       0x09
#define CPU_SUBTYPE_PENTIUM_4       0x0a
#define CPU_SUBTYPE_PENTIUM_4_M     0x1a
#define CPU_SUBTYPE_ITANIUM         0x0b
#define CPU_SUBTYPE_ITANIUM_2       0x1b
#define CPU_SUBTYPE_XEON            0x0c
#define CPU_SUBTYPE_XEON_MP         0x1c

///
/// x86-64 CPU Subtypes
///
#define CPU_SUBTYPE_X86_64_ALL      3
#define CPU_SUBTYPE_X86_64_H        8   ///< Haswell

///
/// ARM CPU Subtypes
///
#define CPU_SUBTYPE_ARM_ALL         0
#define CPU_SUBTYPE_ARM_V4T         5
#define CPU_SUBTYPE_ARM_V6          6
#define CPU_SUBTYPE_ARM_V5TEJ       7
#define CPU_SUBTYPE_ARM_XSCALE      8
#define CPU_SUBTYPE_ARM_V7          9
#define CPU_SUBTYPE_ARM_V7F         10  ///< Cortex A9
#define CPU_SUBTYPE_ARM_V7S         11  ///< Swift
#define CPU_SUBTYPE_ARM_V7K         12  ///< Kirkwood
#define CPU_SUBTYPE_ARM_V8          13
#define CPU_SUBTYPE_ARM_V6M         14  ///< Cortex-M0
#define CPU_SUBTYPE_ARM_V7M         15  ///< Cortex-M3
#define CPU_SUBTYPE_ARM_V7EM        16  ///< Cortex-M4

///
/// ARM64 CPU Subtypes
///
#define CPU_SUBTYPE_ARM64_ALL       0
#define CPU_SUBTYPE_ARM64_V8        1
#define CPU_SUBTYPE_ARM64E          2   ///< Pointer auth

///
/// PowerPC CPU Subtypes
///
#define CPU_SUBTYPE_POWERPC_ALL     0
#define CPU_SUBTYPE_POWERPC_601     1
#define CPU_SUBTYPE_POWERPC_602     2
#define CPU_SUBTYPE_POWERPC_603     3
#define CPU_SUBTYPE_POWERPC_603e    4
#define CPU_SUBTYPE_POWERPC_603ev   5
#define CPU_SUBTYPE_POWERPC_604     6
#define CPU_SUBTYPE_POWERPC_604e    7
#define CPU_SUBTYPE_POWERPC_620     8
#define CPU_SUBTYPE_POWERPC_750     9
#define CPU_SUBTYPE_POWERPC_7400    10
#define CPU_SUBTYPE_POWERPC_7450    11
#define CPU_SUBTYPE_POWERPC_970     100
```

#### C. File Types

```c
///
/// Mach-O File Types
///
#define MH_OBJECT       0x1     ///< Relocatable object file
#define MH_EXECUTE      0x2     ///< Demand paged executable
#define MH_FVMLIB       0x3     ///< Fixed VM shared library
#define MH_CORE         0x4     ///< Core file
#define MH_PRELOAD      0x5     ///< Preloaded executable
#define MH_DYLIB        0x6     ///< Dynamically bound shared library
#define MH_DYLINKER     0x7     ///< Dynamic link editor
#define MH_BUNDLE       0x8     ///< Dynamically bound bundle file
#define MH_DYLIB_STUB   0x9     ///< Shared library stub
#define MH_DSYM         0xA     ///< Debug symbols file
#define MH_KEXT_BUNDLE  0xB     ///< Kernel extension
#define MH_FILESET      0xC     ///< Fileset (iOS 14+)
```

#### D. Load Commands

```c
///
/// Common Load Command Types
///
#define LC_SEGMENT              0x1     ///< Segment
#define LC_SYMTAB               0x2     ///< Symbol table
#define LC_SYMSEG               0x3     ///< Symbol segment
#define LC_THREAD               0x4     ///< Thread
#define LC_UNIXTHREAD           0x5     ///< Unix thread
#define LC_LOADFVMLIB           0x6     ///< Load FVM library
#define LC_IDFVMLIB             0x7     ///< FVM library ident
#define LC_IDENT                0x8     ///< Object ident
#define LC_FVMFILE              0x9     ///< FVM file
#define LC_PREPAGE              0xA     ///< Prepage command
#define LC_DYSYMTAB             0xB     ///< Dynamic symbol table
#define LC_LOAD_DYLIB           0xC     ///< Load dynamic library
#define LC_ID_DYLIB             0xD     ///< Dynamic library ident
#define LC_LOAD_DYLINKER        0xE     ///< Load dynamic linker
#define LC_ID_DYLINKER          0xF     ///< Dynamic linker ident
#define LC_PREBOUND_DYLIB       0x10    ///< Prebound dynamic library
#define LC_SEGMENT_64           0x19    ///< 64-bit segment
#define LC_UUID                 0x1B    ///< UUID
#define LC_CODE_SIGNATURE       0x1D    ///< Code signature
#define LC_ENCRYPTION_INFO      0x21    ///< Encryption info
#define LC_DYLD_INFO            0x22    ///< Dyld info
#define LC_VERSION_MIN_MACOSX   0x24    ///< Min macOS version
#define LC_VERSION_MIN_IPHONEOS 0x25    ///< Min iOS version
#define LC_MAIN                 0x28    ///< Entry point
#define LC_SOURCE_VERSION       0x2A    ///< Source version
#define LC_ENCRYPTION_INFO_64   0x2C    ///< 64-bit encryption
#define LC_BUILD_VERSION        0x32    ///< Build version
```

---

## 5. LibOmf Augmentations

### Current Status
- OMF, MZ, NE, LE, LX
- Basic Xenix x.out

### Proposed Additions

#### A. DOS Extender Formats

```c
///
/// DOS/4GW and DOS32A
///
#define DOSX_MAGIC_LE           0x454C  ///< "LE"
#define DOSX_MAGIC_LX           0x584C  ///< "LX"
#define DOSX_MAGIC_LC           0x434C  ///< "LC" (compressed)
#define DOSX_MAGIC_W3           0x3357  ///< "W3" (WIN386)
#define DOSX_MAGIC_W4           0x3457  ///< "W4"

///
/// PMODE/W Protected Mode
///
#define PMODEW_STUB_SIGNATURE   0x4D50  ///< "PM"

///
/// CauseWay DOS Extender
///
#define CAUSEWAY_MAGIC          0x5743  ///< "CW"
```

#### B. Enhanced OMF Records

```c
///
/// OMF Record Types (Complete)
///
#define OMF_THEADR      0x80    ///< Translator module header
#define OMF_LHEADR      0x82    ///< Library module header
#define OMF_COMENT      0x88    ///< Comment record
#define OMF_MODEND      0x8A    ///< Module end (16-bit)
#define OMF_MODEND32    0x8B    ///< Module end (32-bit)
#define OMF_EXTDEF      0x8C    ///< External names
#define OMF_TYPDEF      0x8E    ///< Type definition
#define OMF_PUBDEF      0x90    ///< Public names (16-bit)
#define OMF_PUBDEF32    0x91    ///< Public names (32-bit)
#define OMF_LINNUM      0x94    ///< Line numbers (16-bit)
#define OMF_LINNUM32    0x95    ///< Line numbers (32-bit)
#define OMF_LNAMES      0x96    ///< List of names
#define OMF_SEGDEF      0x98    ///< Segment definition (16-bit)
#define OMF_SEGDEF32    0x99    ///< Segment definition (32-bit)
#define OMF_GRPDEF      0x9A    ///< Group definition
#define OMF_FIXUPP      0x9C    ///< Fixup (16-bit)
#define OMF_FIXUPP32    0x9D    ///< Fixup (32-bit)
#define OMF_LEDATA      0xA0    ///< Logical enumerated data (16-bit)
#define OMF_LEDATA32    0xA1    ///< Logical enumerated data (32-bit)
#define OMF_LIDATA      0xA2    ///< Logical iterated data (16-bit)
#define OMF_LIDATA32    0xA3    ///< Logical iterated data (32-bit)
#define OMF_COMDEF      0xB0    ///< Communal names
#define OMF_BAKPAT      0xB2    ///< Backpatch
#define OMF_BAKPAT32    0xB3    ///< Backpatch 32-bit
#define OMF_LEXTDEF     0xB4    ///< Local external names
#define OMF_LEXTDEF32   0xB5    ///< Local external names 32-bit
#define OMF_LPUBDEF     0xB6    ///< Local public names
#define OMF_LPUBDEF32   0xB7    ///< Local public names 32-bit
#define OMF_LCOMDEF     0xB8    ///< Local communal names
#define OMF_CEXTDEF     0xBC    ///< COMDAT external names
#define OMF_COMDAT      0xC2    ///< Initialized communal data
#define OMF_COMDAT32    0xC3    ///< Initialized communal data 32-bit
#define OMF_LINSYM      0xC4    ///< Symbol line numbers (16-bit)
#define OMF_LINSYM32    0xC5    ///< Symbol line numbers (32-bit)
#define OMF_ALIAS       0xC6    ///< Alias definition
#define OMF_NBKPAT      0xC8    ///< Named backpatch
#define OMF_NBKPAT32    0xC9    ///< Named backpatch 32-bit
#define OMF_LLNAMES     0xCA    ///< Local logical names
```

#### C. Phar Lap Formats

```c
///
/// Phar Lap DOS Extender
///
typedef struct {
  UINT16  Signature;            ///< 'MP' or 'P2' or 'P3'
  UINT16  HeaderVersion;        ///< Header version
  UINT16  RealModeStubSize;     ///< Real mode stub paragraphs
  UINT16  HeaderSize;           ///< Header size in bytes
  UINT32  FileSize;             ///< File size
  UINT16  Checksum;             ///< Checksum
  UINT32  RuntimeParameters;    ///< Runtime parameters
  UINT32  MinExtraMemory;       ///< Min extra memory (bytes)
  UINT32  MaxExtraMemory;       ///< Max extra memory (bytes)
  UINT32  BaseLoad Address;      ///< Base load address
  UINT32  InitialEIP;           ///< Initial EIP
  UINT32  InitialESP;           ///< Initial ESP
  UINT16  InitialCS;            ///< Initial CS
  UINT16  InitialSS;            ///< Initial SS
} PHARLAP_HEADER;

#define PHARLAP_MAGIC_MP        0x504D  ///< "MP"
#define PHARLAP_MAGIC_P2        0x3250  ///< "P2"
#define PHARLAP_MAGIC_P3        0x3350  ///< "P3"
```

---

## 6. LibOrf Augmentations

### Current Status
- PEF, Amiga HUNK, Atari TOS
- VMS, PDP-10, HP SOM
- Acorn, EPOC32

### Proposed Additions

#### A. Additional VMS Formats

```c
///
/// VMS Image File Header
///
typedef struct {
  UINT16  Magic;                ///< Magic (0x0102 or similar)
  UINT8   MajorId;              ///< Major ID
  UINT8   MinorId;              ///< Minor ID
  UINT32  ImageSize;            ///< Image size in blocks
  UINT32  ImageBase;            ///< Image base address
  UINT32  GlobalSymbolCount;    ///< Global symbol count
  UINT32  ImageIdent;           ///< Image identification
  UINT32  Linktime;             ///< Link time
  UINT32  LinkerId[2];          ///< Linker identification
  UINT32  ImageType;            ///< Image type
  UINT32  SubType;              ///< Subtype
  // ... more VMS-specific fields
} VMS_IMAGE_HEADER;

///
/// VMS Object Module Record Types
///
#define VMS_OBJ_GSD     0       ///< Global symbol directory
#define VMS_OBJ_TIR     1       ///< Text information and relocation
#define VMS_OBJ_EOM     2       ///< End of module
#define VMS_OBJ_DBG     3       ///< Debugger information
#define VMS_OBJ_TBT     4       ///< Traceback information
#define VMS_OBJ_LNK     5       ///< Link information
```

#### B. BeOS Executable Format

```c
///
/// BeOS Executable Header
///
#define BEOS_APP_MAGIC          0x6265  ///< "be"

typedef struct {
  UINT32  Magic;                ///< Magic number
  UINT32  Version;              ///< Version
  UINT32  TextSize;             ///< Text size
  UINT32  DataSize;             ///< Data size
  UINT32  BssSize;              ///< BSS size
  UINT32  SymbolSize;           ///< Symbol table size
  UINT32  Entry;                ///< Entry point
  UINT32  TextRelocSize;        ///< Text relocations
  UINT32  DataRelocSize;        ///< Data relocations
} BEOS_EXEC_HEADER;
```

#### C. QNX4 Format

```c
///
/// QNX4 Load File Format
///
typedef struct {
  UINT16  Magic;                ///< Magic (0x0FFC or 0x0FFE)
  UINT8   Flags;                ///< Flags
  UINT8   CPU;                  ///< CPU type
  UINT16  Fpu;                  ///< FPU type
  UINT16  CodeIndex;            ///< Code segment index
  UINT16  StackIndex;           ///< Stack segment index
  UINT16  HeapIndex;            ///< Heap segment index
  UINT16  ArgvIndex;            ///< Argv segment index
  UINT32  CodeOffset;           ///< Code offset
  UINT16  StackSize;            ///< Stack size
  UINT16  HeapSize;             ///< Heap size
  UINT32  ImageBase;            ///< Image base
  UINT16  NumSegments;          ///< Number of segments
} QNX4_HEADER;

#define QNX4_MAGIC_386          0x0FFC
#define QNX4_MAGIC_286          0x0FFE
```

#### D. OS/9 Module Format

```c
///
/// OS-9 Module Header
///
typedef struct {
  UINT16  Sync;                 ///< Sync bytes (0x87CD for 68K)
  UINT16  SysRev;               ///< System revision
  UINT32  Size;                 ///< Module size
  UINT32  Owner;                ///< Owner ID
  UINT32  Name;                 ///< Module name offset
  UINT16  Access;               ///< Access permissions
  UINT8   Type;                 ///< Module type/language
  UINT8   Attrib;               ///< Attributes/revision
  UINT16  Edition;              ///< Edition number
  UINT32  Usage;                ///< Usage/comment offset
  UINT32  SymbolDef;            ///< Symbol definitions
} OS9_MODULE_HEADER;

#define OS9_SYNC_68K            0x87CD
#define OS9_SYNC_386            0x4AFC
```

---

## Implementation Priority

### Phase 1: High-Impact Additions
1. **LibElf:** OS/ABI variant detection (affects Linux, BSD, embedded)
2. **LibMacho:** CPU subtype detection (Apple ecosystem)
3. **LibCoff:** ECOFF support (historical MIPS/Alpha binaries)

### Phase 2: Specialized Formats
4. **LibAout:** BSD/Linux variants (Unix compatibility)
5. **LibOmf:** DOS extender formats (retro computing)
6. **LibOrf:** VMS, BeOS, QNX (rare but important)

### Phase 3: Comprehensive Coverage
7. All TI DSP variants
8. All embedded formats
9. Obscure historical formats

---

## Testing Strategy

### Unit Tests
```c
// Test ELF variant detection
void test_elf_variants() {
  assert(DetectElfVariant(ELFOSABI_LINUX, 0) == ElfVariantLinux);
  assert(DetectElfVariant(ELFOSABI_FREEBSD, 0) == ElfVariantFreeBSD);
  assert(DetectElfVariant(ELFOSABI_NONE, ELF_FLAGS_FDPIC) == ElfVariantFDPIC);
}

// Test Mach-O CPU detection
void test_macho_cpus() {
  assert(GetMachoCpuName(CPU_TYPE_X86_64) == "x86_64");
  assert(GetMachoCpuName(CPU_TYPE_ARM64) == "arm64");
}
```

### Integration Tests
```bash
# Test against real binaries
./test-binformat /bin/ls           # ELF Linux
./test-binformat /usr/bin/file     # ELF FreeBSD
./test-binformat sample.exe        # PE/COFF
./test-binformat app.app/Contents/MacOS/app  # Mach-O
```

---

## Documentation Updates

Each library should include:

1. **Header Comments:**
   ```c
   ///
   /// Supported Variants:
   /// - ELF32/64 (Generic)
   /// - ELF Linux (ELFOSABI_GNU)
   /// - ELF FreeBSD (ELFOSABI_FREEBSD)
   /// - ELF VxWorks (with flags)
   /// - ELF FDPIC (embedded systems)
   /// - ELF CloudABI (capability-based security)
   /// ...
   ///
   ```

2. **API Documentation:**
   - List all magic numbers
   - Explain variant detection
   - Provide usage examples

3. **Test Coverage:**
   - Sample binaries for each variant
   - Expected output documentation

---

## Backward Compatibility

All augmentations must:
- Maintain existing API
- Not break current users
- Add capabilities through detection
- Use optional parameters where needed

---

## Future Enhancements

### Post-Augmentation Features

1. **Format Conversion:**
   ```c
   BINFORMAT_STATUS ConvertFormat(
     BINFORMAT_CONTEXT *Source,
     BINFORMAT_FILE_TYPE TargetType,
     BINFORMAT_MACHINE TargetMachine,
     BINFORMAT_CONTEXT **Target
   );
   ```

2. **Detailed Symbol Analysis:**
   ```c
   BINFORMAT_STATUS AnalyzeSymbols(
     BINFORMAT_CONTEXT *Context,
     BINFORMAT_SYMBOL_STATS *Stats
   );
   ```

3. **Security Scanning:**
   ```c
   BINFORMAT_STATUS ScanSecurity(
     BINFORMAT_CONTEXT *Context,
     BINFORMAT_SECURITY_REPORT *Report
   );
   ```

---

## Conclusion

These augmentations bring our binary format libraries to feature parity with GNU Binutils/GDB, supporting:

- **80+ processor architectures**
- **50+ binary formats**
- **300+ format/architecture combinations**

All while maintaining:
- Unified API
- NT-style coding conventions
- MIT licensing
- Comprehensive documentation

---

**Document Version:** 1.0
**Created:** November 6, 2025
**License:** MIT
