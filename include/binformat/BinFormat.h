/** @file
  Common Binary Format Library API.

  This header defines the unified API implemented by all binary format
  libraries (libelf, libcoff, libaout, libmacho). All libraries provide
  identical capabilities through a common interface for parsing, creating,
  and manipulating binary executables and object files.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __BINFORMAT_H__
#define __BINFORMAT_H__

#include "../MmixTypes.h"

///
/// Maximum lengths for strings and identifiers
///
#define BINFORMAT_MAX_SECTION_NAME      256
#define BINFORMAT_MAX_SYMBOL_NAME       1024
#define BINFORMAT_MAX_RELOC_NAME        64
#define BINFORMAT_MAX_ARCHITECTURES     32

///
/// Binary format status codes
///
typedef UINT64 BINFORMAT_STATUS;

#define BINFORMAT_SUCCESS                    0x0000000000000000ULL
#define BINFORMAT_ERROR_INVALID_PARAMETER    0x8000000000000001ULL
#define BINFORMAT_ERROR_OUT_OF_MEMORY        0x8000000000000002ULL
#define BINFORMAT_ERROR_INVALID_FORMAT       0x8000000000000003ULL
#define BINFORMAT_ERROR_UNSUPPORTED          0x8000000000000004ULL
#define BINFORMAT_ERROR_NOT_FOUND            0x8000000000000005ULL
#define BINFORMAT_ERROR_BUFFER_TOO_SMALL     0x8000000000000006ULL
#define BINFORMAT_ERROR_IO                   0x8000000000000007ULL
#define BINFORMAT_ERROR_CORRUPTED            0x8000000000000008ULL
#define BINFORMAT_ERROR_NOT_IMPLEMENTED      0x8000000000000009ULL

#define BINFORMAT_IS_ERROR(Status)  ((Status) & 0x8000000000000000ULL)

///
/// Binary file types
///
typedef enum {
  BinFileTypeUnknown = 0,       ///< Unknown or invalid file type
  BinFileTypeRelocatable = 1,   ///< Relocatable object file (.o)
  BinFileTypeExecutable = 2,    ///< Executable file
  BinFileTypeSharedLibrary = 3, ///< Shared library (.so, .dll, .dylib)
  BinFileTypeCore = 4,          ///< Core dump file
  BinFileTypeStaticLibrary = 5  ///< Static library archive (.a)
} BINFORMAT_FILE_TYPE;

///
/// Machine/architecture types
///
typedef enum {
  BinMachineUnknown = 0,
  BinMachineMMIX = 1,
  BinMachineX86 = 2,
  BinMachineX64 = 3,
  BinMachineARM = 4,
  BinMachineARM64 = 5,
  BinMachinePowerPC = 6,
  BinMachinePowerPC64 = 7,
  BinMachineMIPS = 8,
  BinMachineMIPS64 = 9,
  BinMachineSPARC = 10,
  BinMachineSPARC64 = 11,
  BinMachineRISCV32 = 12,
  BinMachineRISCV64 = 13,
  BinMachineIA64 = 14,
  BinMachineAlpha = 15,
  BinMachineM68K = 16,
  BinMachineVAX = 17,
  BinMachinePDP11 = 18,
  BinMachineS390 = 19,
  BinMachineS390X = 20,
  // Historical and specialized architectures
  BinMachineWE32K = 21,         ///< AT&T WE 32000
  BinMachineNS32K = 22,         ///< National Semiconductor 32000
  BinMachineTahoe = 23,         ///< Tahoe (variant of VAX)
  BinMachinePDP10 = 24,         ///< DEC PDP-10
  BinMachineM88K = 25,          ///< Motorola 88000
  BinMachineDLX = 26,           ///< DLX (教學用架構)
  BinMachineMoxie = 27,         ///< Moxie processor
  BinMachineLoongArch32 = 28,   ///< LoongArch 32-bit
  BinMachineLoongArch64 = 29,   ///< LoongArch 64-bit
  BinMachineConvex = 30,        ///< Convex Computer
  BinMachinePyramid = 31,       ///< Pyramid Technology
  BinMachineCray = 32,          ///< Cray vector supercomputers
  BinMachineHPFocus = 33,       ///< HP Focus
  BinMachineEBC = 34,           ///< EFI Byte Code
  BinMachineI8086 = 35,         ///< Intel 8086/8088
  BinMachineI80286 = 36,        ///< Intel 80286 (with Weitek FPU)
  BinMachineAM29K = 37,         ///< AMD 29000
  BinMachineAVR = 38,           ///< Atmel AVR 8-bit
  BinMachineAVR32 = 39,         ///< Atmel AVR 32-bit
  BinMachineNios2 = 40,         ///< Altera Nios II
  BinMachineMicroBlaze = 41,    ///< Xilinx MicroBlaze
  BinMachineOpenRISC = 42,      ///< OpenRISC
  BinMachineMSP430 = 43,        ///< TI MSP430
  BinMachineLanai = 44,         ///< Google Lanai
  BinMachineElbrus2K = 45,      ///< Elbrus 2000 (e2k)
  BinMachineClipper = 46,       ///< Intergraph Clipper
  BinMachineBPF = 47,           ///< Berkeley Packet Filter
  BinMachineHexagon = 48,       ///< Qualcomm Hexagon
  BinMachineCSKY = 49,          ///< C-SKY
  BinMachineFR30 = 50,          ///< Fujitsu FR30
  BinMachineMN10200 = 51,       ///< Matsushita MN10200
  BinMachineMN10300 = 52,       ///< Matsushita MN10300
  BinMachineFRV = 53,           ///< Fujitsu FR-V
  BinMachineNECVE = 54,         ///< NEC Vector Engine (SX-Aurora)
  // Additional embedded and specialized architectures
  BinMachineIP2K = 55,          ///< Ubicom IP2000 series
  BinMachineIQ2000 = 56,        ///< Vitesse IQ2000
  BinMachineCRIS = 57,          ///< Axis CRIS
  BinMachineARC = 58,           ///< ARC Cores
  BinMachineSuperH = 59,        ///< Hitachi SuperH
  BinMachinePARISC = 60,        ///< HP PA-RISC
  BinMachineCR16 = 61,          ///< National Semiconductor CR16
  BinMachineD10V = 62,          ///< Mitsubishi D10V
  BinMachineD30V = 63,          ///< Mitsubishi D30V
  BinMachineXtensa = 64,        ///< Tensilica Xtensa
  BinMachineI860 = 65,          ///< Intel i860
  BinMachineI960 = 66,          ///< Intel i960
  BinMachineTIC54X = 67,        ///< TI TMS320C54x DSP
  BinMachineTIC55X = 68,        ///< TI TMS320C55x DSP
  BinMachineTIC6X = 69,         ///< TI TMS320C6x DSP
  BinMachineBlackfin = 70,      ///< Analog Devices Blackfin
  BinMachineEpiphany = 71,      ///< Adapteva Epiphany
  BinMachineM32R = 72,          ///< Mitsubishi M32R
  BinMachineM32C = 73,          ///< Renesas M32C
  BinMachineLM32 = 74,          ///< Lattice Mico32
  BinMachineNDS32 = 75,         ///< Andes NDS32
  BinMachineV850 = 76,          ///< NEC V850
  BinMachineTILE = 77,          ///< Tilera TILE series
  BinMachineZ80 = 78,           ///< Zilog Z80
  BinMachineZ8000 = 79,         ///< Zilog Z8000
  BinMachineZ80000 = 80,        ///< Zilog Z80000
  BinMachine6502 = 81,          ///< MOS 6502
  BinMachine65816 = 82,         ///< WDC 65816
  BinMachine65832 = 83,         ///< WDC 65832
  BinMachineMCore = 84,         ///< Motorola MCore
  BinMachineTriCore = 85,       ///< Infineon TriCore
  BinMachineMEP = 86,           ///< Toshiba MeP
  BinMachinePicoJava = 87,      ///< Sun picoJava
  BinMachineAM33 = 88,          ///< Matsushita AM33
  BinMachineMetag = 89,         ///< Imagination Technologies Meta
  BinMachineSCORE = 90,         ///< Sunplus S+core
  BinMachineRL78 = 91,          ///< Renesas RL78
  BinMachineRX = 92,            ///< Renesas RX
  BinMachineVisium = 93,        ///< CDS VISium
  BinMachineXGATE = 94,         ///< Freescale XGATE
  BinMachineXStormy16 = 95,     ///< Sanyo XStormy16
  BinMachineKVX = 96,           ///< Kalray VLIW
  BinMachineNFP = 97,           ///< Netronome Flow Processor
  BinMachineMT = 98,            ///< Morpho MT
  BinMachineFT32 = 99,          ///< FTDI FT32
  BinMachineH8200 = 100,        ///< Hitachi H8/200
  BinMachineH8300 = 101,        ///< Hitachi H8/300
  BinMachineSPU = 102,          ///< Cell Broadband Engine SPU
  BinMachineWASM = 103,         ///< WebAssembly
  BinMachinePRU = 104           ///< TI Programmable Realtime Unit
} BINFORMAT_MACHINE;

///
/// Endianness
///
typedef enum {
  BinEndianLittle = 0,
  BinEndianBig = 1
} BINFORMAT_ENDIAN;

///
/// Section flags
///
#define BINFORMAT_SECTION_FLAG_NONE         0x00000000
#define BINFORMAT_SECTION_FLAG_WRITE        0x00000001  ///< Section is writable
#define BINFORMAT_SECTION_FLAG_ALLOC        0x00000002  ///< Section occupies memory
#define BINFORMAT_SECTION_FLAG_EXEC         0x00000004  ///< Section is executable
#define BINFORMAT_SECTION_FLAG_MERGE        0x00000010  ///< Data can be merged
#define BINFORMAT_SECTION_FLAG_STRINGS      0x00000020  ///< Contains null-terminated strings
#define BINFORMAT_SECTION_FLAG_INFO_LINK    0x00000040  ///< sh_info contains section index
#define BINFORMAT_SECTION_FLAG_LINK_ORDER   0x00000080  ///< Preserve order after combining
#define BINFORMAT_SECTION_FLAG_TLS          0x00000400  ///< Thread-local storage

///
/// Section types
///
typedef enum {
  BinSectionTypeNull = 0,       ///< Inactive section
  BinSectionTypeProgBits = 1,   ///< Program data
  BinSectionTypeSymTab = 2,     ///< Symbol table
  BinSectionTypeStrTab = 3,     ///< String table
  BinSectionTypeRela = 4,       ///< Relocation entries with addends
  BinSectionTypeHash = 5,       ///< Symbol hash table
  BinSectionTypeDynamic = 6,    ///< Dynamic linking information
  BinSectionTypeNote = 7,       ///< Notes
  BinSectionTypeNoBits = 8,     ///< BSS (no file space)
  BinSectionTypeRel = 9,        ///< Relocation entries
  BinSectionTypeDynSym = 11,    ///< Dynamic linker symbol table
  BinSectionTypeInitArray = 14, ///< Array of constructors
  BinSectionTypeFiniArray = 15, ///< Array of destructors
  BinSectionTypeDebug = 16      ///< Debug information
} BINFORMAT_SECTION_TYPE;

///
/// Symbol binding
///
typedef enum {
  BinSymbolBindLocal = 0,   ///< Local symbol
  BinSymbolBindGlobal = 1,  ///< Global symbol
  BinSymbolBindWeak = 2     ///< Weak symbol
} BINFORMAT_SYMBOL_BIND;

///
/// Symbol types
///
typedef enum {
  BinSymbolTypeNone = 0,     ///< No type
  BinSymbolTypeObject = 1,   ///< Data object
  BinSymbolTypeFunc = 2,     ///< Function
  BinSymbolTypeSection = 3,  ///< Section
  BinSymbolTypeFile = 4,     ///< Source file name
  BinSymbolTypeCommon = 5,   ///< Common data object
  BinSymbolTypeTls = 6       ///< Thread-local data object
} BINFORMAT_SYMBOL_TYPE;

///
/// Relocation types (generic, normalized across formats)
///
typedef enum {
  BinRelocNone = 0,
  BinRelocAbsolute32 = 1,
  BinRelocAbsolute64 = 2,
  BinRelocRelative32 = 3,
  BinRelocRelative64 = 4,
  BinRelocPCRelative32 = 5,
  BinRelocPCRelative64 = 6,
  BinRelocGOT32 = 7,
  BinRelocGOT64 = 8,
  BinRelocPLT32 = 9,
  BinRelocPLT64 = 10,
  BinRelocCopy = 11,
  BinRelocGlobDat = 12,
  BinRelocJumpSlot = 13,
  BinRelocRelative = 14,
  BinRelocTlsDescriptor = 15,
  BinRelocTlsOffset = 16
} BINFORMAT_RELOC_TYPE;

///
/// Section descriptor
///
typedef struct {
  CHAR8   Name[BINFORMAT_MAX_SECTION_NAME];  ///< Section name
  UINT32  Type;                               ///< Section type
  UINT64  Flags;                              ///< Section flags
  UINT64  VirtualAddress;                     ///< Virtual address in memory
  UINT64  FileOffset;                         ///< Offset in file
  UINT64  Size;                               ///< Section size in bytes
  UINT32  Link;                               ///< Link to another section
  UINT32  Info;                               ///< Additional section information
  UINT64  Alignment;                          ///< Section alignment
  UINT64  EntrySize;                          ///< Entry size if section holds table
  VOID    *Data;                              ///< Pointer to section data
} BINFORMAT_SECTION;

///
/// Symbol descriptor
///
typedef struct {
  CHAR8   Name[BINFORMAT_MAX_SYMBOL_NAME];  ///< Symbol name
  UINT64  Value;                             ///< Symbol value
  UINT64  Size;                              ///< Symbol size
  UINT8   Bind;                              ///< Symbol binding (local/global/weak)
  UINT8   Type;                              ///< Symbol type
  UINT16  SectionIndex;                      ///< Associated section index
  UINT8   Other;                             ///< Symbol visibility
} BINFORMAT_SYMBOL;

///
/// Relocation entry descriptor
///
typedef struct {
  UINT64  Offset;       ///< Offset where to apply relocation
  UINT32  Type;         ///< Relocation type
  UINT32  SymbolIndex;  ///< Symbol table index
  INT64   Addend;       ///< Addend for relocation
} BINFORMAT_RELOCATION;

///
/// Program/segment descriptor
///
typedef struct {
  UINT32  Type;              ///< Segment type
  UINT32  Flags;             ///< Segment flags
  UINT64  FileOffset;        ///< Offset in file
  UINT64  VirtualAddress;    ///< Virtual address
  UINT64  PhysicalAddress;   ///< Physical address
  UINT64  FileSize;          ///< Size in file
  UINT64  MemorySize;        ///< Size in memory
  UINT64  Alignment;         ///< Alignment
} BINFORMAT_SEGMENT;

///
/// Architecture descriptor (for fat/universal binaries)
///
typedef struct {
  BINFORMAT_MACHINE  Machine;       ///< Machine type
  UINT32             CpuSubtype;    ///< CPU subtype (variant)
  UINT64             Offset;        ///< Offset in file
  UINT64             Size;          ///< Size of binary
  UINT32             Alignment;     ///< Alignment
} BINFORMAT_ARCHITECTURE;

///
/// Code signature types
///
typedef enum {
  BinSignatureTypeNone = 0,          ///< No signature
  BinSignatureTypeAdHoc = 1,          ///< Ad-hoc signature (no certificate)
  BinSignatureTypeDeveloper = 2,      ///< Developer signature
  BinSignatureTypeAppStore = 3,       ///< App Store signature
  BinSignatureTypePGP = 4,            ///< PGP signature
  BinSignatureTypeAuthenticode = 5,   ///< Windows Authenticode
  BinSignatureTypePKCS7 = 6,          ///< PKCS#7 signature
  BinSignatureTypeX509 = 7            ///< X.509 certificate
} BINFORMAT_SIGNATURE_TYPE;

///
/// Code signature hash algorithm
///
typedef enum {
  BinHashNone = 0,
  BinHashSHA1 = 1,
  BinHashSHA256 = 2,
  BinHashSHA384 = 3,
  BinHashSHA512 = 4,
  BinHashMD5 = 5
} BINFORMAT_HASH_ALGORITHM;

///
/// Code signature descriptor
///
typedef struct {
  BINFORMAT_SIGNATURE_TYPE  Type;              ///< Signature type
  BINFORMAT_HASH_ALGORITHM  HashAlgorithm;     ///< Hash algorithm used
  BOOLEAN                   IsValid;           ///< Signature verification status
  BOOLEAN                   IsSigned;          ///< TRUE if binary is signed
  UINT64                    SignatureOffset;   ///< Offset of signature in file
  UINT64                    SignatureSize;     ///< Size of signature data
  UINT64                    CodeLimit;         ///< Signed code range end
  CHAR8                     SignerName[256];   ///< Signer/certificate name
  CHAR8                     TeamID[64];        ///< Team ID (Apple)
  CHAR8                     BundleID[256];     ///< Bundle identifier
  UINT32                    Flags;             ///< Signature flags
  UINT64                    Timestamp;         ///< Signing timestamp
  VOID                      *RawData;          ///< Raw signature data
} BINFORMAT_CODE_SIGNATURE;

///
/// String encoding types
///
typedef enum {
  BinStringEncodingASCII = 0,      ///< 7-bit ASCII
  BinStringEncodingUTF8 = 1,       ///< UTF-8
  BinStringEncodingUTF16LE = 2,    ///< UTF-16 Little Endian
  BinStringEncodingUTF16BE = 3,    ///< UTF-16 Big Endian
  BinStringEncodingUTF32LE = 4,    ///< UTF-32 Little Endian
  BinStringEncodingUTF32BE = 5,    ///< UTF-32 Big Endian
  BinStringEncodingISO8859_1 = 6,  ///< ISO-8859-1 (Latin-1)
  BinStringEncodingShiftJIS = 7,   ///< Shift-JIS (Japanese)
  BinStringEncodingEUCJP = 8,      ///< EUC-JP (Japanese)
  BinStringEncodingGBK = 9         ///< GBK (Chinese)
} BINFORMAT_STRING_ENCODING;

///
/// Binary file context (opaque handle)
///
typedef struct _BINFORMAT_CONTEXT BINFORMAT_CONTEXT;

///
/// Iterator types (opaque handles for safe iteration)
///
typedef struct _BINFORMAT_SECTION_ITERATOR    BINFORMAT_SECTION_ITERATOR;
typedef struct _BINFORMAT_SYMBOL_ITERATOR     BINFORMAT_SYMBOL_ITERATOR;
typedef struct _BINFORMAT_SEGMENT_ITERATOR    BINFORMAT_SEGMENT_ITERATOR;
typedef struct _BINFORMAT_RELOCATION_ITERATOR BINFORMAT_RELOCATION_ITERATOR;
typedef struct _BINFORMAT_ARCH_ITERATOR       BINFORMAT_ARCH_ITERATOR;

///
/// Binary stream flags (can be combined with bitwise OR)
///
#define BINFORMAT_STREAM_READ        0x0001  ///< Read access
#define BINFORMAT_STREAM_WRITE       0x0002  ///< Write access
#define BINFORMAT_STREAM_MMAP        0x0010  ///< Use memory mapping
#define BINFORMAT_STREAM_STREAMING   0x0020  ///< Streaming mode (no full buffer, for memory-constrained systems)
#define BINFORMAT_STREAM_BUFFER      0x0040  ///< External buffer (caller-owned, not freed on close)
#define BINFORMAT_STREAM_ALLOCATED   0x0080  ///< Allocated buffer (growable, owned by stream)

///
/// Binary stream structure for input/output operations
///
/// This structure provides a unified abstraction for different I/O methods,
/// allowing backends to avoid code duplication and support:
/// - Regular file I/O (loading entire file into memory)
/// - Memory-mapped files (for large files with virtual memory)
/// - Existing buffers (for in-memory processing)
/// - Allocated buffers (for creating new binary images)
/// - Streaming I/O (for memory-constrained embedded systems)
///
/// The Flags field determines the behavior:
/// - BINFORMAT_STREAM_READ: Read-only access
/// - BINFORMAT_STREAM_READ | BINFORMAT_STREAM_WRITE: Read-write access
/// - BINFORMAT_STREAM_MMAP: Use memory mapping
/// - BINFORMAT_STREAM_STREAMING: Use streaming I/O (read/write on demand)
/// - BINFORMAT_STREAM_BUFFER: Use external buffer (caller owns Data pointer)
/// - BINFORMAT_STREAM_ALLOCATED: Use allocated buffer (stream owns Data pointer)
///
typedef struct {
  UINT32   Flags;            ///< Stream flags (combination of BINFORMAT_STREAM_*)
  CHAR8    *FilePath;        ///< File path (if applicable)
  UINT8    *Data;            ///< Buffer data pointer (NULL for streaming mode)
  UINT64   Size;             ///< Current data size
  UINT64   Capacity;         ///< Allocated capacity (for resizable buffers)
  INT32    FileDescriptor;   ///< File descriptor (for mmap/streaming, -1 if not used)
  VOID     *MmapBase;        ///< Base address for munmap (mmap only, NULL if not used)
  UINT64   FilePosition;     ///< Current file position (streaming mode)
} BINFORMAT_STREAM;

/**
  Initialize stream from file path.

  @param[out]  Stream            Pointer to stream structure.
  @param[in]   FilePath          Path to file.
  @param[in]   Flags             Stream flags (BINFORMAT_STREAM_*).
                                 Common combinations:
                                 - BINFORMAT_STREAM_READ: Read-only, load entire file
                                 - BINFORMAT_STREAM_READ | BINFORMAT_STREAM_MMAP: Read-only, memory-mapped
                                 - BINFORMAT_STREAM_READ | BINFORMAT_STREAM_STREAMING: Read-only, streaming I/O
                                 - BINFORMAT_STREAM_READ | BINFORMAT_STREAM_WRITE: Read-write, load entire file

  @retval BINFORMAT_SUCCESS      Stream initialized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatStreamInitFile(
  OUT BINFORMAT_STREAM  *Stream,
  IN  CONST CHAR8       *FilePath,
  IN  UINT32            Flags
  );

/**
  Initialize stream from existing buffer.

  @param[out]  Stream            Pointer to stream structure.
  @param[in]   Buffer            Pointer to buffer data.
  @param[in]   Size              Size of buffer.
  @param[in]   Flags             Stream flags (must include BINFORMAT_STREAM_BUFFER,
                                 optionally BINFORMAT_STREAM_READ and/or BINFORMAT_STREAM_WRITE).

  @retval BINFORMAT_SUCCESS      Stream initialized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatStreamInitBuffer(
  OUT BINFORMAT_STREAM  *Stream,
  IN  CONST VOID        *Buffer,
  IN  UINT64            Size,
  IN  UINT32            Flags
  );

/**
  Initialize stream with allocated buffer (for creating new binary images).

  @param[out]  Stream            Pointer to stream structure.
  @param[in]   InitialSize       Initial buffer size (0 for default).

  @retval BINFORMAT_SUCCESS      Stream initialized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatStreamInitAllocated(
  OUT BINFORMAT_STREAM  *Stream,
  IN  UINT64            InitialSize
  );

/**
  Read data from stream at specified offset.

  Works with all stream types. For buffered streams, this is a memory copy.
  For streaming types, this performs a file seek and read operation.

  @param[in]  Stream             Stream structure.
  @param[in]  Offset             Offset to read from.
  @param[out] Buffer             Buffer to read into.
  @param[in]  Size               Number of bytes to read.

  @retval BINFORMAT_SUCCESS      Data read successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatStreamRead(
  IN  BINFORMAT_STREAM  *Stream,
  IN  UINT64            Offset,
  OUT VOID              *Buffer,
  IN  UINT64            Size
  );

/**
  Write data to stream at specified offset.

  Works with writable streams. For buffered types, this is a memory copy.
  For streaming types, this performs a file seek and write operation.
  For allocated types, automatically resizes the buffer if needed.

  @param[in]  Stream             Stream structure.
  @param[in]  Offset             Offset to write to.
  @param[in]  Buffer             Buffer to write from.
  @param[in]  Size               Number of bytes to write.

  @retval BINFORMAT_SUCCESS      Data written successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatStreamWrite(
  IN  BINFORMAT_STREAM  *Stream,
  IN  UINT64            Offset,
  IN  CONST VOID        *Buffer,
  IN  UINT64            Size
  );

/**
  Resize an allocated buffer.

  Only works with BINFORMAT_STREAM_ALLOCATED streams.

  @param[in]  Stream             Stream structure.
  @param[in]  NewSize            New buffer size.

  @retval BINFORMAT_SUCCESS      Buffer resized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatStreamResize(
  IN  BINFORMAT_STREAM  *Stream,
  IN  UINT64            NewSize
  );

/**
  Flush any pending writes to disk.

  For streaming mode, ensures all buffered writes are written to disk.
  For memory-mapped files, performs msync().
  For other modes, this is a no-op.

  @param[in]  Stream             Stream structure.

  @retval BINFORMAT_SUCCESS      Data flushed successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatStreamFlush(
  IN  BINFORMAT_STREAM  *Stream
  );

/**
  Get direct pointer to data (if available).

  For buffered streams, returns pointer to data.
  For streaming types, returns NULL (must use BinFormatStreamRead/Write).

  @param[in]  Stream             Stream structure.

  @return Pointer to data, or NULL if not available.
**/
VOID *
BinFormatStreamGetDataPointer(
  IN  BINFORMAT_STREAM  *Stream
  );

