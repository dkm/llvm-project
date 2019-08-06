//===- LLVMBitCodes.h - Enum values for the LLVM bitcode format -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This header defines Bitcode enum values for LLVM IR bitcode files.
//
// The enum values defined in this file should be considered permanent.  If
// new features are added, they should have values added at the end of the
// respective lists.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_BITCODE_LLVMBITCODES_H
#define LLVM_BITCODE_LLVMBITCODES_H

#include "llvm/Bitstream/BitCodes.h"

namespace llvm {
namespace bitc {
// The only top-level block types are MODULE, IDENTIFICATION, STRTAB and SYMTAB.
enum BlockIDs {
  // Blocks
  MODULE_BLOCK_ID = FIRST_APPLICATION_BLOCKID,

  // Module sub-block id's.
  PARAMATTR_BLOCK_ID,
  PARAMATTR_GROUP_BLOCK_ID,

  CONSTANTS_BLOCK_ID,
  FUNCTION_BLOCK_ID,

  // Block intended to contains information on the bitcode versioning.
  // Can be used to provide better error messages when we fail to parse a
  // bitcode file.
  IDENTIFICATION_BLOCK_ID,

  VALUE_SYMTAB_BLOCK_ID,
  METADATA_BLOCK_ID,
  METADATA_ATTACHMENT_ID,

  TYPE_BLOCK_ID_NEW,

  USELIST_BLOCK_ID,

  MODULE_STRTAB_BLOCK_ID,
  GLOBALVAL_SUMMARY_BLOCK_ID,

  OPERAND_BUNDLE_TAGS_BLOCK_ID,

  METADATA_KIND_BLOCK_ID,

  STRTAB_BLOCK_ID,

  FULL_LTO_GLOBALVAL_SUMMARY_BLOCK_ID,

  SYMTAB_BLOCK_ID,

  SYNC_SCOPE_NAMES_BLOCK_ID,
};

/// Identification block contains a string that describes the producer details,
/// and an epoch that defines the auto-upgrade capability.
enum IdentificationCodes {
  IDENTIFICATION_CODE_STRING = 1, // IDENTIFICATION:      [strchr x N]
  IDENTIFICATION_CODE_EPOCH = 2,  // EPOCH:               [epoch#]
};

/// The epoch that defines the auto-upgrade compatibility for the bitcode.
///
/// LLVM guarantees in a major release that a minor release can read bitcode
/// generated by previous minor releases. We translate this by making the reader
/// accepting only bitcode with the same epoch, except for the X.0 release which
/// also accepts N-1.
enum { BITCODE_CURRENT_EPOCH = 0 };

/// MODULE blocks have a number of optional fields and subblocks.
enum ModuleCodes {
  MODULE_CODE_VERSION = 1,     // VERSION:     [version#]
  MODULE_CODE_TRIPLE = 2,      // TRIPLE:      [strchr x N]
  MODULE_CODE_DATALAYOUT = 3,  // DATALAYOUT:  [strchr x N]
  MODULE_CODE_ASM = 4,         // ASM:         [strchr x N]
  MODULE_CODE_SECTIONNAME = 5, // SECTIONNAME: [strchr x N]

  // FIXME: Remove DEPLIB in 4.0.
  MODULE_CODE_DEPLIB = 6, // DEPLIB:      [strchr x N]

  // GLOBALVAR: [pointer type, isconst, initid,
  //             linkage, alignment, section, visibility, threadlocal]
  MODULE_CODE_GLOBALVAR = 7,

  // FUNCTION:  [type, callingconv, isproto, linkage, paramattrs, alignment,
  //             section, visibility, gc, unnamed_addr]
  MODULE_CODE_FUNCTION = 8,

  // ALIAS: [alias type, aliasee val#, linkage, visibility]
  MODULE_CODE_ALIAS_OLD = 9,

  MODULE_CODE_GCNAME = 11, // GCNAME: [strchr x N]
  MODULE_CODE_COMDAT = 12, // COMDAT: [selection_kind, name]

  MODULE_CODE_VSTOFFSET = 13, // VSTOFFSET: [offset]

  // ALIAS: [alias value type, addrspace, aliasee val#, linkage, visibility]
  MODULE_CODE_ALIAS = 14,

  MODULE_CODE_METADATA_VALUES_UNUSED = 15,

  // SOURCE_FILENAME: [namechar x N]
  MODULE_CODE_SOURCE_FILENAME = 16,

  // HASH: [5*i32]
  MODULE_CODE_HASH = 17,

