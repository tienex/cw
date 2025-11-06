/** @file
  codesign - Code signature inspection and management tool.

  This tool inspects, verifies, and manages code signatures in binary files.
  Similar to Apple's codesign utility.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "binformat/BinFormat.h"

typedef enum {
  CMD_NONE = 0,
  CMD_DISPLAY,
  CMD_VERIFY,
  CMD_SIGN,
  CMD_REMOVE
} CODESIGN_COMMAND;

typedef struct {
  CODESIGN_COMMAND  Command;
  CHAR8             *InputFile;
  CHAR8             *Identity;
  CHAR8             *OutputFile;
  BOOLEAN           Verbose;
  BOOLEAN           Deep;
  BOOLEAN           Force;
} CODESIGN_OPTIONS;

STATIC CODESIGN_OPTIONS gOptions = {0};

/**
  Print usage information.
**/
STATIC
VOID
PrintUsage (
  VOID
  )
{
  printf ("Usage: codesign [options] <file>\n");
  printf ("\n");
  printf ("Commands:\n");
  printf ("  -d, --display            Display code signature information\n");
  printf ("  -v, --verify             Verify code signature\n");
  printf ("  -s <identity>            Sign code with identity\n");
  printf ("  --remove-signature       Remove code signature\n");
  printf ("\n");
  printf ("Options:\n");
  printf ("  -o <file>                Output file (for signing/removal)\n");
  printf ("  --verbose                Verbose output\n");
  printf ("  --deep                   Deep verification\n");
  printf ("  -f, --force              Force operation\n");
  printf ("  -h, --help               Show this help message\n");
  printf ("\n");
  printf ("Examples:\n");
  printf ("  codesign -d app.exe              Display signature info\n");
  printf ("  codesign -v app.exe              Verify signature\n");
  printf ("  codesign --remove-signature app.exe -o app_unsigned.exe\n");
  printf ("  codesign -s \"Developer ID\" app -o app_signed\n");
}

/**
  Get signature type name.

  @param[in]  Type  Signature type.

  @return String name of signature type.
**/
STATIC
CONST CHAR8 *
GetSignatureTypeName (
  IN  BINFORMAT_SIGNATURE_TYPE  Type
  )
{
  switch (Type) {
    case BinSignatureTypeNone:
      return "None";
    case BinSignatureTypeAdHoc:
      return "Ad-hoc";
    case BinSignatureTypeDeveloper:
      return "Developer";
    case BinSignatureTypeAppStore:
      return "App Store";
    case BinSignatureTypePGP:
      return "PGP";
    case BinSignatureTypeAuthenticode:
      return "Authenticode";
    case BinSignatureTypePKCS7:
      return "PKCS#7";
    case BinSignatureTypeX509:
      return "X.509";
    default:
      return "Unknown";
  }
}

/**
  Get hash algorithm name.

  @param[in]  Algorithm  Hash algorithm.

  @return String name of hash algorithm.
**/
STATIC
CONST CHAR8 *
GetHashAlgorithmName (
  IN  BINFORMAT_HASH_ALGORITHM  Algorithm
  )
{
  switch (Algorithm) {
    case BinHashNone:
      return "None";
    case BinHashSHA1:
      return "SHA-1";
    case BinHashSHA256:
      return "SHA-256";
    case BinHashSHA384:
      return "SHA-384";
    case BinHashSHA512:
      return "SHA-512";
    case BinHashMD5:
      return "MD5";
    default:
      return "Unknown";
  }
}