/**
  Close and free stream resources.

  Closes files, unmaps memory, frees buffers as appropriate.

  @param[in]  Stream             Stream structure.
**/
VOID
BinFormatStreamClose(
  IN  BINFORMAT_STREAM  *Stream
  );

///
/// File header information
///
typedef struct {
  BINFORMAT_FILE_TYPE  FileType;              ///< Type of binary file
  BINFORMAT_MACHINE    Machine;               ///< Target machine/architecture
  BINFORMAT_ENDIAN     Endianness;            ///< Data endianness
  UINT32               Version;               ///< Format version
  UINT64               EntryPoint;            ///< Entry point address
  UINT32               Flags;                 ///< Architecture-specific flags
  BOOLEAN              Is64Bit;               ///< TRUE if 64-bit format
  UINT32               SectionCount;          ///< Number of sections
  UINT32               SegmentCount;          ///< Number of segments
  UINT32               SymbolCount;           ///< Number of symbols

  ///
  /// Multi-architecture support (Fat/Universal binaries)
  ///
  UINT32               ArchitectureCount;     ///< Number of architectures
  BINFORMAT_ARCHITECTURE Architectures[BINFORMAT_MAX_ARCHITECTURES];
} BINFORMAT_HEADER_INFO;

/**
  Initialize a binary format context from a file.

  @param[out]  Context           Pointer to receive context handle.
  @param[in]   FilePath          Path to binary file.
  @param[in]   ReadOnly          TRUE for read-only access.

  @retval BINFORMAT_SUCCESS      Context initialized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_INIT_FILE)(
  OUT BINFORMAT_CONTEXT  **Context,
  IN  CONST CHAR8        *FilePath,
  IN  BOOLEAN            ReadOnly
  );

/**
  Initialize a binary format context from memory buffer.

  @param[out]  Context           Pointer to receive context handle.
  @param[in]   Buffer            Pointer to binary data.
  @param[in]   Size              Size of binary data.

  @retval BINFORMAT_SUCCESS      Context initialized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_INIT_MEMORY)(
  OUT BINFORMAT_CONTEXT  **Context,
  IN  CONST VOID         *Buffer,
  IN  UINT64             Size
  );

/**
  Create a new binary format context for writing.

  @param[out]  Context           Pointer to receive context handle.
  @param[in]   FileType          Type of binary to create.
  @param[in]   Machine           Target machine.
  @param[in]   Is64Bit           TRUE for 64-bit format.

  @retval BINFORMAT_SUCCESS      Context created successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_CREATE)(
  OUT BINFORMAT_CONTEXT   **Context,
  IN  BINFORMAT_FILE_TYPE FileType,
  IN  BINFORMAT_MACHINE   Machine,
  IN  BOOLEAN             Is64Bit
  );

/**
  Close and free a binary format context.

  @param[in]  Context            Context to close.

**/
typedef
VOID
(*BINFORMAT_CLOSE)(
  IN  BINFORMAT_CONTEXT  *Context
  );