  // IFUNC: [ifunc value type, addrspace, resolver val#, linkage, visibility]
  MODULE_CODE_IFUNC = 18,
};

/// PARAMATTR blocks have code for defining a parameter attribute set.
enum AttributeCodes {
  // FIXME: Remove `PARAMATTR_CODE_ENTRY_OLD' in 4.0
  PARAMATTR_CODE_ENTRY_OLD = 1, // ENTRY: [paramidx0, attr0,
                                //         paramidx1, attr1...]
  PARAMATTR_CODE_ENTRY = 2,     // ENTRY: [attrgrp0, attrgrp1, ...]
  PARAMATTR_GRP_CODE_ENTRY = 3  // ENTRY: [grpid, idx, attr0, attr1, ...]
};

/// TYPE blocks have codes for each type primitive they use.
enum TypeCodes {
  TYPE_CODE_NUMENTRY = 1, // NUMENTRY: [numentries]

  // Type Codes
  TYPE_CODE_VOID = 2,    // VOID
  TYPE_CODE_FLOAT = 3,   // FLOAT
  TYPE_CODE_DOUBLE = 4,  // DOUBLE
  TYPE_CODE_LABEL = 5,   // LABEL
  TYPE_CODE_OPAQUE = 6,  // OPAQUE
  TYPE_CODE_INTEGER = 7, // INTEGER: [width]
  TYPE_CODE_POINTER = 8, // POINTER: [pointee type]

  TYPE_CODE_FUNCTION_OLD = 9, // FUNCTION: [vararg, attrid, retty,
                              //            paramty x N]

  TYPE_CODE_HALF = 10, // HALF

  TYPE_CODE_ARRAY = 11,  // ARRAY: [numelts, eltty]
  TYPE_CODE_VECTOR = 12, // VECTOR: [numelts, eltty]

  // These are not with the other floating point types because they're
  // a late addition, and putting them in the right place breaks
  // binary compatibility.
  TYPE_CODE_X86_FP80 = 13,  // X86 LONG DOUBLE
  TYPE_CODE_FP128 = 14,     // LONG DOUBLE (112 bit mantissa)
  TYPE_CODE_PPC_FP128 = 15, // PPC LONG DOUBLE (2 doubles)

  TYPE_CODE_METADATA = 16, // METADATA

  TYPE_CODE_X86_MMX = 17, // X86 MMX

  TYPE_CODE_STRUCT_ANON = 18,  // STRUCT_ANON: [ispacked, eltty x N]
  TYPE_CODE_STRUCT_NAME = 19,  // STRUCT_NAME: [strchr x N]
  TYPE_CODE_STRUCT_NAMED = 20, // STRUCT_NAMED: [ispacked, eltty x N]

  TYPE_CODE_FUNCTION = 21, // FUNCTION: [vararg, retty, paramty x N]