/**
  Display code signature information.

  @param[in]  FileName  Path to binary file.

  @return 0 on success, 1 on error.
**/
STATIC
INT32
DisplaySignature (
  IN  CONST CHAR8  *FileName
  )
{
  CONST BINFORMAT_API       *Api;
  BINFORMAT_CONTEXT         *Context;
  BINFORMAT_STATUS          Status;
  BINFORMAT_CODE_SIGNATURE  Signature;
  struct tm                 *TimeInfo;
  time_t                    Timestamp;

  Api = BinFormatDetectFile (FileName, &Context, TRUE);
  if (Api == NULL) {
    fprintf (stderr, "codesign: %s: File format not recognized\n", FileName);
    return 1;
  }

  if (Api->GetSignature == NULL) {
    printf ("Executable=%s\n", FileName);
    printf ("Format does not support code signatures\n");
    Api->Close (Context);
    return 0;
  }

  Status = Api->GetSignature (Context, &Signature);
  if (BINFORMAT_IS_ERROR (Status)) {
    if (Status == BINFORMAT_ERROR_NOT_FOUND) {
      printf ("Executable=%s\n", FileName);
      printf ("Code signature: not signed\n");
    } else {
      fprintf (stderr, "codesign: %s: Failed to get signature\n", FileName);
      Api->Close (Context);
      return 1;
    }
    Api->Close (Context);
    return 0;
  }

  //
  // Display signature information
  //
  printf ("Executable=%s\n", FileName);
  printf ("Signature type=%s\n", GetSignatureTypeName (Signature.Type));
  printf ("Hash algorithm=%s\n", GetHashAlgorithmName (Signature.HashAlgorithm));

  if (Signature.SignerName[0] != '\0') {
    printf ("Signer=%s\n", Signature.SignerName);
  }

  if (Signature.TeamID[0] != '\0') {
    printf ("TeamID=%s\n", Signature.TeamID);
  }

  if (Signature.BundleID[0] != '\0') {
    printf ("BundleID=%s\n", Signature.BundleID);
  }

  printf ("Signature offset=%llu\n", (unsigned long long)Signature.SignatureOffset);
  printf ("Signature size=%llu bytes\n", (unsigned long long)Signature.SignatureSize);
  printf ("Code limit=%llu bytes\n", (unsigned long long)Signature.CodeLimit);
  printf ("Flags=0x%x\n", Signature.Flags);

  if (Signature.Timestamp != 0) {
    Timestamp = (time_t)Signature.Timestamp;
    TimeInfo = localtime (&Timestamp);
    if (TimeInfo != NULL) {
      printf ("Signed time=%04d-%02d-%02d %02d:%02d:%02d\n",
              TimeInfo->tm_year + 1900,
              TimeInfo->tm_mon + 1,
              TimeInfo->tm_mday,
              TimeInfo->tm_hour,
              TimeInfo->tm_min,
              TimeInfo->tm_sec);
    }
  }

  if (gOptions.Verbose) {
    printf ("\nDetailed Information:\n");
    printf ("  Signed: %s\n", Signature.IsSigned ? "Yes" : "No");
    printf ("  Valid: %s\n", Signature.IsValid ? "Yes" : "No (or not verified)");
  }

  Api->Close (Context);
  return 0;
}

/**
  Verify code signature.

  @param[in]  FileName  Path to binary file.

  @return 0 if valid, 1 if invalid or error.
**/
STATIC
INT32
VerifySignature (
  IN  CONST CHAR8  *FileName
  )
{
  CONST BINFORMAT_API  *Api;
  BINFORMAT_CONTEXT    *Context;
  BINFORMAT_STATUS     Status;
  BOOLEAN              IsValid = FALSE;

  Api = BinFormatDetectFile (FileName, &Context, TRUE);
  if (Api == NULL) {
    fprintf (stderr, "codesign: %s: File format not recognized\n", FileName);
    return 1;
  }

  if (Api->VerifySignature == NULL) {
    printf ("%s: format does not support signature verification\n", FileName);
    Api->Close (Context);
    return 1;
  }

  Status = Api->VerifySignature (Context, &IsValid);
  if (BINFORMAT_IS_ERROR (Status)) {
    if (Status == BINFORMAT_ERROR_NOT_FOUND) {
      printf ("%s: code signature not found\n", FileName);
      if (gOptions.Verbose) {
        printf ("Binary is not signed\n");
      }
    } else {
      fprintf (stderr, "codesign: %s: Signature verification failed\n", FileName);
    }
    Api->Close (Context);
    return 1;
  }

  if (IsValid) {
    printf ("%s: valid signature\n", FileName);
    if (gOptions.Verbose) {
      printf ("Code signature is valid and verified\n");
    }
    Api->Close (Context);
    return 0;
  } else {
    printf ("%s: invalid signature\n", FileName);
    if (gOptions.Verbose) {
      printf ("Code signature verification failed\n");
      printf ("The signature may be corrupted or the binary has been modified\n");
    }
    Api->Close (Context);
    return 1;
  }
}

/**
  Remove code signature from binary.

  @param[in]  InputFile   Path to input binary file.
  @param[in]  OutputFile  Path to output file (optional).

  @return 0 on success, 1 on error.
**/
STATIC
INT32
RemoveSignature (
  IN  CONST CHAR8  *InputFile,
  IN  CONST CHAR8  *OutputFile
  )
{
  CONST BINFORMAT_API  *Api;
  BINFORMAT_CONTEXT    *Context;
  BINFORMAT_STATUS     Status;

  if (OutputFile == NULL && !gOptions.Force) {
    fprintf (stderr, "codesign: removing signature requires -o or -f flag\n");
    return 1;
  }

  Api = BinFormatDetectFile (InputFile, &Context, OutputFile == NULL ? FALSE : TRUE);
  if (Api == NULL) {
    fprintf (stderr, "codesign: %s: File format not recognized\n", InputFile);
    return 1;
  }

  if (Api->RemoveSignature == NULL) {
    fprintf (stderr, "codesign: format does not support signature removal\n");
    Api->Close (Context);
    return 1;
  }

  Status = Api->RemoveSignature (Context);
  if (BINFORMAT_IS_ERROR (Status)) {
    if (Status == BINFORMAT_ERROR_NOT_FOUND) {
      fprintf (stderr, "codesign: %s: no signature to remove\n", InputFile);
    } else {
      fprintf (stderr, "codesign: %s: Failed to remove signature\n", InputFile);
    }
    Api->Close (Context);
    return 1;
  }

  //
  // Write output if specified
  //
  if (OutputFile != NULL) {
    if (Api->WriteFile == NULL) {
      fprintf (stderr, "codesign: format does not support writing\n");
      Api->Close (Context);
      return 1;
    }

    Status = Api->WriteFile (Context, OutputFile);
    if (BINFORMAT_IS_ERROR (Status)) {
      fprintf (stderr, "codesign: failed to write output file %s\n", OutputFile);
      Api->Close (Context);
      return 1;
    }

    printf ("%s: signature removed, written to %s\n", InputFile, OutputFile);
  } else {
    printf ("%s: signature removed (in-place)\n", InputFile);
  }

  Api->Close (Context);
  return 0;
}