/**
  Get header information from binary.

  @param[in]   Context           Binary context.
  @param[out]  HeaderInfo        Pointer to receive header information.

  @retval BINFORMAT_SUCCESS      Header information retrieved.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_GET_HEADER)(
  IN  BINFORMAT_CONTEXT      *Context,
  OUT BINFORMAT_HEADER_INFO  *HeaderInfo
  );

/**
  Get section information by index.

  @param[in]   Context           Binary context.
  @param[in]   Index             Section index.
  @param[out]  Section           Pointer to receive section information.

  @retval BINFORMAT_SUCCESS      Section information retrieved.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_GET_SECTION)(
  IN  BINFORMAT_CONTEXT   *Context,
  IN  UINT32              Index,
  OUT BINFORMAT_SECTION   *Section
  );

/**
  Get section information by name.

  @param[in]   Context           Binary context.
  @param[in]   Name              Section name.
  @param[out]  Section           Pointer to receive section information.

  @retval BINFORMAT_SUCCESS      Section found and retrieved.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_GET_SECTION_BY_NAME)(
  IN  BINFORMAT_CONTEXT   *Context,
  IN  CONST CHAR8         *Name,
  OUT BINFORMAT_SECTION   *Section
  );

/**
  Get segment information by index.

  @param[in]   Context           Binary context.
  @param[in]   Index             Segment index.
  @param[out]  Segment           Pointer to receive segment information.

  @retval BINFORMAT_SUCCESS      Segment information retrieved.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_GET_SEGMENT)(
  IN  BINFORMAT_CONTEXT   *Context,
  IN  UINT32              Index,
  OUT BINFORMAT_SEGMENT   *Segment
  );

/**
  Get symbol information by index.

  @param[in]   Context           Binary context.
  @param[in]   Index             Symbol index.
  @param[out]  Symbol            Pointer to receive symbol information.

  @retval BINFORMAT_SUCCESS      Symbol information retrieved.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_GET_SYMBOL)(
  IN  BINFORMAT_CONTEXT   *Context,
  IN  UINT32              Index,
  OUT BINFORMAT_SYMBOL    *Symbol
  );

/**
  Get symbol information by name.

  @param[in]   Context           Binary context.
  @param[in]   Name              Symbol name.
  @param[out]  Symbol            Pointer to receive symbol information.

  @retval BINFORMAT_SUCCESS      Symbol found and retrieved.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_GET_SYMBOL_BY_NAME)(
  IN  BINFORMAT_CONTEXT   *Context,
  IN  CONST CHAR8         *Name,
  OUT BINFORMAT_SYMBOL    *Symbol
  );

/**
  Add a new section to the binary.

  @param[in]   Context           Binary context.
  @param[in]   Section           Section to add.
  @param[out]  Index             Pointer to receive new section index.

  @retval BINFORMAT_SUCCESS      Section added.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_ADD_SECTION)(
  IN  BINFORMAT_CONTEXT   *Context,
  IN  BINFORMAT_SECTION   *Section,
  OUT UINT32              *Index
  );

/**
  Add a new symbol to the binary.

  @param[in]   Context           Binary context.
  @param[in]   Symbol            Symbol to add.
  @param[out]  Index             Pointer to receive new symbol index.

  @retval BINFORMAT_SUCCESS      Symbol added.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_ADD_SYMBOL)(
  IN  BINFORMAT_CONTEXT   *Context,
  IN  BINFORMAT_SYMBOL    *Symbol,
  OUT UINT32              *Index
  );

/**
  Add a relocation entry.

  @param[in]   Context           Binary context.
  @param[in]   SectionIndex      Target section index.
  @param[in]   Relocation        Relocation to add.

  @retval BINFORMAT_SUCCESS      Relocation added.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_ADD_RELOCATION)(
  IN  BINFORMAT_CONTEXT       *Context,
  IN  UINT32                  SectionIndex,
  IN  BINFORMAT_RELOCATION    *Relocation
  );

/**
  Update an existing section in place.

  @param[in]   Context           Binary context.
  @param[in]   Index             Section index to update.
  @param[in]   Section           New section data.

  @retval BINFORMAT_SUCCESS      Section updated.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_UPDATE_SECTION)(
  IN  BINFORMAT_CONTEXT   *Context,
  IN  UINT32              Index,
  IN  BINFORMAT_SECTION   *Section
  );

/**
  Update an existing symbol in place.

  @param[in]   Context           Binary context.
  @param[in]   Index             Symbol index to update.
  @param[in]   Symbol            New symbol data.

  @retval BINFORMAT_SUCCESS      Symbol updated.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_UPDATE_SYMBOL)(
  IN  BINFORMAT_CONTEXT   *Context,
  IN  UINT32              Index,
  IN  BINFORMAT_SYMBOL    *Symbol
  );

/**
  Update an existing relocation in place.

  @param[in]   Context           Binary context.
  @param[in]   SectionIndex      Section containing the relocation.
  @param[in]   RelocationIndex   Relocation index within section.
  @param[in]   Relocation        New relocation data.

  @retval BINFORMAT_SUCCESS      Relocation updated.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_UPDATE_RELOCATION)(
  IN  BINFORMAT_CONTEXT       *Context,
  IN  UINT32                  SectionIndex,
  IN  UINT32                  RelocationIndex,
  IN  BINFORMAT_RELOCATION    *Relocation
  );

/**
  Write binary to file.

  @param[in]   Context           Binary context.
  @param[in]   FilePath          Output file path.

  @retval BINFORMAT_SUCCESS      Binary written successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_WRITE_FILE)(
  IN  BINFORMAT_CONTEXT   *Context,
  IN  CONST CHAR8         *FilePath
  );

/**
  Write binary to memory buffer.

  @param[in]   Context           Binary context.
  @param[out]  Buffer            Buffer to write to.
  @param[in]   BufferSize        Size of buffer.
  @param[out]  BytesWritten      Pointer to receive bytes written.

  @retval BINFORMAT_SUCCESS      Binary written successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_WRITE_MEMORY)(
  IN  BINFORMAT_CONTEXT   *Context,
  OUT VOID                *Buffer,
  IN  UINT64              BufferSize,
  OUT UINT64              *BytesWritten
  );

/**
  Select architecture in multi-architecture binary.

  @param[in]   Context           Binary context.
  @param[in]   ArchIndex         Architecture index.

  @retval BINFORMAT_SUCCESS      Architecture selected.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_SELECT_ARCHITECTURE)(
  IN  BINFORMAT_CONTEXT   *Context,
  IN  UINT32              ArchIndex
  );

/**
  Create a new fat/universal binary (multi-architecture).

  @param[out]  Context           Pointer to receive fat binary context.
  @param[in]   FileType          File type for the fat binary.
  @param[in]   Format            Binary format (e.g., Mach-O, ELF).

  @retval BINFORMAT_SUCCESS      Fat binary created.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_CREATE_FAT)(
  OUT BINFORMAT_CONTEXT     **Context,
  IN  BINFORMAT_FILE_TYPE   FileType,
  IN  CONST CHAR8           *Format
  );

/**
  Add an architecture slice to a fat binary.

  @param[in]   FatContext        Fat binary context.
  @param[in]   SliceContext      Architecture slice to add.
  @param[out]  ArchIndex         Pointer to receive new architecture index.

  @retval BINFORMAT_SUCCESS      Slice added.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_ADD_ARCH_SLICE)(
  IN  BINFORMAT_CONTEXT   *FatContext,
  IN  BINFORMAT_CONTEXT   *SliceContext,
  OUT UINT32              *ArchIndex
  );

/**
  Remove an architecture slice from a fat binary.

  @param[in]   Context           Fat binary context.
  @param[in]   ArchIndex         Architecture index to remove.

  @retval BINFORMAT_SUCCESS      Slice removed.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_REMOVE_ARCH_SLICE)(
  IN  BINFORMAT_CONTEXT   *Context,
  IN  UINT32              ArchIndex
  );

/**
  Replace an architecture slice in a fat binary.

  @param[in]   FatContext        Fat binary context.
  @param[in]   ArchIndex         Architecture index to replace.
  @param[in]   SliceContext      New architecture slice.

  @retval BINFORMAT_SUCCESS      Slice replaced.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_REPLACE_ARCH_SLICE)(
  IN  BINFORMAT_CONTEXT   *FatContext,
  IN  UINT32              ArchIndex,
  IN  BINFORMAT_CONTEXT   *SliceContext
  );

/**
  Extract a thin (single-architecture) binary from a fat binary.

  @param[in]   FatContext        Fat binary context.
  @param[in]   ArchIndex         Architecture index to extract.
  @param[out]  ThinContext       Pointer to receive thin binary context.

  @retval BINFORMAT_SUCCESS      Thin binary extracted.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_EXTRACT_THIN)(
  IN  BINFORMAT_CONTEXT   *FatContext,
  IN  UINT32              ArchIndex,
  OUT BINFORMAT_CONTEXT   **ThinContext
  );

/**
  Get code signature information.

  @param[in]   Context           Binary context.
  @param[out]  Signature         Pointer to receive signature information.

  @retval BINFORMAT_SUCCESS      Signature information retrieved.
  @retval BINFORMAT_ERROR_NOT_FOUND  Binary is not signed.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_GET_SIGNATURE)(
  IN  BINFORMAT_CONTEXT       *Context,
  OUT BINFORMAT_CODE_SIGNATURE *Signature
  );

/**
  Verify code signature.

  @param[in]   Context           Binary context.
  @param[out]  IsValid           Pointer to receive validation result.

  @retval BINFORMAT_SUCCESS      Verification completed.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_VERIFY_SIGNATURE)(
  IN  BINFORMAT_CONTEXT   *Context,
  OUT BOOLEAN             *IsValid
  );

/**
  Add or update code signature.

  @param[in]   Context           Binary context.
  @param[in]   Signature         Signature data to add.

  @retval BINFORMAT_SUCCESS      Signature added.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_SIGN_BINARY)(
  IN  BINFORMAT_CONTEXT             *Context,
  IN  CONST BINFORMAT_CODE_SIGNATURE *Signature
  );

/**
  Remove code signature from binary.

  @param[in]   Context           Binary context.

  @retval BINFORMAT_SUCCESS      Signature removed.
  @retval BINFORMAT_ERROR_NOT_FOUND  Binary is not signed.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_REMOVE_SIGNATURE)(
  IN  BINFORMAT_CONTEXT   *Context
  );

/**
  Create section iterator.

  @param[in]   Context           Binary context.
  @param[out]  Iterator          Pointer to receive iterator.

  @retval BINFORMAT_SUCCESS      Iterator created.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_SECTION_ITER_CREATE)(
  IN  BINFORMAT_CONTEXT            *Context,
  OUT BINFORMAT_SECTION_ITERATOR   **Iterator
  );

/**
  Get next section from iterator.

  @param[in]   Iterator          Section iterator.
  @param[out]  Section           Pointer to receive section.

  @retval BINFORMAT_SUCCESS      Section retrieved.
  @retval BINFORMAT_ERROR_NOT_FOUND  No more sections.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_SECTION_ITER_NEXT)(
  IN  BINFORMAT_SECTION_ITERATOR  *Iterator,
  OUT BINFORMAT_SECTION           *Section
  );

/**
  Free section iterator.

  @param[in]   Iterator          Section iterator to free.

**/
typedef
VOID
(*BINFORMAT_SECTION_ITER_FREE)(
  IN  BINFORMAT_SECTION_ITERATOR  *Iterator
  );