  TYPE_CODE_TOKEN = 22 // TOKEN
};

enum OperandBundleTagCode {
  OPERAND_BUNDLE_TAG = 1, // TAG: [strchr x N]
};

enum SyncScopeNameCode {
  SYNC_SCOPE_NAME = 1,
};

// Value symbol table codes.
enum ValueSymtabCodes {
  VST_CODE_ENTRY = 1,   // VST_ENTRY: [valueid, namechar x N]
  VST_CODE_BBENTRY = 2, // VST_BBENTRY: [bbid, namechar x N]
  VST_CODE_FNENTRY = 3, // VST_FNENTRY: [valueid, offset, namechar x N]
  // VST_COMBINED_ENTRY: [valueid, refguid]
  VST_CODE_COMBINED_ENTRY = 5
};

// The module path symbol table only has one code (MST_CODE_ENTRY).
enum ModulePathSymtabCodes {
  MST_CODE_ENTRY = 1, // MST_ENTRY: [modid, namechar x N]
  MST_CODE_HASH = 2,  // MST_HASH:  [5*i32]
};

// The summary section uses different codes in the per-module
// and combined index cases.
enum GlobalValueSummarySymtabCodes {
  // PERMODULE: [valueid, flags, instcount, numrefs, numrefs x valueid,
  //             n x (valueid)]
  FS_PERMODULE = 1,
  // PERMODULE_PROFILE: [valueid, flags, instcount, numrefs,
  //                     numrefs x valueid,
  //                     n x (valueid, hotness)]
  FS_PERMODULE_PROFILE = 2,
  // PERMODULE_GLOBALVAR_INIT_REFS: [valueid, flags, n x valueid]
  FS_PERMODULE_GLOBALVAR_INIT_REFS = 3,
  // COMBINED: [valueid, modid, flags, instcount, numrefs, numrefs x valueid,
  //            n x (valueid)]
  FS_COMBINED = 4,
  // COMBINED_PROFILE: [valueid, modid, flags, instcount, numrefs,
  //                    numrefs x valueid,
  //                    n x (valueid, hotness)]
  FS_COMBINED_PROFILE = 5,
  // COMBINED_GLOBALVAR_INIT_REFS: [valueid, modid, flags, n x valueid]
  FS_COMBINED_GLOBALVAR_INIT_REFS = 6,
  // ALIAS: [valueid, flags, valueid]
  FS_ALIAS = 7,
  // COMBINED_ALIAS: [valueid, modid, flags, valueid]
  FS_COMBINED_ALIAS = 8,
  // COMBINED_ORIGINAL_NAME: [original_name_hash]
  FS_COMBINED_ORIGINAL_NAME = 9,
  // VERSION of the summary, bumped when adding flags for instance.
  FS_VERSION = 10,
  // The list of llvm.type.test type identifiers used by the following function
  // that are used other than by an llvm.assume.
  // [n x typeid]
  FS_TYPE_TESTS = 11,
  // The list of virtual calls made by this function using
  // llvm.assume(llvm.type.test) intrinsics that do not have all constant
  // integer arguments.
  // [n x (typeid, offset)]
  FS_TYPE_TEST_ASSUME_VCALLS = 12,
  // The list of virtual calls made by this function using
  // llvm.type.checked.load intrinsics that do not have all constant integer
  // arguments.
  // [n x (typeid, offset)]
  FS_TYPE_CHECKED_LOAD_VCALLS = 13,
  // Identifies a virtual call made by this function using an
  // llvm.assume(llvm.type.test) intrinsic with all constant integer arguments.
  // [typeid, offset, n x arg]
  FS_TYPE_TEST_ASSUME_CONST_VCALL = 14,
  // Identifies a virtual call made by this function using an
  // llvm.type.checked.load intrinsic with all constant integer arguments.
  // [typeid, offset, n x arg]
  FS_TYPE_CHECKED_LOAD_CONST_VCALL = 15,
  // Assigns a GUID to a value ID. This normally appears only in combined
  // summaries, but it can also appear in per-module summaries for PGO data.
  // [valueid, guid]
  FS_VALUE_GUID = 16,
  // The list of local functions with CFI jump tables. Function names are
  // strings in strtab.
  // [n * name]
  FS_CFI_FUNCTION_DEFS = 17,
  // The list of external functions with CFI jump tables. Function names are
  // strings in strtab.
  // [n * name]
  FS_CFI_FUNCTION_DECLS = 18,
  // Per-module summary that also adds relative block frequency to callee info.
  // PERMODULE_RELBF: [valueid, flags, instcount, numrefs,
  //                   numrefs x valueid,
  //                   n x (valueid, relblockfreq)]
  FS_PERMODULE_RELBF = 19,
  // Index-wide flags
  FS_FLAGS = 20,
  // Maps type identifier to summary information for that type identifier.
  // Produced by the thin link (only lives in combined index).
  // TYPE_ID: [typeid, kind, bitwidth, align, size, bitmask, inlinebits,
  //           n x (typeid, kind, name, numrba,
  //                numrba x (numarg, numarg x arg, kind, info, byte, bit))]
  FS_TYPE_ID = 21,
  // For background see overview at https://llvm.org/docs/TypeMetadata.html.
  // The type metadata includes both the type identifier and the offset of
  // the address point of the type (the address held by objects of that type
  // which may not be the beginning of the virtual table). Vtable definitions
  // are decorated with type metadata for the types they are compatible with.
  //
  // Maps type identifier to summary information for that type identifier
  // computed from type metadata: the valueid of each vtable definition
  // decorated with a type metadata for that identifier, and the offset from
  // the corresponding type metadata.
  // Exists in the per-module summary to provide information to thin link
  // for index-based whole program devirtualization.
  // TYPE_ID_METADATA: [typeid, n x (valueid, offset)]
  FS_TYPE_ID_METADATA = 22,
  // Summarizes vtable definition for use in index-based whole program
  // devirtualization during the thin link.
  // PERMODULE_VTABLE_GLOBALVAR_INIT_REFS: [valueid, flags, varflags,
  //                                        numrefs, numrefs x valueid,
  //                                        n x (valueid, offset)]
  FS_PERMODULE_VTABLE_GLOBALVAR_INIT_REFS = 23,
};

enum MetadataCodes {
  METADATA_STRING_OLD = 1,     // MDSTRING:      [values]
  METADATA_VALUE = 2,          // VALUE:         [type num, value num]
  METADATA_NODE = 3,           // NODE:          [n x md num]
  METADATA_NAME = 4,           // STRING:        [values]
  METADATA_DISTINCT_NODE = 5,  // DISTINCT_NODE: [n x md num]
  METADATA_KIND = 6,           // [n x [id, name]]
  METADATA_LOCATION = 7,       // [distinct, line, col, scope, inlined-at?]
  METADATA_OLD_NODE = 8,       // OLD_NODE:      [n x (type num, value num)]
  METADATA_OLD_FN_NODE = 9,    // OLD_FN_NODE:   [n x (type num, value num)]
  METADATA_NAMED_NODE = 10,    // NAMED_NODE:    [n x mdnodes]
  METADATA_ATTACHMENT = 11,    // [m x [value, [n x [id, mdnode]]]
  METADATA_GENERIC_DEBUG = 12, // [distinct, tag, vers, header, n x md num]
  METADATA_SUBRANGE = 13,      // [distinct, count, lo]
  METADATA_ENUMERATOR = 14,    // [isUnsigned|distinct, value, name]
  METADATA_BASIC_TYPE = 15,    // [distinct, tag, name, size, align, enc]
  METADATA_FILE = 16, // [distinct, filename, directory, checksumkind, checksum]
  METADATA_DERIVED_TYPE = 17,       // [distinct, ...]
  METADATA_COMPOSITE_TYPE = 18,     // [distinct, ...]
  METADATA_SUBROUTINE_TYPE = 19,    // [distinct, flags, types, cc]
  METADATA_COMPILE_UNIT = 20,       // [distinct, ...]
  METADATA_SUBPROGRAM = 21,         // [distinct, ...]
  METADATA_LEXICAL_BLOCK = 22,      // [distinct, scope, file, line, column]
  METADATA_LEXICAL_BLOCK_FILE = 23, //[distinct, scope, file, discriminator]
  METADATA_NAMESPACE = 24, // [distinct, scope, file, name, line, exportSymbols]
  METADATA_TEMPLATE_TYPE = 25,   // [distinct, scope, name, type, ...]
  METADATA_TEMPLATE_VALUE = 26,  // [distinct, scope, name, type, value, ...]
  METADATA_GLOBAL_VAR = 27,      // [distinct, ...]
  METADATA_LOCAL_VAR = 28,       // [distinct, ...]
  METADATA_EXPRESSION = 29,      // [distinct, n x element]
  METADATA_OBJC_PROPERTY = 30,   // [distinct, name, file, line, ...]
  METADATA_IMPORTED_ENTITY = 31, // [distinct, tag, scope, entity, line, name]
  METADATA_MODULE = 32,          // [distinct, scope, name, ...]
  METADATA_MACRO = 33,           // [distinct, macinfo, line, name, value]
  METADATA_MACRO_FILE = 34,      // [distinct, macinfo, line, file, ...]
  METADATA_STRINGS = 35,         // [count, offset] blob([lengths][chars])
  METADATA_GLOBAL_DECL_ATTACHMENT = 36, // [valueid, n x [id, mdnode]]
  METADATA_GLOBAL_VAR_EXPR = 37,        // [distinct, var, expr]
  METADATA_INDEX_OFFSET = 38,           // [offset]
  METADATA_INDEX = 39,                  // [bitpos]
  METADATA_LABEL = 40,                  // [distinct, scope, name, file, line]
  METADATA_COMMON_BLOCK = 44,     // [distinct, scope, name, variable,...]
};

// The constants block (CONSTANTS_BLOCK_ID) describes emission for each
// constant and maintains an implicit current type value.
enum ConstantsCodes {
  CST_CODE_SETTYPE = 1,          // SETTYPE:       [typeid]
  CST_CODE_NULL = 2,             // NULL
  CST_CODE_UNDEF = 3,            // UNDEF
  CST_CODE_INTEGER = 4,          // INTEGER:       [intval]
  CST_CODE_WIDE_INTEGER = 5,     // WIDE_INTEGER:  [n x intval]
  CST_CODE_FLOAT = 6,            // FLOAT:         [fpval]
  CST_CODE_AGGREGATE = 7,        // AGGREGATE:     [n x value number]
  CST_CODE_STRING = 8,           // STRING:        [values]
  CST_CODE_CSTRING = 9,          // CSTRING:       [values]
  CST_CODE_CE_BINOP = 10,        // CE_BINOP:      [opcode, opval, opval]
  CST_CODE_CE_CAST = 11,         // CE_CAST:       [opcode, opty, opval]
  CST_CODE_CE_GEP = 12,          // CE_GEP:        [n x operands]
  CST_CODE_CE_SELECT = 13,       // CE_SELECT:     [opval, opval, opval]
  CST_CODE_CE_EXTRACTELT = 14,   // CE_EXTRACTELT: [opty, opval, opval]
  CST_CODE_CE_INSERTELT = 15,    // CE_INSERTELT:  [opval, opval, opval]
  CST_CODE_CE_SHUFFLEVEC = 16,   // CE_SHUFFLEVEC: [opval, opval, opval]
  CST_CODE_CE_CMP = 17,          // CE_CMP:        [opty, opval, opval, pred]
  CST_CODE_INLINEASM_OLD = 18,   // INLINEASM:     [sideeffect|alignstack,
                                 //                 asmstr,conststr]
  CST_CODE_CE_SHUFVEC_EX = 19,   // SHUFVEC_EX:    [opty, opval, opval, opval]
  CST_CODE_CE_INBOUNDS_GEP = 20, // INBOUNDS_GEP:  [n x operands]
  CST_CODE_BLOCKADDRESS = 21,    // CST_CODE_BLOCKADDRESS [fnty, fnval, bb#]
  CST_CODE_DATA = 22,            // DATA:          [n x elements]
  CST_CODE_INLINEASM = 23,       // INLINEASM:     [sideeffect|alignstack|
                                 //                 asmdialect,asmstr,conststr]
  CST_CODE_CE_GEP_WITH_INRANGE_INDEX = 24, //      [opty, flags, n x operands]
  CST_CODE_CE_UNOP = 25,         // CE_UNOP:      [opcode, opval]
};

/// CastOpcodes - These are values used in the bitcode files to encode which
/// cast a CST_CODE_CE_CAST or a XXX refers to.  The values of these enums
/// have no fixed relation to the LLVM IR enum values.  Changing these will
/// break compatibility with old files.
enum CastOpcodes {
  CAST_TRUNC = 0,
  CAST_ZEXT = 1,
  CAST_SEXT = 2,
  CAST_FPTOUI = 3,
  CAST_FPTOSI = 4,
  CAST_UITOFP = 5,
  CAST_SITOFP = 6,
  CAST_FPTRUNC = 7,
  CAST_FPEXT = 8,
  CAST_PTRTOINT = 9,
  CAST_INTTOPTR = 10,
  CAST_BITCAST = 11,
  CAST_ADDRSPACECAST = 12
};

/// UnaryOpcodes - These are values used in the bitcode files to encode which
/// unop a CST_CODE_CE_UNOP or a XXX refers to.  The values of these enums
/// have no fixed relation to the LLVM IR enum values.  Changing these will
/// break compatibility with old files.
enum UnaryOpcodes {
  UNOP_NEG = 0
};

/// BinaryOpcodes - These are values used in the bitcode files to encode which
/// binop a CST_CODE_CE_BINOP or a XXX refers to.  The values of these enums
/// have no fixed relation to the LLVM IR enum values.  Changing these will
/// break compatibility with old files.
enum BinaryOpcodes {
  BINOP_ADD = 0,
  BINOP_SUB = 1,
  BINOP_MUL = 2,
  BINOP_UDIV = 3,
  BINOP_SDIV = 4, // overloaded for FP
  BINOP_UREM = 5,
  BINOP_SREM = 6, // overloaded for FP
  BINOP_SHL = 7,
  BINOP_LSHR = 8,
  BINOP_ASHR = 9,
  BINOP_AND = 10,
  BINOP_OR = 11,
  BINOP_XOR = 12
};

/// These are values used in the bitcode files to encode AtomicRMW operations.
/// The values of these enums have no fixed relation to the LLVM IR enum
/// values.  Changing these will break compatibility with old files.
enum RMWOperations {
  RMW_XCHG = 0,
  RMW_ADD = 1,
  RMW_SUB = 2,
  RMW_AND = 3,
  RMW_NAND = 4,
  RMW_OR = 5,
  RMW_XOR = 6,
  RMW_MAX = 7,
  RMW_MIN = 8,
  RMW_UMAX = 9,
  RMW_UMIN = 10,
  RMW_FADD = 11,
  RMW_FSUB = 12
};

/// OverflowingBinaryOperatorOptionalFlags - Flags for serializing
/// OverflowingBinaryOperator's SubclassOptionalData contents.
enum OverflowingBinaryOperatorOptionalFlags {
  OBO_NO_UNSIGNED_WRAP = 0,
  OBO_NO_SIGNED_WRAP = 1
};

/// FastMath Flags
/// This is a fixed layout derived from the bitcode emitted by LLVM 5.0
/// intended to decouple the in-memory representation from the serialization.
enum FastMathMap {
  UnsafeAlgebra   = (1 << 0), // Legacy
  NoNaNs          = (1 << 1),
  NoInfs          = (1 << 2),
  NoSignedZeros   = (1 << 3),
  AllowReciprocal = (1 << 4),
  AllowContract   = (1 << 5),
  ApproxFunc      = (1 << 6),
  AllowReassoc    = (1 << 7)
};

/// PossiblyExactOperatorOptionalFlags - Flags for serializing
/// PossiblyExactOperator's SubclassOptionalData contents.
enum PossiblyExactOperatorOptionalFlags { PEO_EXACT = 0 };

/// Encoded AtomicOrdering values.
enum AtomicOrderingCodes {
  ORDERING_NOTATOMIC = 0,
  ORDERING_UNORDERED = 1,
  ORDERING_MONOTONIC = 2,
  ORDERING_ACQUIRE = 3,
  ORDERING_RELEASE = 4,
  ORDERING_ACQREL = 5,
  ORDERING_SEQCST = 6
};

/// Markers and flags for call instruction.
enum CallMarkersFlags {
  CALL_TAIL = 0,
  CALL_CCONV = 1,
  CALL_MUSTTAIL = 14,
  CALL_EXPLICIT_TYPE = 15,
  CALL_NOTAIL = 16,
  CALL_FMF = 17 // Call has optional fast-math-flags.
};

// The function body block (FUNCTION_BLOCK_ID) describes function bodies.  It
// can contain a constant block (CONSTANTS_BLOCK_ID).
enum FunctionCodes {
  FUNC_CODE_DECLAREBLOCKS = 1, // DECLAREBLOCKS: [n]

