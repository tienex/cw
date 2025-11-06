/** @file
  Windows Minidump Binary Format Library.

  This library provides support for reading and creating Windows minidump
  files. Minidumps are crash dump files created by Windows when a process
  crashes or when MiniDumpWriteDump() is called. They implement the unified
  BINFORMAT_API interface and use NT CONTEXT structures from CoreContext.h.

  Supported format:
  - Windows Minidump format (MDMP signature)

  Features:
  - Read thread contexts (register state) for all threads
  - Read loaded module information
  - Read memory regions
  - Read exception information
  - Read system information
  - Convert minidump to ELF core or vice versa
  - Create minidumps programmatically

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __LIBMINIDUMP_H__
#define __LIBMINIDUMP_H__

#include "../../include/binformat/BinFormat.h"
#include "CoreContext.h"

///
/// Minidump signature "MDMP" (0x504D444D in little-endian)
///
#define MINIDUMP_SIGNATURE     0x504D444D
#define MINIDUMP_VERSION       ((UINT16)0xA793)

///
/// MINIDUMP_STREAM_TYPE enumeration
///
/// Identifies the type of information stored in a minidump stream.
///
typedef enum _MINIDUMP_STREAM_TYPE {
  UnusedStream                = 0,
  ReservedStream0             = 1,
  ReservedStream1             = 2,
  ThreadListStream            = 3,    ///< List of threads with contexts
  ModuleListStream            = 4,    ///< List of loaded modules
  MemoryListStream            = 5,    ///< List of memory regions
  ExceptionStream             = 6,    ///< Exception information
  SystemInfoStream            = 7,    ///< System information
  ThreadExListStream          = 8,    ///< Extended thread list
  Memory64ListStream          = 9,    ///< Large memory list
  CommentStreamA              = 10,   ///< ASCII comment
  CommentStreamW              = 11,   ///< Unicode comment
  HandleDataStream            = 12,   ///< Handle information
  FunctionTableStream         = 13,   ///< Function table
  UnloadedModuleListStream    = 14,   ///< Unloaded modules
  MiscInfoStream              = 15,   ///< Miscellaneous info
  MemoryInfoListStream        = 16,   ///< Memory region info
  ThreadInfoListStream        = 17,   ///< Thread info
  HandleOperationListStream   = 18,   ///< Handle operations
  TokenStream                 = 19,   ///< Process token
  JavaScriptDataStream        = 20,   ///< JavaScript data
  SystemMemoryInfoStream      = 21,   ///< System memory info
  ProcessVmCountersStream     = 22,   ///< Process VM counters
  IptTraceStream              = 23,   ///< Intel PT trace
  ThreadNamesStream           = 24,   ///< Thread names
  LastReservedStream          = 0xFFFF
} MINIDUMP_STREAM_TYPE;

///
/// MINIDUMP_LOCATION_DESCRIPTOR
///
/// Describes the location of data within the minidump file.
///
typedef struct _MINIDUMP_LOCATION_DESCRIPTOR {
  UINT32  DataSize;   ///< Size of the data in bytes
  UINT32  Rva;        ///< Relative virtual address (file offset)
} MINIDUMP_LOCATION_DESCRIPTOR, *PMINIDUMP_LOCATION_DESCRIPTOR;

///
/// MINIDUMP_MEMORY_DESCRIPTOR
///
/// Describes a memory region.
///
typedef struct _MINIDUMP_MEMORY_DESCRIPTOR {
  UINT64                        StartOfMemoryRange; ///< Virtual address
  MINIDUMP_LOCATION_DESCRIPTOR  Memory;              ///< Memory contents location
} MINIDUMP_MEMORY_DESCRIPTOR, *PMINIDUMP_MEMORY_DESCRIPTOR;

///
/// MINIDUMP_MEMORY_DESCRIPTOR64
///
/// Describes a 64-bit memory region (used in Memory64ListStream).
///
typedef struct _MINIDUMP_MEMORY_DESCRIPTOR64 {
  UINT64  StartOfMemoryRange; ///< Virtual address
  UINT64  DataSize;            ///< Size of memory region
} MINIDUMP_MEMORY_DESCRIPTOR64, *PMINIDUMP_MEMORY_DESCRIPTOR64;

///
/// MINIDUMP_HEADER
///
/// The minidump file header appears at the start of every minidump file.
///
typedef struct _MINIDUMP_HEADER {
  UINT32  Signature;         ///< Must be MINIDUMP_SIGNATURE (0x504D444D "MDMP")
  UINT16  Version;           ///< MINIDUMP_VERSION (0xA793)
  UINT16  ImplementationVersion; ///< Implementation-specific version
  UINT32  NumberOfStreams;   ///< Number of directory entries
  UINT32  StreamDirectoryRva; ///< RVA to stream directory
  UINT32  CheckSum;          ///< Checksum (0 if not used)
  UINT32  Reserved;          ///< Reserved (should be 0)
  UINT64  TimeDateStamp;     ///< Time of dump creation
  UINT64  Flags;             ///< MINIDUMP_TYPE flags
} MINIDUMP_HEADER, *PMINIDUMP_HEADER;

///
/// MINIDUMP_DIRECTORY
///
/// A directory entry that describes the location and type of a stream.
///
typedef struct _MINIDUMP_DIRECTORY {
  UINT32                        StreamType; ///< MINIDUMP_STREAM_TYPE
  MINIDUMP_LOCATION_DESCRIPTOR  Location;   ///< Stream location
} MINIDUMP_DIRECTORY, *PMINIDUMP_DIRECTORY;

///
/// MINIDUMP_STRING
///
/// A Unicode string stored in the minidump.
///
typedef struct _MINIDUMP_STRING {
  UINT32  Length;     ///< Length in bytes (not including null terminator)
  UINT16  Buffer[1];  ///< Unicode string data (variable length)
} MINIDUMP_STRING, *PMINIDUMP_STRING;

///
/// MINIDUMP_THREAD
///
/// Describes a thread in the process.
///
typedef struct _MINIDUMP_THREAD {
  UINT32                        ThreadId;
  UINT32                        SuspendCount;
  UINT32                        PriorityClass;
  UINT32                        Priority;
  UINT64                        Teb;           ///< Thread Environment Block address
  MINIDUMP_MEMORY_DESCRIPTOR    Stack;         ///< Stack memory descriptor
  MINIDUMP_LOCATION_DESCRIPTOR  ThreadContext; ///< Thread CONTEXT structure
} MINIDUMP_THREAD, *PMINIDUMP_THREAD;

///
/// MINIDUMP_THREAD_LIST
///
/// A list of threads in the process.
///
typedef struct _MINIDUMP_THREAD_LIST {
  UINT32            NumberOfThreads;
  MINIDUMP_THREAD   Threads[1]; ///< Variable length array
} MINIDUMP_THREAD_LIST, *PMINIDUMP_THREAD_LIST;

///
/// MINIDUMP_THREAD_EX
///
/// Extended thread information (Windows 7+).
///
typedef struct _MINIDUMP_THREAD_EX {
  UINT32                        ThreadId;
  UINT32                        SuspendCount;
  UINT32                        PriorityClass;
  UINT32                        Priority;
  UINT64                        Teb;
  MINIDUMP_MEMORY_DESCRIPTOR    Stack;
  MINIDUMP_LOCATION_DESCRIPTOR  ThreadContext;
  MINIDUMP_MEMORY_DESCRIPTOR    BackingStore; ///< IA-64 backing store
} MINIDUMP_THREAD_EX, *PMINIDUMP_THREAD_EX;

///
/// MINIDUMP_THREAD_EX_LIST
///
/// A list of extended threads.
///
typedef struct _MINIDUMP_THREAD_EX_LIST {
  UINT32               NumberOfThreads;
  MINIDUMP_THREAD_EX   Threads[1];
} MINIDUMP_THREAD_EX_LIST, *PMINIDUMP_THREAD_EX_LIST;

///
/// MINIDUMP_MODULE
///
/// Describes a loaded module (DLL or EXE).
///
typedef struct _MINIDUMP_MODULE {
  UINT64  BaseOfImage;
  UINT32  SizeOfImage;
  UINT32  CheckSum;
  UINT32  TimeDateStamp;
  UINT32  ModuleNameRva;       ///< RVA to MINIDUMP_STRING
  UINT32  VersionInfoSize;
  UINT32  VersionInfoOffset;
  UINT64  CvRecord;             ///< CodeView debug info
  UINT32  CvRecordSize;
  UINT32  CvRecordOffset;
  UINT32  MiscRecordSize;
  UINT32  MiscRecordOffset;
  UINT64  Reserved0;
  UINT64  Reserved1;
} MINIDUMP_MODULE, *PMINIDUMP_MODULE;

///
/// MINIDUMP_MODULE_LIST
///
/// A list of loaded modules.
///
typedef struct _MINIDUMP_MODULE_LIST {
  UINT32           NumberOfModules;
  MINIDUMP_MODULE  Modules[1];
} MINIDUMP_MODULE_LIST, *PMINIDUMP_MODULE_LIST;

///
/// MINIDUMP_MEMORY_LIST
///
/// A list of memory regions.
///
typedef struct _MINIDUMP_MEMORY_LIST {
  UINT32                       NumberOfMemoryRanges;
  MINIDUMP_MEMORY_DESCRIPTOR   MemoryRanges[1];
} MINIDUMP_MEMORY_LIST, *PMINIDUMP_MEMORY_LIST;

///
/// MINIDUMP_MEMORY64_LIST
///
/// A list of 64-bit memory regions (more efficient for large dumps).
///
typedef struct _MINIDUMP_MEMORY64_LIST {
  UINT64                         NumberOfMemoryRanges;
  UINT64                         BaseRva; ///< Base RVA for all memory ranges
  MINIDUMP_MEMORY_DESCRIPTOR64   MemoryRanges[1];
} MINIDUMP_MEMORY64_LIST, *PMINIDUMP_MEMORY64_LIST;

///
/// MINIDUMP_EXCEPTION
///
/// Describes an exception.
///
typedef struct _MINIDUMP_EXCEPTION {
  UINT32  ExceptionCode;
  UINT32  ExceptionFlags;
  UINT64  ExceptionRecord; ///< Pointer to nested EXCEPTION_RECORD
  UINT64  ExceptionAddress;
  UINT32  NumberParameters;
  UINT32  Unused;
  UINT64  ExceptionInformation[15]; ///< EXCEPTION_MAXIMUM_PARAMETERS
} MINIDUMP_EXCEPTION, *PMINIDUMP_EXCEPTION;

///
/// MINIDUMP_EXCEPTION_STREAM
///
/// Exception information stream.
///
typedef struct _MINIDUMP_EXCEPTION_STREAM {
  UINT32                        ThreadId;
  UINT32                        Alignment;
  MINIDUMP_EXCEPTION            ExceptionRecord;
  MINIDUMP_LOCATION_DESCRIPTOR  ThreadContext; ///< Context at time of exception
} MINIDUMP_EXCEPTION_STREAM, *PMINIDUMP_EXCEPTION_STREAM;

///
/// MINIDUMP_SYSTEM_INFO
///
/// System information stream.
///
typedef struct _MINIDUMP_SYSTEM_INFO {
  UINT16  ProcessorArchitecture;
  UINT16  ProcessorLevel;
  UINT16  ProcessorRevision;
  UINT8   NumberOfProcessors;
  UINT8   ProductType;
  UINT32  MajorVersion;
  UINT32  MinorVersion;
  UINT32  BuildNumber;
  UINT32  PlatformId;
  UINT32  CSDVersionRva; ///< RVA to service pack string
  UINT16  SuiteMask;
  UINT16  Reserved2;
  UINT32  VendorId[3];   ///< CPU vendor (e.g., "GenuineIntel")
  UINT32  VersionInformation;
  UINT32  FeatureInformation;
  UINT32  AMDExtendedCpuFeatures;
} MINIDUMP_SYSTEM_INFO, *PMINIDUMP_SYSTEM_INFO;

///
/// Processor architectures
///
#define PROCESSOR_ARCHITECTURE_INTEL     0
#define PROCESSOR_ARCHITECTURE_ARM       5
#define PROCESSOR_ARCHITECTURE_IA64      6
#define PROCESSOR_ARCHITECTURE_AMD64     9
#define PROCESSOR_ARCHITECTURE_ARM64     12
#define PROCESSOR_ARCHITECTURE_UNKNOWN   0xFFFF

///
/// MINIDUMP_MISC_INFO
///
/// Miscellaneous information stream.
///
typedef struct _MINIDUMP_MISC_INFO {
  UINT32  SizeOfInfo;
  UINT32  Flags1;
  UINT32  ProcessId;
  UINT32  ProcessCreateTime;
  UINT32  ProcessUserTime;
  UINT32  ProcessKernelTime;
} MINIDUMP_MISC_INFO, *PMINIDUMP_MISC_INFO;

///
/// MINIDUMP_MISC_INFO_2
///
/// Extended miscellaneous information (Windows Vista+).
///
typedef struct _MINIDUMP_MISC_INFO_2 {
  UINT32  SizeOfInfo;
  UINT32  Flags1;
  UINT32  ProcessId;
  UINT32  ProcessCreateTime;
  UINT32  ProcessUserTime;
  UINT32  ProcessKernelTime;
  UINT32  ProcessorMaxMhz;
  UINT32  ProcessorCurrentMhz;
  UINT32  ProcessorMhzLimit;
  UINT32  ProcessorMaxIdleState;
  UINT32  ProcessorCurrentIdleState;
} MINIDUMP_MISC_INFO_2, *PMINIDUMP_MISC_INFO_2;

///
/// MINIDUMP_HANDLE_DESCRIPTOR
///
/// Describes an open handle.
///
typedef struct _MINIDUMP_HANDLE_DESCRIPTOR {
  UINT64  Handle;
  UINT32  TypeNameRva;
  UINT32  ObjectNameRva;
  UINT32  Attributes;
  UINT32  GrantedAccess;
  UINT32  HandleCount;
  UINT32  PointerCount;
} MINIDUMP_HANDLE_DESCRIPTOR, *PMINIDUMP_HANDLE_DESCRIPTOR;

///
/// MINIDUMP_HANDLE_DATA_STREAM
///
/// List of handles.
///
typedef struct _MINIDUMP_HANDLE_DATA_STREAM {
  UINT32  SizeOfHeader;
  UINT32  SizeOfDescriptor;
  UINT32  NumberOfDescriptors;
  UINT32  Reserved;
} MINIDUMP_HANDLE_DATA_STREAM, *PMINIDUMP_HANDLE_DATA_STREAM;

///
/// MINIDUMP_MEMORY_INFO
///
/// Describes a memory region's properties.
///
typedef struct _MINIDUMP_MEMORY_INFO {
  UINT64  BaseAddress;
  UINT64  AllocationBase;
  UINT32  AllocationProtect;
  UINT32  Alignment1;
  UINT64  RegionSize;
  UINT32  State;
  UINT32  Protect;
  UINT32  Type;
  UINT32  Alignment2;
} MINIDUMP_MEMORY_INFO, *PMINIDUMP_MEMORY_INFO;

///
/// MINIDUMP_MEMORY_INFO_LIST
///
/// List of memory region information.
///
typedef struct _MINIDUMP_MEMORY_INFO_LIST {
  UINT32                 SizeOfHeader;
  UINT32                 SizeOfEntry;
  UINT64                 NumberOfEntries;
  MINIDUMP_MEMORY_INFO   MemoryInfo[1];
} MINIDUMP_MEMORY_INFO_LIST, *PMINIDUMP_MEMORY_INFO_LIST;

///
/// MINIDUMP_THREAD_INFO
///
/// Extended thread information.
///
typedef struct _MINIDUMP_THREAD_INFO {
  UINT32  ThreadId;
  UINT32  DumpFlags;
  UINT32  DumpError;
  UINT32  ExitStatus;
  UINT64  CreateTime;
  UINT64  ExitTime;
  UINT64  KernelTime;
  UINT64  UserTime;
  UINT64  StartAddress;
  UINT64  Affinity;
} MINIDUMP_THREAD_INFO, *PMINIDUMP_THREAD_INFO;

///
/// MINIDUMP_THREAD_INFO_LIST
///
/// List of extended thread information.
///
typedef struct _MINIDUMP_THREAD_INFO_LIST {
  UINT32                 SizeOfHeader;
  UINT32                 SizeOfEntry;
  UINT32                 NumberOfEntries;
  MINIDUMP_THREAD_INFO   ThreadInfo[1];
} MINIDUMP_THREAD_INFO_LIST, *PMINIDUMP_THREAD_INFO_LIST;

///
/// MINIDUMP_THREAD_NAME
///
/// Thread name (Windows 10+).
///
typedef struct _MINIDUMP_THREAD_NAME {
  UINT32  ThreadId;
  UINT64  RvaOfThreadName; ///< RVA to MINIDUMP_STRING
} MINIDUMP_THREAD_NAME, *PMINIDUMP_THREAD_NAME;

///
/// MINIDUMP_THREAD_NAME_LIST
///
/// List of thread names.
///
typedef struct _MINIDUMP_THREAD_NAME_LIST {
  UINT32                NumberOfThreadNames;
  MINIDUMP_THREAD_NAME  ThreadNames[1];
} MINIDUMP_THREAD_NAME_LIST, *PMINIDUMP_THREAD_NAME_LIST;

///
/// MINIDUMP_TYPE flags
///
/// Specifies what information to include in the minidump.
///
#define MiniDumpNormal                         0x00000000
#define MiniDumpWithDataSegs                   0x00000001
#define MiniDumpWithFullMemory                 0x00000002
#define MiniDumpWithHandleData                 0x00000004
#define MiniDumpFilterMemory                   0x00000008
#define MiniDumpScanMemory                     0x00000010
#define MiniDumpWithUnloadedModules            0x00000020
#define MiniDumpWithIndirectlyReferencedMemory 0x00000040
#define MiniDumpFilterModulePaths              0x00000080
#define MiniDumpWithProcessThreadData          0x00000100
#define MiniDumpWithPrivateReadWriteMemory     0x00000200
#define MiniDumpWithoutOptionalData            0x00000400
#define MiniDumpWithFullMemoryInfo             0x00000800
#define MiniDumpWithThreadInfo                 0x00001000
#define MiniDumpWithCodeSegs                   0x00002000
#define MiniDumpWithoutAuxiliaryState          0x00004000
#define MiniDumpWithFullAuxiliaryState         0x00008000
#define MiniDumpWithPrivateWriteCopyMemory     0x00010000
#define MiniDumpIgnoreInaccessibleMemory       0x00020000
#define MiniDumpWithTokenInformation           0x00040000
#define MiniDumpWithModuleHeaders              0x00080000
#define MiniDumpFilterTriage                   0x00100000
#define MiniDumpWithAvxXStateContext           0x00200000
#define MiniDumpWithIptTrace                   0x00400000
#define MiniDumpScanInaccessiblePartialPages   0x00800000
#define MiniDumpFilterWriteCombinedMemory      0x01000000
#define MiniDumpValidTypeFlags                 0x01FFFFFF

///
/// Get the Windows Minidump library API table.
///
/// @return Pointer to Minidump library API table.
///
EXTERN CONST BINFORMAT_API *
MinidumpGetApi (
  VOID
  );

#endif // __LIBMINIDUMP_H__