/**
  Create symbol iterator.

  @param[in]   Context           Binary context.
  @param[out]  Iterator          Pointer to receive iterator.

  @retval BINFORMAT_SUCCESS      Iterator created.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_SYMBOL_ITER_CREATE)(
  IN  BINFORMAT_CONTEXT           *Context,
  OUT BINFORMAT_SYMBOL_ITERATOR   **Iterator
  );

/**
  Get next symbol from iterator.

  @param[in]   Iterator          Symbol iterator.
  @param[out]  Symbol            Pointer to receive symbol.

  @retval BINFORMAT_SUCCESS      Symbol retrieved.
  @retval BINFORMAT_ERROR_NOT_FOUND  No more symbols.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_SYMBOL_ITER_NEXT)(
  IN  BINFORMAT_SYMBOL_ITERATOR  *Iterator,
  OUT BINFORMAT_SYMBOL           *Symbol
  );

/**
  Free symbol iterator.

  @param[in]   Iterator          Symbol iterator to free.

**/
typedef
VOID
(*BINFORMAT_SYMBOL_ITER_FREE)(
  IN  BINFORMAT_SYMBOL_ITERATOR  *Iterator
  );

/**
  Create segment iterator.

  @param[in]   Context           Binary context.
  @param[out]  Iterator          Pointer to receive iterator.

  @retval BINFORMAT_SUCCESS      Iterator created.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_SEGMENT_ITER_CREATE)(
  IN  BINFORMAT_CONTEXT            *Context,
  OUT BINFORMAT_SEGMENT_ITERATOR   **Iterator
  );

/**
  Get next segment from iterator.

  @param[in]   Iterator          Segment iterator.
  @param[out]  Segment           Pointer to receive segment.

  @retval BINFORMAT_SUCCESS      Segment retrieved.
  @retval BINFORMAT_ERROR_NOT_FOUND  No more segments.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_SEGMENT_ITER_NEXT)(
  IN  BINFORMAT_SEGMENT_ITERATOR  *Iterator,
  OUT BINFORMAT_SEGMENT           *Segment
  );

/**
  Free segment iterator.

  @param[in]   Iterator          Segment iterator to free.

**/
typedef
VOID
(*BINFORMAT_SEGMENT_ITER_FREE)(
  IN  BINFORMAT_SEGMENT_ITERATOR  *Iterator
  );