  FUNC_CODE_INST_BINOP = 2,      // BINOP:      [opcode, ty, opval, opval]
  FUNC_CODE_INST_CAST = 3,       // CAST:       [opcode, ty, opty, opval]
  FUNC_CODE_INST_GEP_OLD = 4,    // GEP:        [n x operands]
  FUNC_CODE_INST_SELECT = 5,     // SELECT:     [ty, opval, opval, opval]
  FUNC_CODE_INST_EXTRACTELT = 6, // EXTRACTELT: [opty, opval, opval]
  FUNC_CODE_INST_INSERTELT = 7,  // INSERTELT:  [ty, opval, opval, opval]
  FUNC_CODE_INST_SHUFFLEVEC = 8, // SHUFFLEVEC: [ty, opval, opval, opval]
  FUNC_CODE_INST_CMP = 9,        // CMP:        [opty, opval, opval, pred]

  FUNC_CODE_INST_RET = 10,    // RET:        [opty,opval<both optional>]
  FUNC_CODE_INST_BR = 11,     // BR:         [bb#, bb#, cond] or [bb#]
  FUNC_CODE_INST_SWITCH = 12, // SWITCH:     [opty, op0, op1, ...]
  FUNC_CODE_INST_INVOKE = 13, // INVOKE:     [attr, fnty, op0,op1, ...]
  // 14 is unused.
  FUNC_CODE_INST_UNREACHABLE = 15, // UNREACHABLE