/**
  Sign binary with code signature.
  Note: This is a stub - SignBinary API not yet fully implemented.

  @param[in]  InputFile   Path to input binary file.
  @param[in]  Identity    Signing identity.
  @param[in]  OutputFile  Path to output file (optional).

  @return 0 on success, 1 on error.
**/
STATIC
INT32
SignBinary (
  IN  CONST CHAR8  *InputFile,
  IN  CONST CHAR8  *Identity,
  IN  CONST CHAR8  *OutputFile
  )
{
  if (OutputFile == NULL) {
    fprintf (stderr, "codesign: signing requires -o output file\n");
    return 1;
  }

  fprintf (stderr, "codesign: signing not yet fully implemented\n");
  fprintf (stderr, "Would sign %s with identity '%s' and save to %s\n",
           InputFile, Identity, OutputFile);
  fprintf (stderr, "\nNote: SignBinary callback needs implementation in library\n");
  fprintf (stderr, "For production signing, use platform-specific tools:\n");
  fprintf (stderr, "  - macOS: codesign (built-in)\n");
  fprintf (stderr, "  - Windows: signtool.exe\n");
  fprintf (stderr, "  - Linux: gpg or osslsigncode\n");

  return 1;
}

/**
  Main entry point.

  @param[in]  argc  Argument count.
  @param[in]  argv  Argument array.

  @return Exit code (0 for success).
**/
INT32
main (
  IN  INT32   argc,
  IN  CHAR8   **argv
  )
{
  INT32   opt;
  INT32   Result = 0;

  struct option longOptions[] = {
    {"display",          no_argument,       0, 'd'},
    {"verify",           no_argument,       0, 'v'},
    {"remove-signature", no_argument,       0, 'R'},
    {"verbose",          no_argument,       0, 'V'},
    {"deep",             no_argument,       0, 'D'},
    {"force",            no_argument,       0, 'f'},
    {"help",             no_argument,       0, 'h'},
    {0, 0, 0, 0}
  };

  if (argc < 2) {
    PrintUsage ();
    return 1;
  }

  //
  // Parse command-line options
  //
  while ((opt = getopt_long (argc, argv, "dvs:o:fhRVD", longOptions, NULL)) != -1) {
    switch (opt) {
      case 'd':
        gOptions.Command = CMD_DISPLAY;
        break;

      case 'v':
        gOptions.Command = CMD_VERIFY;
        break;

      case 's':
        gOptions.Command = CMD_SIGN;
        gOptions.Identity = optarg;
        break;

      case 'R':
        gOptions.Command = CMD_REMOVE;
        break;

      case 'o':
        gOptions.OutputFile = optarg;
        break;

      case 'V':
        gOptions.Verbose = TRUE;
        break;

      case 'D':
        gOptions.Deep = TRUE;
        break;

      case 'f':
        gOptions.Force = TRUE;
        break;

      case 'h':
        PrintUsage ();
        return 0;

      default:
        PrintUsage ();
        return 1;
    }
  }

  //
  // Get input file
  //
  if (optind < argc) {
    gOptions.InputFile = argv[optind];
  } else {
    fprintf (stderr, "codesign: no input file specified\n");
    return 1;
  }

  //
  // Execute command
  //
  switch (gOptions.Command) {
    case CMD_DISPLAY:
      Result = DisplaySignature (gOptions.InputFile);
      break;

    case CMD_VERIFY:
      Result = VerifySignature (gOptions.InputFile);
      break;

    case CMD_SIGN:
      Result = SignBinary (gOptions.InputFile, gOptions.Identity, gOptions.OutputFile);
      break;

    case CMD_REMOVE:
      Result = RemoveSignature (gOptions.InputFile, gOptions.OutputFile);
      break;

    default:
      fprintf (stderr, "codesign: no command specified (use -d, -v, -s, or --remove-signature)\n");
      PrintUsage ();
      Result = 1;
      break;
  }

  return Result;
}