/**
  Create relocation iterator for a section.

  @param[in]   Context           Binary context.
  @param[in]   SectionIndex      Section index.
  @param[out]  Iterator          Pointer to receive iterator.

  @retval BINFORMAT_SUCCESS      Iterator created.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_RELOCATION_ITER_CREATE)(
  IN  BINFORMAT_CONTEXT               *Context,
  IN  UINT32                          SectionIndex,
  OUT BINFORMAT_RELOCATION_ITERATOR   **Iterator
  );

/**
  Get next relocation from iterator.

  @param[in]   Iterator          Relocation iterator.
  @param[out]  Relocation        Pointer to receive relocation.

  @retval BINFORMAT_SUCCESS      Relocation retrieved.
  @retval BINFORMAT_ERROR_NOT_FOUND  No more relocations.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_RELOCATION_ITER_NEXT)(
  IN  BINFORMAT_RELOCATION_ITERATOR  *Iterator,
  OUT BINFORMAT_RELOCATION           *Relocation
  );

/**
  Free relocation iterator.

  @param[in]   Iterator          Relocation iterator to free.

**/
typedef
VOID
(*BINFORMAT_RELOCATION_ITER_FREE)(
  IN  BINFORMAT_RELOCATION_ITERATOR  *Iterator
  );

/**
  Create architecture iterator for fat/universal binaries.

  @param[in]   Context           Binary context.
  @param[out]  Iterator          Pointer to receive iterator.

  @retval BINFORMAT_SUCCESS      Iterator created.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_ARCH_ITER_CREATE)(
  IN  BINFORMAT_CONTEXT         *Context,
  OUT BINFORMAT_ARCH_ITERATOR   **Iterator
  );

/**
  Get next architecture from iterator.

  @param[in]   Iterator          Architecture iterator.
  @param[out]  Architecture      Pointer to receive architecture info.

  @retval BINFORMAT_SUCCESS      Architecture retrieved.
  @retval BINFORMAT_ERROR_NOT_FOUND  No more architectures.
  @retval BINFORMAT_ERROR_*      Error occurred.

**/
typedef
BINFORMAT_STATUS
(*BINFORMAT_ARCH_ITER_NEXT)(
  IN  BINFORMAT_ARCH_ITERATOR   *Iterator,
  OUT BINFORMAT_ARCHITECTURE    *Architecture
  );