  FUNC_CODE_INST_PHI = 16, // PHI:        [ty, val0,bb0, ...]
  // 17 is unused.
  // 18 is unused.
  FUNC_CODE_INST_ALLOCA = 19, // ALLOCA:     [instty, opty, op, align]
  FUNC_CODE_INST_LOAD = 20,   // LOAD:       [opty, op, align, vol]
  // 21 is unused.
  // 22 is unused.
  FUNC_CODE_INST_VAARG = 23, // VAARG:      [valistty, valist, instty]
  // This store code encodes the pointer type, rather than the value type
  // this is so information only available in the pointer type (e.g. address
  // spaces) is retained.
  FUNC_CODE_INST_STORE_OLD = 24, // STORE:      [ptrty,ptr,val, align, vol]
  // 25 is unused.
  FUNC_CODE_INST_EXTRACTVAL = 26, // EXTRACTVAL: [n x operands]
  FUNC_CODE_INST_INSERTVAL = 27,  // INSERTVAL:  [n x operands]
  // fcmp/icmp returning Int1TY or vector of Int1Ty. Same as CMP, exists to
  // support legacy vicmp/vfcmp instructions.
  FUNC_CODE_INST_CMP2 = 28, // CMP2:       [opty, opval, opval, pred]
  // new select on i1 or [N x i1]
  FUNC_CODE_INST_VSELECT = 29, // VSELECT:    [ty,opval,opval,predty,pred]
  FUNC_CODE_INST_INBOUNDS_GEP_OLD = 30, // INBOUNDS_GEP: [n x operands]
  FUNC_CODE_INST_INDIRECTBR = 31,       // INDIRECTBR: [opty, op0, op1, ...]
  // 32 is unused.
  FUNC_CODE_DEBUG_LOC_AGAIN = 33, // DEBUG_LOC_AGAIN