/**
  Free architecture iterator.

  @param[in]   Iterator          Architecture iterator to free.

**/
typedef
VOID
(*BINFORMAT_ARCH_ITER_FREE)(
  IN  BINFORMAT_ARCH_ITERATOR  *Iterator
  );

/**
  Binary format library API table.

  All binary format libraries (ELF, COFF, a.out, Mach-O) implement
  this identical interface.
**/
typedef struct {
  ///
  /// Library identification
  ///
  CONST CHAR8  *LibraryName;     ///< Library name (e.g., "libelf")
  UINT32       Version;          ///< Library version

  ///
  /// Initialization and teardown
  ///
  BINFORMAT_INIT_FILE              InitFile;
  BINFORMAT_INIT_MEMORY            InitMemory;
  BINFORMAT_CREATE                 Create;
  BINFORMAT_CLOSE                  Close;

  ///
  /// Query operations
  ///
  BINFORMAT_GET_HEADER             GetHeader;
  BINFORMAT_GET_SECTION            GetSection;
  BINFORMAT_GET_SECTION_BY_NAME    GetSectionByName;
  BINFORMAT_GET_SEGMENT            GetSegment;
  BINFORMAT_GET_SYMBOL             GetSymbol;
  BINFORMAT_GET_SYMBOL_BY_NAME     GetSymbolByName;

  ///
  /// Modification operations
  ///
  BINFORMAT_ADD_SECTION            AddSection;
  BINFORMAT_ADD_SYMBOL             AddSymbol;
  BINFORMAT_ADD_RELOCATION         AddRelocation;
  BINFORMAT_UPDATE_SECTION         UpdateSection;
  BINFORMAT_UPDATE_SYMBOL          UpdateSymbol;
  BINFORMAT_UPDATE_RELOCATION      UpdateRelocation;

  ///
  /// Output operations
  ///
  BINFORMAT_WRITE_FILE             WriteFile;
  BINFORMAT_WRITE_MEMORY           WriteMemory;

  ///
  /// Multi-architecture support
  ///
  BINFORMAT_SELECT_ARCHITECTURE    SelectArchitecture;

  ///
  /// Fat/Universal binary operations (for lipo tool)
  ///
  BINFORMAT_CREATE_FAT             CreateFat;
  BINFORMAT_ADD_ARCH_SLICE         AddArchSlice;
  BINFORMAT_REMOVE_ARCH_SLICE      RemoveArchSlice;
  BINFORMAT_REPLACE_ARCH_SLICE     ReplaceArchSlice;
  BINFORMAT_EXTRACT_THIN           ExtractThin;

  ///
  /// Code signature operations (for codesign tool)
  ///
  BINFORMAT_GET_SIGNATURE          GetSignature;
  BINFORMAT_VERIFY_SIGNATURE       VerifySignature;
  BINFORMAT_SIGN_BINARY            SignBinary;
  BINFORMAT_REMOVE_SIGNATURE       RemoveSignature;

  ///
  /// Iterator operations - safe iteration without raw arrays/pointers
  ///
  BINFORMAT_SECTION_ITER_CREATE    SectionIterCreate;
  BINFORMAT_SECTION_ITER_NEXT      SectionIterNext;
  BINFORMAT_SECTION_ITER_FREE      SectionIterFree;

  BINFORMAT_SYMBOL_ITER_CREATE     SymbolIterCreate;
  BINFORMAT_SYMBOL_ITER_NEXT       SymbolIterNext;
  BINFORMAT_SYMBOL_ITER_FREE       SymbolIterFree;

  BINFORMAT_SEGMENT_ITER_CREATE    SegmentIterCreate;
  BINFORMAT_SEGMENT_ITER_NEXT      SegmentIterNext;
  BINFORMAT_SEGMENT_ITER_FREE      SegmentIterFree;

  BINFORMAT_RELOCATION_ITER_CREATE RelocationIterCreate;
  BINFORMAT_RELOCATION_ITER_NEXT   RelocationIterNext;
  BINFORMAT_RELOCATION_ITER_FREE   RelocationIterFree;

  BINFORMAT_ARCH_ITER_CREATE       ArchIterCreate;
  BINFORMAT_ARCH_ITER_NEXT         ArchIterNext;
  BINFORMAT_ARCH_ITER_FREE         ArchIterFree;
} BINFORMAT_API;