  FUNC_CODE_INST_CALL = 34, // CALL:    [attr, cc, fnty, fnid, args...]

  FUNC_CODE_DEBUG_LOC = 35,        // DEBUG_LOC:  [Line,Col,ScopeVal, IAVal]
  FUNC_CODE_INST_FENCE = 36,       // FENCE: [ordering, synchscope]
  FUNC_CODE_INST_CMPXCHG_OLD = 37, // CMPXCHG: [ptrty,ptr,cmp,new, align, vol,
                                   //           ordering, synchscope]
  FUNC_CODE_INST_ATOMICRMW = 38,   // ATOMICRMW: [ptrty,ptr,val, operation,
                                   //             align, vol,
                                   //             ordering, synchscope]
  FUNC_CODE_INST_RESUME = 39,      // RESUME:     [opval]
  FUNC_CODE_INST_LANDINGPAD_OLD =
      40,                         // LANDINGPAD: [ty,val,val,num,id0,val0...]
  FUNC_CODE_INST_LOADATOMIC = 41, // LOAD: [opty, op, align, vol,
                                  //        ordering, synchscope]
  FUNC_CODE_INST_STOREATOMIC_OLD = 42, // STORE: [ptrty,ptr,val, align, vol
                                       //         ordering, synchscope]
  FUNC_CODE_INST_GEP = 43,             // GEP:  [inbounds, n x operands]
  FUNC_CODE_INST_STORE = 44,       // STORE: [ptrty,ptr,valty,val, align, vol]
  FUNC_CODE_INST_STOREATOMIC = 45, // STORE: [ptrty,ptr,val, align, vol
  FUNC_CODE_INST_CMPXCHG = 46,     // CMPXCHG: [ptrty,ptr,valty,cmp,new, align,
                                   //           vol,ordering,synchscope]
  FUNC_CODE_INST_LANDINGPAD = 47,  // LANDINGPAD: [ty,val,num,id0,val0...]
  FUNC_CODE_INST_CLEANUPRET = 48,  // CLEANUPRET: [val] or [val,bb#]
  FUNC_CODE_INST_CATCHRET = 49,    // CATCHRET: [val,bb#]
  FUNC_CODE_INST_CATCHPAD = 50,    // CATCHPAD: [bb#,bb#,num,args...]
  FUNC_CODE_INST_CLEANUPPAD = 51,  // CLEANUPPAD: [num,args...]
  FUNC_CODE_INST_CATCHSWITCH =
      52, // CATCHSWITCH: [num,args...] or [num,args...,bb]
  // 53 is unused.
  // 54 is unused.
  FUNC_CODE_OPERAND_BUNDLE = 55, // OPERAND_BUNDLE: [tag#, value...]
  FUNC_CODE_INST_UNOP = 56,      // UNOP:       [opcode, ty, opval]
  FUNC_CODE_INST_CALLBR = 57,    // CALLBR:     [attr, cc, norm, transfs,
                                 //              fnty, fnid, args...]
};

enum UseListCodes {
  USELIST_CODE_DEFAULT = 1, // DEFAULT: [index..., value-id]
  USELIST_CODE_BB = 2       // BB: [index..., bb-id]
};

enum AttributeKindCodes {
  // = 0 is unused
  ATTR_KIND_ALIGNMENT = 1,
  ATTR_KIND_ALWAYS_INLINE = 2,
  ATTR_KIND_BY_VAL = 3,
  ATTR_KIND_INLINE_HINT = 4,
  ATTR_KIND_IN_REG = 5,
  ATTR_KIND_MIN_SIZE = 6,
  ATTR_KIND_NAKED = 7,
  ATTR_KIND_NEST = 8,
  ATTR_KIND_NO_ALIAS = 9,
  ATTR_KIND_NO_BUILTIN = 10,
  ATTR_KIND_NO_CAPTURE = 11,
  ATTR_KIND_NO_DUPLICATE = 12,
  ATTR_KIND_NO_IMPLICIT_FLOAT = 13,
  ATTR_KIND_NO_INLINE = 14,
  ATTR_KIND_NON_LAZY_BIND = 15,
  ATTR_KIND_NO_RED_ZONE = 16,
  ATTR_KIND_NO_RETURN = 17,
  ATTR_KIND_NO_UNWIND = 18,
  ATTR_KIND_OPTIMIZE_FOR_SIZE = 19,
  ATTR_KIND_READ_NONE = 20,
  ATTR_KIND_READ_ONLY = 21,
  ATTR_KIND_RETURNED = 22,
  ATTR_KIND_RETURNS_TWICE = 23,
  ATTR_KIND_S_EXT = 24,
  ATTR_KIND_STACK_ALIGNMENT = 25,
  ATTR_KIND_STACK_PROTECT = 26,
  ATTR_KIND_STACK_PROTECT_REQ = 27,
  ATTR_KIND_STACK_PROTECT_STRONG = 28,
  ATTR_KIND_STRUCT_RET = 29,
  ATTR_KIND_SANITIZE_ADDRESS = 30,
  ATTR_KIND_SANITIZE_THREAD = 31,
  ATTR_KIND_SANITIZE_MEMORY = 32,
  ATTR_KIND_UW_TABLE = 33,
  ATTR_KIND_Z_EXT = 34,
  ATTR_KIND_BUILTIN = 35,
  ATTR_KIND_COLD = 36,
  ATTR_KIND_OPTIMIZE_NONE = 37,
  ATTR_KIND_IN_ALLOCA = 38,
  ATTR_KIND_NON_NULL = 39,
  ATTR_KIND_JUMP_TABLE = 40,
  ATTR_KIND_DEREFERENCEABLE = 41,
  ATTR_KIND_DEREFERENCEABLE_OR_NULL = 42,
  ATTR_KIND_CONVERGENT = 43,
  ATTR_KIND_SAFESTACK = 44,
  ATTR_KIND_ARGMEMONLY = 45,
  ATTR_KIND_SWIFT_SELF = 46,
  ATTR_KIND_SWIFT_ERROR = 47,
  ATTR_KIND_NO_RECURSE = 48,
  ATTR_KIND_INACCESSIBLEMEM_ONLY = 49,
  ATTR_KIND_INACCESSIBLEMEM_OR_ARGMEMONLY = 50,
  ATTR_KIND_ALLOC_SIZE = 51,
  ATTR_KIND_WRITEONLY = 52,
  ATTR_KIND_SPECULATABLE = 53,
  ATTR_KIND_STRICT_FP = 54,
  ATTR_KIND_SANITIZE_HWADDRESS = 55,
  ATTR_KIND_NOCF_CHECK = 56,
  ATTR_KIND_OPT_FOR_FUZZING = 57,
  ATTR_KIND_SHADOWCALLSTACK = 58,
  ATTR_KIND_SPECULATIVE_LOAD_HARDENING = 59,
  ATTR_KIND_IMMARG = 60,
  ATTR_KIND_WILLRETURN = 61,
  ATTR_KIND_NOFREE = 62,
  ATTR_KIND_NOSYNC = 63,
  ATTR_KIND_SANITIZE_MEMTAG = 64,
  ATTR_KIND_MPPANATIVE = 65,
};

enum ComdatSelectionKindCodes {
  COMDAT_SELECTION_KIND_ANY = 1,
  COMDAT_SELECTION_KIND_EXACT_MATCH = 2,
  COMDAT_SELECTION_KIND_LARGEST = 3,
  COMDAT_SELECTION_KIND_NO_DUPLICATES = 4,
  COMDAT_SELECTION_KIND_SAME_SIZE = 5,
};

enum StrtabCodes {
  STRTAB_BLOB = 1,
};

enum SymtabCodes {
  SYMTAB_BLOB = 1,
};

} // End bitc namespace
} // End llvm namespace

#endif