///
/// Library API Accessors
///
/// These functions return the API table for each binary format library.
/// Tools should use these to access format-specific implementations.
///

/**
  Get ELF binary format library API.

  @return Pointer to ELF library API table.
**/
CONST BINFORMAT_API *
ElfGetApi(
  VOID
  );

/**
  Get COFF/PE binary format library API.

  @return Pointer to COFF library API table.
**/
CONST BINFORMAT_API *
CoffGetApi(
  VOID
  );

/**
  Get Mach-O binary format library API.

  @return Pointer to Mach-O library API table.
**/
CONST BINFORMAT_API *
MachoGetApi(
  VOID
  );

/**
  Get a.out binary format library API.

  @return Pointer to a.out library API table.
**/
CONST BINFORMAT_API *
AoutGetApi(
  VOID
  );

/**
  Get OMF binary format library API.

  @return Pointer to OMF library API table.
**/
CONST BINFORMAT_API *
OmfGetApi(
  VOID
  );

/**
  Get ORF binary format library API.

  @return Pointer to ORF library API table.
**/
CONST BINFORMAT_API *
OrfGetApi(
  VOID
  );

/**
  Get Minidump format library API.

  @return Pointer to Minidump library API table.
**/
CONST BINFORMAT_API *
MinidumpGetApi(
  VOID
  );

///
/// Helper Functions and Default Implementations
///
/// These provide default implementations for common operations to avoid
/// developers and users having to reinvent the wheel.
///

/**
  Detect binary format from file.

  This function tries to detect the binary format by attempting to initialize
  with each known format library. Returns the API of the first library that
  successfully recognizes the format.

  @param[in]   FilePath          Path to binary file.
  @param[out]  Context           Pointer to receive context handle.
  @param[in]   ReadOnly          TRUE for read-only access.

  @retval Pointer to API table if format detected.
  @retval NULL if format not recognized.
**/
CONST BINFORMAT_API *
BinFormatDetectFile(
  IN  CONST CHAR8        *FilePath,
  OUT BINFORMAT_CONTEXT  **Context,
  IN  BOOLEAN            ReadOnly
  );

/**
  Detect binary format from memory buffer.

  @param[in]   Buffer            Pointer to binary data.
  @param[in]   Size              Size of binary data.
  @param[out]  Context           Pointer to receive context handle.

  @retval Pointer to API table if format detected.
  @retval NULL if format not recognized.
**/
CONST BINFORMAT_API *
BinFormatDetectMemory(
  IN  CONST VOID         *Buffer,
  IN  UINT64             Size,
  OUT BINFORMAT_CONTEXT  **Context
  );

/**
  Get human-readable name for machine type.

  @param[in]   Machine           Machine type.

  @return Pointer to machine name string.
**/
CONST CHAR8 *
BinFormatGetMachineName(
  IN  BINFORMAT_MACHINE  Machine
  );

/**
  Get human-readable name for file type.

  @param[in]   FileType          File type.

  @return Pointer to file type name string.
**/
CONST CHAR8 *
BinFormatGetFileTypeName(
  IN  BINFORMAT_FILE_TYPE  FileType
  );

/**
  Get human-readable name for symbol type.

  @param[in]   SymbolType        Symbol type.

  @return Pointer to symbol type name string.
**/
CONST CHAR8 *
BinFormatGetSymbolTypeName(
  IN  BINFORMAT_SYMBOL_TYPE  SymbolType
  );

/**
  Get human-readable name for symbol binding.

  @param[in]   SymbolBind        Symbol binding.

  @return Pointer to symbol binding name string.
**/
CONST CHAR8 *
BinFormatGetSymbolBindName(
  IN  BINFORMAT_SYMBOL_BIND  SymbolBind
  );

/**
  Get short symbol type character (for nm-style output).

  Returns characters like 'T' (text), 'D' (data), 'B' (BSS), 'U' (undefined).

  @param[in]   Symbol            Symbol descriptor.

  @return Symbol type character.
**/
CHAR8
BinFormatGetSymbolTypeChar(
  IN  CONST BINFORMAT_SYMBOL  *Symbol
  );

#endif // __BINFORMAT_H__
