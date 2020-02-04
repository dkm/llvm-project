//===-- K1CISelLowering.cpp - K1C DAG Lowering Implementation  ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the K1CTargetLowering class.
//
//===----------------------------------------------------------------------===//

#include "K1CISelLowering.h"
#include "K1C.h"
#include "K1CMachineFunctionInfo.h"
#include "K1CTargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "K1CISelLowering"

STATISTIC(NumTailCalls, "Number of tail calls");

#include "K1CGenCallingConv.inc"

static bool CC_SRET_K1C(unsigned ValNo, MVT ValVT, MVT LocVT,
                        CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                        CCState &State) {
  if (ArgFlags.isSRet()) {
    if (unsigned Reg = State.AllocateReg(K1C::R15)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
    return true;
  }

  return CC_K1C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State);
}

K1CTargetLowering::K1CTargetLowering(const TargetMachine &TM,
                                     const K1CSubtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {

  (void)Subtarget;
  // set up the register classes
  addRegisterClass(MVT::i32, &K1C::SingleRegRegClass);
  addRegisterClass(MVT::i64, &K1C::SingleRegRegClass);
  addRegisterClass(MVT::v8i8, &K1C::SingleRegRegClass);
  addRegisterClass(MVT::v2i16, &K1C::SingleRegRegClass);
  addRegisterClass(MVT::v2i32, &K1C::SingleRegRegClass);
  addRegisterClass(MVT::v4i16, &K1C::SingleRegRegClass);
  addRegisterClass(MVT::v2i16, &K1C::SingleRegRegClass);
  addRegisterClass(MVT::v2i64, &K1C::PairedRegRegClass);
  addRegisterClass(MVT::v4i32, &K1C::PairedRegRegClass);

  addRegisterClass(MVT::f16, &K1C::SingleRegRegClass);
  addRegisterClass(MVT::f32, &K1C::SingleRegRegClass);
  addRegisterClass(MVT::f64, &K1C::SingleRegRegClass);
  addRegisterClass(MVT::v4f16, &K1C::SingleRegRegClass);
  addRegisterClass(MVT::v2f16, &K1C::SingleRegRegClass);
  addRegisterClass(MVT::v2f32, &K1C::SingleRegRegClass);
  addRegisterClass(MVT::v4f32, &K1C::PairedRegRegClass);
  addRegisterClass(MVT::v2f64, &K1C::PairedRegRegClass);

  // Compute derived properties from the register classes
  computeRegisterProperties(STI.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(getSPReg());

  setSchedulingPreference(Sched::Source);

  setOperationAction(ISD::SDIV, MVT::i32, Promote);
  setOperationAction(ISD::SDIVREM, MVT::i32, Promote);
  setOperationAction(ISD::SREM, MVT::i32, Promote);
  setOperationAction(ISD::UDIV, MVT::i32, Promote);
  setOperationAction(ISD::UDIVREM, MVT::i32, Promote);
  setOperationAction(ISD::UREM, MVT::i32, Promote);

  setOperationAction(ISD::SDIV, MVT::i64, Expand);
  setOperationAction(ISD::SDIVREM, MVT::i64, Expand);
  setOperationAction(ISD::SREM, MVT::i64, Expand);
  setOperationAction(ISD::UDIV, MVT::i64, Expand);
  setOperationAction(ISD::UDIVREM, MVT::i64, Expand);
  setOperationAction(ISD::UREM, MVT::i64, Expand);

  setOperationAction(ISD::MULHU, MVT::v4i16, Custom);
  setOperationAction(ISD::MULHS, MVT::v4i16, Custom);
  setOperationAction(ISD::MULHU, MVT::v2i16, Custom);
  setOperationAction(ISD::MULHS, MVT::v2i16, Custom);
  setOperationAction(ISD::MULHU, MVT::v8i8, Custom);
  setOperationAction(ISD::MULHS, MVT::v8i8, Custom);

  setLoadExtAction(ISD::ZEXTLOAD, MVT::v2i16, MVT::v2i8, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v2i16, MVT::v2i8, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v2i16, MVT::v2i8, Expand);

  setLoadExtAction(ISD::ZEXTLOAD, MVT::v2i32, MVT::v2i16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v2i32, MVT::v2i16, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v2i32, MVT::v2i16, Expand);

  setLoadExtAction(ISD::ZEXTLOAD, MVT::v2i32, MVT::v2i8, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v2i32, MVT::v2i8, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v2i32, MVT::v2i8, Expand);

  setLoadExtAction(ISD::ZEXTLOAD, MVT::v4i16, MVT::v4i8, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v4i16, MVT::v4i8, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v4i16, MVT::v4i8, Expand);

  setTruncStoreAction(MVT::v2i32, MVT::v2i16, Expand);
  setTruncStoreAction(MVT::v2i32, MVT::v2i8, Expand);
  setTruncStoreAction(MVT::v4i16, MVT::v4i8, Expand);
  setTruncStoreAction(MVT::v4i32, MVT::v4i16, Expand);
  setTruncStoreAction(MVT::v4i32, MVT::v4i8, Expand);

  setOperationAction(ISD::TRUNCATE, MVT::v2i16, Expand);
  setOperationAction(ISD::TRUNCATE, MVT::v2i32, Expand);
  setOperationAction(ISD::TRUNCATE, MVT::v4i16, Expand);
  setOperationAction(ISD::TRUNCATE, MVT::v4i32, Expand);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v2i16, Expand);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v2f16, Expand);

  setOperationAction(ISD::AND, MVT::v8i8, Expand);
  setOperationAction(ISD::OR, MVT::v8i8, Expand);
  setOperationAction(ISD::XOR, MVT::v8i8, Expand);

  setOperationAction(ISD::SIGN_EXTEND, MVT::v2i64, Expand);
  setOperationAction(ISD::ANY_EXTEND, MVT::v2i64, Expand);

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v4i32, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v4i16, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v2i16, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v2i32, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v2i64, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v4i16, Expand);

  for (auto VT : {MVT::v2i32, MVT::v4i16, MVT::v2i16, MVT::v2i64, MVT::v4i32,
                  MVT::v8i8, MVT::v4f32, MVT::v2f64}) {
    setOperationAction(ISD::UDIV, VT, Expand);
    setOperationAction(ISD::SDIV, VT, Expand);
    setOperationAction(ISD::VECTOR_SHUFFLE, VT, Expand);
    setOperationAction(ISD::SCALAR_TO_VECTOR, VT, Expand);

    setOperationAction(ISD::SETCC, VT, Expand);

    setOperationAction(ISD::SDIV, VT, Expand);
    setOperationAction(ISD::SDIVREM, VT, Expand);
    setOperationAction(ISD::SREM, VT, Expand);
    setOperationAction(ISD::UDIV, VT, Expand);
    setOperationAction(ISD::UDIVREM, VT, Expand);
    setOperationAction(ISD::UREM, VT, Expand);
    setOperationAction(ISD::SHL, VT, Expand);
    setOperationAction(ISD::SRL, VT, Expand);
    setOperationAction(ISD::SRA, VT, Expand);
  }

  for (auto VT : {MVT::v2i16, MVT::v4i16, MVT::v2i32, MVT::v4i32, MVT::v8i8,
                  MVT::v2f16, MVT::v4f16, MVT::v2f32, MVT::v4f32}) {
    setOperationAction(ISD::BUILD_VECTOR, VT, Custom);
  }

  for (auto VT :
       {MVT::v2i16, MVT::v4i16, MVT::v2i32, MVT::v2i64, MVT::v4i32, MVT::v8i8,
        MVT::v2f16, MVT::v4f16, MVT::v2f32, MVT::v4f32, MVT::v2f64}) {
    setOperationAction(ISD::INSERT_VECTOR_ELT, VT, Custom);
    setOperationAction(ISD::EXTRACT_VECTOR_ELT, VT, Custom);
    setOperationAction(ISD::VSELECT, VT, Expand);
    setOperationAction(ISD::CONCAT_VECTORS, VT, Custom);
  }

  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v2f32, Expand);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v2i32, Expand);

  setOperationAction(ISD::MULHU, MVT::v2i32, Expand);
  setOperationAction(ISD::MULHU, MVT::v2i64, Expand);
  setOperationAction(ISD::MULHS, MVT::v2i32, Expand);
  setOperationAction(ISD::MULHS, MVT::v2i64, Expand);
  setOperationAction(ISD::MULHU, MVT::v4i32, Expand);
  setOperationAction(ISD::MULHS, MVT::v4i32, Expand);

  for (auto VT : {MVT::v2f16, MVT::v2f32, MVT::v4f16, MVT::v4f32, MVT::v2f64,
                  MVT::v2i64}) {
    setOperationAction(ISD::FDIV, VT, Expand);
    setOperationAction(ISD::VECTOR_SHUFFLE, VT, Expand);
    setOperationAction(ISD::SCALAR_TO_VECTOR, VT, Expand);
  }
  setOperationAction(ISD::FMUL, MVT::v2f64, Expand);

  setOperationAction(ISD::AND, MVT::v2i64, Expand);
  setOperationAction(ISD::ADD, MVT::v2i64, Expand);
  setOperationAction(ISD::SUB, MVT::v2i64, Expand);
  setOperationAction(ISD::MUL, MVT::v2i64, Expand);
  setOperationAction(ISD::OR, MVT::v2i64, Expand);
  setOperationAction(ISD::AND, MVT::v2i64, Expand);
  setOperationAction(ISD::XOR, MVT::v2i64, Expand);

  setOperationAction(ISD::ADD, MVT::v4i32, Expand);
  setOperationAction(ISD::SUB, MVT::v4i32, Expand);
  setOperationAction(ISD::AND, MVT::v4i32, Expand);
  setOperationAction(ISD::OR, MVT::v4i32, Expand);
  setOperationAction(ISD::XOR, MVT::v4i32, Expand);

  setOperationAction(ISD::SMUL_LOHI, MVT::v2i64, Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::v2i64, Expand);
  setOperationAction(ISD::MUL, MVT::v8i8, Expand);

  for (auto VT : { MVT::i32, MVT::i64 }) {
    setOperationAction(ISD::SMUL_LOHI, VT, Expand);
    setOperationAction(ISD::UMUL_LOHI, VT, Expand);
    setOperationAction(ISD::MULHS, VT, Expand);
    setOperationAction(ISD::MULHU, VT, Expand);

    setOperationAction(ISD::SELECT_CC, VT, Expand);
    setOperationAction(ISD::SELECT, VT, Custom);

    setOperationAction(ISD::BR_CC, VT, Expand);

    setOperationAction(ISD::BSWAP, VT, Expand);

    setOperationAction(ISD::CTPOP, VT, Expand);
  }
  setOperationAction(ISD::ROTL, MVT::i64, Expand);
  setOperationAction(ISD::ROTR, MVT::i64, Expand);

  setOperationAction(ISD::BlockAddress, MVT::i64, Custom);
  setOperationAction(ISD::GlobalAddress, MVT::i64, Custom);
  setOperationAction(ISD::GlobalTLSAddress, MVT::i64, Custom);
  setOperationAction(ISD::VASTART, MVT::Other, Custom);
  setOperationAction(ISD::VAARG, MVT::Other, Custom);
  setOperationAction(ISD::VACOPY, MVT::Other, Expand);
  setOperationAction(ISD::VAEND, MVT::Other, Expand);

  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i64, Expand);

  setOperationAction(ISD::BR_JT, MVT::Other, Expand);

  for (unsigned im = (unsigned)ISD::PRE_INC;
       im != (unsigned)ISD::LAST_INDEXED_MODE; ++im) {
    setIndexedLoadAction(im, MVT::i32, Legal);
    setIndexedStoreAction(im, MVT::i32, Legal);
  }

  for (auto VT : { MVT::f16, MVT::f32, MVT::f64 }) {
    setOperationAction(ISD::ConstantFP, VT, Legal);

    setOperationAction(ISD::FABS, VT, Legal);
    setOperationAction(ISD::BR_CC, VT, Expand);

    setOperationAction(ISD::FSUB, VT, Custom);

    setOperationAction(ISD::FDIV, VT, VT == MVT::f16 ? Promote : Expand);
    setOperationAction(ISD::FREM, VT, VT == MVT::f16 ? Promote : Expand);
    setOperationAction(ISD::FSQRT, VT, VT == MVT::f16 ? Promote : Expand);
    setOperationAction(ISD::FSIN, VT, VT == MVT::f16 ? Promote : Expand);
    setOperationAction(ISD::FCOS, VT, VT == MVT::f16 ? Promote : Expand);

    setOperationAction(ISD::SELECT_CC, VT, Expand);
    setOperationAction(ISD::SELECT, VT, Custom);
  }

  for (auto VT : {MVT::v2f64, MVT::v2i64, MVT::v4i32, MVT::v2i32, MVT::v2i16,
                  MVT::v4i16, MVT::v8i8}) {
    setOperationAction(ISD::SELECT_CC, VT, Expand);
    setOperationAction(ISD::SELECT, VT, Expand);
  }

  for (auto VT : {MVT::v2i16, MVT::v2i32, MVT::v2i64, MVT::v4i16, MVT::v4i32}) {
    setOperationAction(ISD::FP_TO_SINT, VT, Expand);
    setOperationAction(ISD::FP_TO_UINT, VT, Expand);
    setOperationAction(ISD::SINT_TO_FP, VT, Expand);
    setOperationAction(ISD::UINT_TO_FP, VT, Expand);
  }

  setOperationAction(ISD::FP_ROUND, MVT::v4f16, Expand);
  setOperationAction(ISD::FP_ROUND, MVT::v2f16, Expand);
  setOperationAction(ISD::FP_ROUND, MVT::v2f32, Expand);
  setOperationAction(ISD::FP_EXTEND, MVT::v2f32, Expand);
  setOperationAction(ISD::FP_EXTEND, MVT::v4f32, Expand);
  setOperationAction(ISD::FP_EXTEND, MVT::v2f64, Expand);

  setOperationAction(ISD::FNEG, MVT::v2f64, Expand);
  setOperationAction(ISD::FNEG, MVT::v4f32, Expand);

  setTruncStoreAction(MVT::v2i16, MVT::v2i8, Expand);
  setTruncStoreAction(MVT::v2i64, MVT::v2i8, Expand);
  setTruncStoreAction(MVT::v2i64, MVT::v2i16, Expand);
  setTruncStoreAction(MVT::v2f32, MVT::v2f16, Expand);
  setTruncStoreAction(MVT::v2i64, MVT::v2i32, Expand);
  setTruncStoreAction(MVT::v2f64, MVT::v2f16, Expand);
  setTruncStoreAction(MVT::v2f64, MVT::v2f32, Expand);
  setTruncStoreAction(MVT::v4f32, MVT::v4f16, Expand);

  setOperationAction(ISD::SIGN_EXTEND, MVT::v2i32, Expand);
  setOperationAction(ISD::ZERO_EXTEND, MVT::v2i32, Expand);
  setOperationAction(ISD::ANY_EXTEND, MVT::v2i32, Expand);

  setOperationAction(ISD::SIGN_EXTEND, MVT::v2i64, Expand);
  setOperationAction(ISD::ZERO_EXTEND, MVT::v2i64, Expand);
  setOperationAction(ISD::ANY_EXTEND, MVT::v2i64, Expand);

  setOperationAction(ISD::SIGN_EXTEND, MVT::v4i32, Expand);
  setOperationAction(ISD::ZERO_EXTEND, MVT::v4i32, Expand);
  setOperationAction(ISD::ANY_EXTEND, MVT::v4i32, Expand);

  setLoadExtAction(ISD::SEXTLOAD, MVT::v2i64, MVT::v2i8, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v2i64, MVT::v2i8, Expand);

  setLoadExtAction(ISD::SEXTLOAD, MVT::v4i32, MVT::v4i8, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v4i32, MVT::v4i8, Expand);

  setLoadExtAction(ISD::EXTLOAD, MVT::v2f32, MVT::v2f16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v2f64, MVT::v2f16, Expand);

  setLoadExtAction(ISD::EXTLOAD, MVT::v4f32, MVT::v4f16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v2f64, MVT::v2f32, Expand);

  for (MVT VT : MVT::fp_valuetypes()) {
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::f16, Expand);
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::f32, Expand);
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::f64, Expand);
  }

  setMaxAtomicSizeInBitsSupported(64);
  setMinCmpXchgSizeInBits(32);

  // Effectively disable jump table generation.
  setMinimumJumpTableEntries(INT_MAX);
}

EVT K1CTargetLowering::getSetCCResultType(const DataLayout &DL, LLVMContext &C,
                                          EVT VT) const {
  if (!VT.isVector())
    return MVT::i32;
  return EVT::getVectorVT(C, MVT::i32, VT.getVectorNumElements());
}

const char *K1CTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case K1CISD::RET:
    return "K1C::RET";
  case K1CISD::CALL:
    return "K1C::CALL";
  case K1CISD::WRAPPER:
    return "K1C::WRAPPER";
  case K1CISD::SELECT_CC:
    return "K1C::SELECT_CC";
  case K1CISD::PICInternIndirection:
    return "K1C::PICInternIndirection";
  case K1CISD::PICExternIndirection:
    return "K1C::PICExternIndirection";
  case K1CISD::PICPCRelativeGOTAddr:
    return "K1C::PICPCRelativeGOTAddr";
  case K1CISD::TAIL:
    return "K1C::TAIL";
  case K1CISD::GetSystemReg:
    return "K1C::GetSystemReg";
  default:
    return NULL;
  }
}

bool K1CTargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &MF, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Context) const {

  return true;
}

SDValue
K1CTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &OutVals,
                               const SDLoc &DL, SelectionDAG &DAG) const {

  SmallVector<CCValAssign, 16> RVLocs;

  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeReturn(Outs, RetCC_K1C);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  for (unsigned int i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");
    SDValue Arg = OutVals[i];
    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), Arg, Flag);
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  return DAG.getNode(K1CISD::RET, DL, MVT::Other, RetOps);
}

static const MCPhysReg ArgGPRs[] = { K1C::R0, K1C::R1, K1C::R2,  K1C::R3,
                                     K1C::R4, K1C::R5, K1C::R6,  K1C::R7,
                                     K1C::R8, K1C::R9, K1C::R10, K1C::R11 };

SDValue K1CTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  switch (CallConv) {
  default:
    report_fatal_error("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
  case CallingConv::SPIR_KERNEL:
    break;
  }

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  std::vector<SDValue> OutChains;

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_SRET_K1C);

  unsigned MemArgsSaveSize = 0;

  for (auto &VA : ArgLocs) {
    if (VA.isRegLoc()) {
      EVT RegVT = VA.getLocVT();

      unsigned VReg;
      if (VA.getValVT() == MVT::v4f32 || VA.getValVT() == MVT::v2f64 ||
          VA.getValVT() == MVT::v2i64 || VA.getValVT() == MVT::v4i32)
        VReg = RegInfo.createVirtualRegister(&K1C::PairedRegRegClass);
      else
        VReg = RegInfo.createVirtualRegister(&K1C::SingleRegRegClass);
      RegInfo.addLiveIn(VA.getLocReg(), VReg);
      SDValue ArgIn = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);

      InVals.push_back(ArgIn);
      continue;
    }

    assert(VA.isMemLoc());

    unsigned Offset = VA.getLocMemOffset();
    unsigned StoreSize = VA.getValVT().getStoreSize();
    int FI = MF.getFrameInfo().CreateFixedObject(StoreSize, Offset, false);
    InVals.push_back(
        DAG.getLoad(VA.getValVT(), DL, Chain,
                    DAG.getFrameIndex(FI, getPointerTy(MF.getDataLayout())),
                    MachinePointerInfo::getFixedStack(MF, FI)));

    MemArgsSaveSize += StoreSize;
  }

  K1CMachineFunctionInfo *K1CFI = MF.getInfo<K1CMachineFunctionInfo>();

  if (IsVarArg) {
    ArrayRef<MCPhysReg> ArgRegs = makeArrayRef(ArgGPRs);
    unsigned Idx = CCInfo.getFirstUnallocated(ArgRegs);
    int VarArgsOffset = CCInfo.getNextStackOffset();
    int VarArgsSaveSize = 0;
    MachineFrameInfo &MFI = MF.getFrameInfo();
    const unsigned ArgRegsSize = ArgRegs.size();
    int FI;

    if (Idx >= ArgRegsSize) {
      FI = MFI.CreateFixedObject(8, VarArgsOffset, true);
      MemArgsSaveSize += 8;
    } else {
      FI = -(int)(MFI.getNumFixedObjects() + 1);
      VarArgsOffset = (Idx - ArgRegs.size()) * 8;
      VarArgsSaveSize = -VarArgsOffset;
    }

    K1CFI->setVarArgsFrameIndex(FI);
    K1CFI->setVarArgsSaveSize(VarArgsSaveSize);

    // Copy the integer registers that may have been used for passing varargs
    // to the vararg save area.
    for (unsigned I = Idx; I < ArgRegs.size(); ++I, VarArgsOffset += 8) {
      const unsigned Reg =
          RegInfo.createVirtualRegister(&K1C::SingleRegRegClass);
      RegInfo.addLiveIn(ArgRegs[I], Reg);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, MVT::i64);
      int FI = MFI.CreateFixedObject(8, VarArgsOffset, true);
      SDValue PtrOff = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
      SDValue Store = DAG.getStore(Chain, DL, ArgValue, PtrOff,
                                   MachinePointerInfo::getFixedStack(MF, FI));
      OutChains.push_back(Store);
    }
  }

  K1CFI->setMemArgsSaveSize(MemArgsSaveSize);

  // All stores are grouped in one node to allow the matching between
  // the size of Ins and InVals. This only happens for vararg functions.
  if (!OutChains.empty()) {
    OutChains.push_back(Chain);
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, OutChains);
  }

  return Chain;
}

SDValue K1CTargetLowering::LowerCall(CallLoweringInfo &CLI,
                                     SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  bool &IsTailCall = CLI.IsTailCall;
  CallingConv::ID CallConv = CLI.CallConv;
  bool isVarArg = CLI.IsVarArg;
  EVT PtrVT = getPointerTy(DAG.getDataLayout());
  MachineFunction &MF = DAG.getMachineFunction();
  K1CMachineFunctionInfo *FuncInfo = MF.getInfo<K1CMachineFunctionInfo>();

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeCallOperands(Outs, CC_SRET_K1C);

  if (IsTailCall)
    IsTailCall = IsEligibleForTailCallOptimization(CCInfo, CLI, MF, ArgLocs);

  if (IsTailCall)
    ++NumTailCalls;
  else if (CLI.CS && CLI.CS.isMustTailCall())
    report_fatal_error("failed to perform tail call elimination on a call"
                       "site marked musttail");

  // Get the size of the outgoing arguments stack space requirement.
  unsigned ArgsSize = CCInfo.getNextStackOffset();

  // Keep stack frames 8-byte aligned.
  ArgsSize = (ArgsSize + 7) & ~7;

  // Create local copies for byval args
  SmallVector<SDValue, 8> ByValArgs;
  for (unsigned i = 0, e = Outs.size(); i != e; ++i) {
    ISD::ArgFlagsTy Flags = Outs[i].Flags;
    if (!Flags.isByVal())
      continue;

    SDValue Arg = OutVals[i];
    unsigned Size = Flags.getByValSize();
    unsigned Align = Flags.getByValAlign();

    int FI = MF.getFrameInfo().CreateStackObject(Size, Align, /*isSS=*/false);
    SDValue FIPtr = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
    SDValue SizeNode = DAG.getConstant(Size, DL, MVT::i64);

    Chain = DAG.getMemcpy(Chain, DL, FIPtr, Arg, SizeNode, Align,
                          /*IsVolatile=*/false,
                          /*AlwaysInline=*/false, false /*IsTailCall*/,
                          MachinePointerInfo(), MachinePointerInfo());
    ByValArgs.push_back(FIPtr);
  }

  if (!IsTailCall)
    Chain = DAG.getCALLSEQ_START(Chain, ArgsSize, 0, DL);

  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;
  SDValue StackPtr;

  for (unsigned i = 0, j = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue ArgValue = OutVals[i];
    ISD::ArgFlagsTy Flags = Outs[i].Flags;

    if (Flags.isByVal())
      ArgValue = ByValArgs[j++];

    if (VA.isRegLoc()) {
      // Queue up the argument copies and emit them at the end.
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), ArgValue));
    } else {
      assert(VA.isMemLoc() && "Argument not register or memory");
      assert(!IsTailCall && "Tail call not allowed if stack is used "
                            "for passing parameters");

      // Work out the address of the stack slot.
      if (!StackPtr.getNode())
        StackPtr = DAG.getCopyFromReg(Chain, DL, getSPReg(), PtrVT);

      // Create a store off the stack pointer for this argument.
      SDValue PtrOff = DAG.getIntPtrConstant(VA.getLocMemOffset(), DL);
      PtrOff = DAG.getNode(ISD::ADD, DL, MVT::i64, StackPtr, PtrOff);
      MemOpChains.push_back(
          DAG.getStore(Chain, DL, ArgValue, PtrOff, MachinePointerInfo()));
    }
  }

  FuncInfo->setOutgoingArgsMaxSize(ArgsSize);

  // Join the stores, which are independent of one another.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

  SDValue Glue;

  // Build a sequence of copy-to-reg nodes, chained and glued together.
  for (auto &Reg : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, DL, Reg.first, Reg.second, Glue);
    Glue = Chain.getValue(1);
  }

  if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    Callee = DAG.getTargetExternalSymbol(S->getSymbol(), PtrVT, 0);
  }

  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);
  for (auto &Reg : RegsToPass)
    Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));

  if (!IsTailCall) {
    // Add a register mask operand representing the call-preserved registers.
    const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo();
    const uint32_t *Mask = TRI->getCallPreservedMask(MF, CallConv);
    assert(Mask && "Missing call preserved mask for calling convention");
    Ops.push_back(DAG.getRegisterMask(Mask));
  }

  // Glue the call to the argument copies, if any.
  if (Glue.getNode())
    Ops.push_back(Glue);

  // Emit the call.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);

  if (IsTailCall) {
    MF.getFrameInfo().setHasTailCall();
    return DAG.getNode(K1CISD::TAIL, DL, NodeTys, Ops);
  }

  Chain = DAG.getNode(K1CISD::CALL, DL, NodeTys, Ops);
  Glue = Chain.getValue(1);

  // Mark the end of the call, which is glued to the call itself.
  Chain = DAG.getCALLSEQ_END(Chain, DAG.getConstant(ArgsSize, DL, PtrVT, true),
                             DAG.getConstant(0, DL, PtrVT, true), Glue, DL);
  Glue = Chain.getValue(1);

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState RetCCInfo(CallConv, CLI.IsVarArg, MF, RVLocs, *DAG.getContext());
  RetCCInfo.AnalyzeCallResult(Ins, RetCC_K1C);
  // analyzeInputArgs(MF, RetCCInfo, Ins, /*IsRet=*/true);

  // Copy all of the result registers out of their specified physreg.
  for (auto &VA : RVLocs) {
    // Copy the value out
    SDValue RetValue =
        DAG.getCopyFromReg(Chain, DL, VA.getLocReg(), VA.getLocVT(), Glue);
    // Glue the RetValue to the end of the call sequence
    Chain = RetValue.getValue(1);
    Glue = RetValue.getValue(2);

    InVals.push_back(RetValue);
  }

  return Chain;
}

SDValue K1CTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  default:
    report_fatal_error("unimplemented operand");
  case ISD::RETURNADDR:
    return lowerRETURNADDR(Op, DAG);
  case ISD::GlobalAddress:
    return lowerGlobalAddress(Op, DAG);
  case ISD::GlobalTLSAddress:
    return lowerGlobalTLSAddress(Op, DAG);
  case ISD::VASTART:
    return lowerVASTART(Op, DAG);
  case ISD::VAARG:
    return lowerVAARG(Op, DAG);
  case ISD::FRAMEADDR:
    return lowerFRAMEADDR(Op, DAG);
  case ISD::FSUB:
    return lowerFSUB(Op, DAG);
  case ISD::SELECT:
    return lowerSELECT(Op, DAG);
  case ISD::MULHS:
    return lowerMULHVectorGeneric(Op, DAG, true);
  case ISD::MULHU:
    return lowerMULHVectorGeneric(Op, DAG, false);
  case ISD::BlockAddress:
    return lowerBlockAddress(Op, DAG);
  case ISD::BUILD_VECTOR:
    return lowerBUILD_VECTOR(Op, DAG);
  case ISD::INSERT_VECTOR_ELT:
    return lowerINSERT_VECTOR_ELT(Op, DAG);
  case ISD::EXTRACT_VECTOR_ELT:
    return lowerEXTRACT_VECTOR_ELT(Op, DAG);
  case ISD::CONCAT_VECTORS:
    return lowerCONCAT_VECTORS(Op, DAG);
  }
}

Instruction *K1CTargetLowering::emitLeadingFence(IRBuilder<> &Builder,
                                                 Instruction *Inst,
                                                 AtomicOrdering Ord) const {
  if (isa<LoadInst>(Inst) && Ord == AtomicOrdering::SequentiallyConsistent)
    return Builder.CreateFence(Ord);
  if (isa<StoreInst>(Inst) && Ord != AtomicOrdering::Monotonic)
    return Builder.CreateFence(Ord);
  return nullptr;
}

Instruction *K1CTargetLowering::emitTrailingFence(IRBuilder<> &Builder,
                                                  Instruction *Inst,
                                                  AtomicOrdering Ord) const {
  if (isa<LoadInst>(Inst) && isAcquireOrStronger(Ord))
    return Builder.CreateFence(AtomicOrdering::Acquire);
  if (isa<StoreInst>(Inst) && Ord == AtomicOrdering::SequentiallyConsistent)
    return Builder.CreateFence(Ord);

  return nullptr;
}

SDValue K1CTargetLowering::lowerRETURNADDR(SDValue Op,
                                           SelectionDAG &DAG) const {
  auto VT = getPointerTy(DAG.getDataLayout());
  SDLoc DL(Op);

  // This node takes one operand, the depth of the return address to return. A
  // depth of zero corresponds to the current function's return address, a depth
  // of one to the parent's return address, and so on.
  unsigned Depth = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();
  if (Depth) {
    // Depth > 0 not supported yet
    report_fatal_error("Unsupported Depth for lowerRETURNADDR");
  }

  SDValue RAReg = DAG.getRegister(K1C::RA, VT);
  return DAG.getNode(K1CISD::GetSystemReg, DL, VT, RAReg);
}

SDValue K1CTargetLowering::lowerGlobalAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
  LLVM_DEBUG(dbgs() << "== K1CTargetLowering::lowerGlobalAddress(Op, DAG)"
                    << '\n');
  LLVM_DEBUG(dbgs() << "Op: " << '\n'; Op.dump());
  LLVM_DEBUG(dbgs() << "DAG: " << '\n'; DAG.dump());

  SDLoc DL(Op);
  auto PtrVT = getPointerTy(DAG.getDataLayout());
  GlobalAddressSDNode *N = cast<GlobalAddressSDNode>(Op);
  const GlobalValue *GV = N->getGlobal();
  int64_t Offset = N->getOffset();
  SDValue Result;

  // -fPIC enabled
  if (isPositionIndependent()) {
    LLVM_DEBUG(dbgs() << "LT: " << GV->getLinkage()
                      << " isPrivate: " << GV->hasPrivateLinkage() << '\n');

    GlobalValue::LinkageTypes LT = GV->getLinkage();
    switch (LT) {
    case GlobalValue::CommonLinkage:
    case GlobalValue::ExternalLinkage: {
      LLVM_DEBUG(dbgs() << "@got(sym)[gaddr]" << '\n');

      SDValue GlobalSymbol = DAG.getTargetGlobalAddress(GV, DL, PtrVT);
      SDValue GOTAddr = DAG.getNode(K1CISD::PICPCRelativeGOTAddr, DL, PtrVT);

      // Indirect global symbol using GOT with
      // @got(GLOBALSYMBOL)[GOTADDR] asm macro.  Note: function
      // symbols don't need indirection since everything is handled by
      // the loader. Consequently, call indirections are ignored at
      // insn selection.
      SDValue DataPtr = DAG.getNode(K1CISD::PICExternIndirection, DL, PtrVT,
                                    GOTAddr, GlobalSymbol);

      if (Offset != 0)
        Result = DAG.getNode(ISD::ADD, DL, PtrVT, DataPtr,
                             DAG.getConstant(Offset, DL, PtrVT));
      else
        Result = DataPtr;

    } break;
    case GlobalValue::InternalLinkage:
    case GlobalValue::PrivateLinkage: {
      LLVM_DEBUG(dbgs() << "@gotoff(sym)" << '\n');

      SDValue GlobalSymbol = DAG.getTargetGlobalAddress(GV, DL, PtrVT);
      SDValue GOTAddr = DAG.getNode(K1CISD::PICPCRelativeGOTAddr, DL, PtrVT);

      // Indirect global symbol using GOT with @gotoff(GLOBALSYMBOL)
      // asm macro.
      SDValue DataPtr = DAG.getNode(K1CISD::PICInternIndirection, DL, PtrVT,
                                    GOTAddr, GlobalSymbol);

      if (Offset != 0)
        Result = DAG.getNode(ISD::ADD, DL, PtrVT, DataPtr,
                             DAG.getConstant(Offset, DL, PtrVT));
      else
        Result = DataPtr;

    } break;
    default:
      llvm_unreachable("Unsupported LinkageType");
    }
  } else {
    Result = DAG.getTargetGlobalAddress(GV, DL, PtrVT, 0);
    Result = DAG.getNode(K1CISD::WRAPPER, DL, PtrVT, Result);

    if (Offset != 0) {
      SDValue PtrOff = DAG.getIntPtrConstant(Offset, DL);
      Result = DAG.getNode(ISD::ADD, DL, MVT::i64, Result, PtrOff);
    }
  }

  LLVM_DEBUG(dbgs() << "Result: " << '\n'; Result.dump());
  return Result;
}

SDValue K1CTargetLowering::lowerGlobalTLSAddress(SDValue Op,
                                                 SelectionDAG &DAG) const {

  MachineFunction &MF = DAG.getMachineFunction();
  GlobalAddressSDNode *N = cast<GlobalAddressSDNode>(Op);
  const GlobalValue *GV = N->getGlobal();
  TLSModel::Model model = DAG.getTarget().getTLSModel(GV);

  switch (model) {
  default:
    if (MF.getSubtarget().getTargetTriple().isOSClusterOS() ||
        MF.getSubtarget().getTargetTriple().isOSK1ELF())
      report_fatal_error(
          "ClusterOS and K1ELF only supports TLS model LocalExec");
    else
      report_fatal_error("Unknown TLSModel");
  case TLSModel::LocalExec:
    SDLoc DL(Op);
    int64_t Offset = N->getOffset();
    EVT PtrVT = getPointerTy(DAG.getDataLayout());

    SDValue Result = DAG.getTargetGlobalAddress(GV, DL, PtrVT, Offset);
    Result = DAG.getNode(K1CISD::WRAPPER, DL, PtrVT, Result);
    Result = DAG.getNode(ISD::ADD, DL, PtrVT,
                         DAG.getRegister(K1C::R13, MVT::i64), Result);
    return Result;
  }

  return SDValue();
}

SDValue K1CTargetLowering::lowerVASTART(SDValue Op, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  K1CMachineFunctionInfo *FuncInfo = MF.getInfo<K1CMachineFunctionInfo>();

  SDLoc DL(Op);
  SDValue FI = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(),
                                 getPointerTy(MF.getDataLayout()));

  // vastart just stores the address of the VarArgsFrameIndex slot into the
  // memory location argument.
  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
  return DAG.getStore(Op.getOperand(0), DL, FI, Op.getOperand(1),
                      MachinePointerInfo(SV));
}

SDValue K1CTargetLowering::lowerVAARG(SDValue Op, SelectionDAG &DAG) const {
  SDNode *Node = Op.getNode();
  EVT VT = Node->getValueType(0);
  SDValue InChain = Node->getOperand(0);
  SDValue VAListPtr = Node->getOperand(1);
  EVT PtrVT = VAListPtr.getValueType();
  const Value *SV = cast<SrcValueSDNode>(Node->getOperand(2))->getValue();
  SDLoc DL(Node);
  SDValue VAList =
      DAG.getLoad(PtrVT, DL, InChain, VAListPtr, MachinePointerInfo(SV));
  // Increment the pointer, VAList, to the next vaarg.
  // Increment va_list pointer to the next multiple of 64 bits / 8 bytes
  int increment = (VT.getSizeInBits() + 63) / 64 * 8;
  SDValue NextPtr = DAG.getNode(ISD::ADD, DL, PtrVT, VAList,
                                DAG.getIntPtrConstant(increment, DL));
  // Store the incremented VAList to the legalized pointer.
  InChain = DAG.getStore(VAList.getValue(1), DL, NextPtr, VAListPtr,
                         MachinePointerInfo(SV));
  // Load the actual argument out of the pointer VAList.
  // We can't count on greater alignment than the word size.
  return DAG.getLoad(VT, DL, InChain, VAList, MachinePointerInfo(), 8);
}

SDValue K1CTargetLowering::lowerFRAMEADDR(SDValue Op, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MFI.setFrameAddressIsTaken(true);
  unsigned FrameReg = getSPReg();
  int XLenInBytes = 8;

  EVT VT = Op.getValueType();
  SDLoc DL(Op);
  SDValue FrameAddr = DAG.getCopyFromReg(DAG.getEntryNode(), DL, FrameReg, VT);
  unsigned Depth = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();
  while (Depth--) {
    int Offset = -(XLenInBytes * 2);
    SDValue Ptr = DAG.getNode(ISD::ADD, DL, VT, FrameAddr,
                              DAG.getIntPtrConstant(Offset, DL));
    FrameAddr =
        DAG.getLoad(VT, DL, DAG.getEntryNode(), Ptr, MachinePointerInfo());
  }
  return FrameAddr;
}

SDValue K1CTargetLowering::lowerFSUB(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getSimpleValueType().SimpleTy) {
  default:
    return Op;
  case MVT::f16:
  case MVT::f32:
  case MVT::f64:
    if (auto *ConstFPNode = dyn_cast<ConstantFPSDNode>(Op.getOperand(0))) {
      uint64_t Val = ConstFPNode->getValueAPF().bitcastToAPInt().getZExtValue();

      if (Val == 0)
        return DAG.getNode(ISD::FNEG, SDLoc(Op), Op.getValueType(),
                           Op.getOperand(1));
    }

    return Op;
  }
}

SDValue K1CTargetLowering::lowerSELECT(SDValue Op, SelectionDAG &DAG) const {

  SDValue CondV = Op.getOperand(0);
  SDValue TrueV = Op.getOperand(1);
  SDValue FalseV = Op.getOperand(2);
  SDLoc DL(Op);

  SDVTList VTs = DAG.getVTList(Op.getValueType());
  SDValue Ops[] = {CondV, TrueV, FalseV};

  SDValue result = DAG.getNode(K1CISD::SELECT_CC, DL, VTs, Ops);

  return result;
}

bool K1CTargetLowering::IsEligibleForTailCallOptimization(
    CCState &CCInfo, CallLoweringInfo &CLI, MachineFunction &MF,
    const SmallVector<CCValAssign, 16> &ArgsLocs) const {
  auto &Callee = CLI.Callee;
  auto CalleeCC = CLI.CallConv;
  auto IsVarArg = CLI.IsVarArg;
  auto &Outs = CLI.Outs;
  auto &Caller = MF.getFunction();
  auto CallerCC = Caller.getCallingConv();

  // Do not tail call opt functions with "disable-tail-calls" attribute.
  if (Caller.getFnAttribute("disable-tail-calls").getValueAsString() == "true")
    return false;

  // Do not tail call opt functions with varargs, unless arguments are all
  // passed in registers.
  if (IsVarArg)
    for (unsigned Idx = 0, End = ArgsLocs.size(); Idx != End; ++Idx)
      if (!ArgsLocs[Idx].isRegLoc())
        return false;

  // Do not tail call opt if the stack is used to pass parameters.
  if (CCInfo.getNextStackOffset() != 0)
    return false;

  // Do not tail call opt if either caller or callee uses struct return
  // semantics.
  auto IsCallerStructRet = Caller.hasStructRetAttr();
  auto IsCalleeStructRet = Outs.empty() ? false : Outs[0].Flags.isSRet();
  if (IsCallerStructRet || IsCalleeStructRet)
    return false;

  // Externally-defined functions with weak linkage should not be
  // tail-called. The behaviour of branch instructions in this situation (as
  // used for tail calls) is implementation-defined, so we cannot rely on the
  // linker replacing the tail call with a return.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
    const GlobalValue *GV = G->getGlobal();
    if (GV->hasExternalWeakLinkage())
      return false;
  }

  // The callee has to preserve all registers the caller needs to preserve.
  if (CalleeCC != CallerCC) {
    const K1CRegisterInfo *TRI = Subtarget.getRegisterInfo();
    const uint32_t *CallerPreserved = TRI->getCallPreservedMask(MF, CallerCC);
    const uint32_t *CalleePreserved = TRI->getCallPreservedMask(MF, CalleeCC);
    if (!TRI->regmaskSubsetEqual(CallerPreserved, CalleePreserved))
      return false;
  }

  return true;
}

SDValue K1CTargetLowering::lowerBlockAddress(SDValue Op,
                                             SelectionDAG &DAG) const {
  BlockAddressSDNode *N = cast<BlockAddressSDNode>(Op);
  const BlockAddress *BA = N->getBlockAddress();
  int64_t Offset = N->getOffset();
  auto PtrVT = getPointerTy(DAG.getDataLayout());

  SDLoc DL(Op);

  SDValue Result = DAG.getTargetBlockAddress(BA, PtrVT, Offset);

  // -fPIC
  if (isPositionIndependent())
    Result = DAG.getNode(K1CISD::PICWRAPPER, DL, PtrVT, Result);
  else
    Result = DAG.getNode(K1CISD::WRAPPER, DL, PtrVT, Result);

  return Result;
}

#define GET_REGISTER_MATCHER
#include "K1CGenAsmMatcher.inc"

std::pair<unsigned, const TargetRegisterClass *>
K1CTargetLowering::getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                                StringRef Constraint,
                                                MVT VT) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'r':
      if (VT == MVT::i32 || VT == MVT::i64)
        return std::make_pair(0U, &K1C::SingleRegRegClass);
      llvm_unreachable("unsuported register type");
    default:
      break;
    }
  }
  if (Constraint.size() >= 4 && Constraint.front() == '{' &&
      Constraint.back() == '}') {
    StringRef regName = Constraint.substr(1, Constraint.size() - 2);
    unsigned RegNo = MatchRegisterName(regName);
    if (RegNo == 0) {
      RegNo = MatchRegisterAltName(regName);
    }
    if (K1C::SingleRegRegClass.contains(RegNo))
      return std::make_pair(RegNo, &K1C::SingleRegRegClass);
  }

  return TargetLowering::getRegForInlineAsmConstraint(TRI, Constraint, VT);
}

SDValue K1CTargetLowering::lowerMULHVectorGeneric(SDValue Op, SelectionDAG &DAG,
                                                  bool Signed) const {
  SDLoc DL(Op);

  ISD::NodeType ExtensionType = Signed ? ISD::SIGN_EXTEND : ISD::ZERO_EXTEND;

  EVT VectorType = Op.getValueType();
  unsigned VectorSize = VectorType.getVectorNumElements();
  const unsigned BitSize = VectorType.getScalarSizeInBits();
  EVT ScalarType = VectorType.getScalarType();

  SmallVector<SDValue, 8> RV;
  for (unsigned int i = 0; i < VectorSize; i++) {
    SDValue AVal =
        DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, ScalarType,
                    {Op.getOperand(0), DAG.getConstant(i, DL, MVT::i64)});
    SDValue AValExt = DAG.getNode(ExtensionType, DL, MVT::i32, AVal);
    SDValue BVal =
        DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, ScalarType,
                    {Op.getOperand(1), DAG.getConstant(i, DL, MVT::i64)});
    SDValue BValExt = DAG.getNode(ExtensionType, DL, MVT::i32, BVal);
    SDValue R =
        DAG.getNode(ISD::SRL, DL, MVT::i32,
                    DAG.getNode(ISD::MUL, DL, MVT::i32, {AValExt, BValExt}),
                    DAG.getConstant(BitSize, DL, MVT::i32));
    RV.push_back(R);
  }
  // Building a vector from the computed values
  return DAG.getBuildVector(VectorType, DL, RV);
}

static unsigned selectExtend(unsigned BitSize) {
  // i8 and i16 are not legal, explicit selection must be done
  switch (BitSize) {
  default:
    llvm_unreachable("Unknown extend for this type");
  case 8:
    return K1C::ZXBD;
  case 16:
    return K1C::ZXHD;
  case 32:
    return K1C::ZXWD;
  }
}

static uint64_t selectMask(unsigned Size) {
  if (4 < Size && Size <= 64 && Size % 2 == 0)
    return Size == 64 ? UINT64_MAX : (1L << Size) - 1L;

  llvm_unreachable("Unsupported size, cannot select mask!");
}

static SDValue getINSF(const SDLoc &DL, const SDValue &Vec, const SDValue &Val,
                       unsigned ElementBitSize, unsigned Pos,
                       SelectionDAG &DAG) {
  const uint64_t StartBit = Pos * ElementBitSize;
  const uint64_t StopBit = StartBit + ElementBitSize - 1;

  return SDValue(DAG.getMachineNode(
                     K1C::INSF, DL, MVT::i64,
                     {Vec, Val, DAG.getTargetConstant(StopBit, DL, MVT::i64),
                      DAG.getTargetConstant(StartBit, DL, MVT::i64)}),
                 0);
}

static SDValue lowerBUILD_VECTORGeneric(const SDValue &Op, SelectionDAG &DAG) {
  SDLoc DL(Op);
  uint64_t CurrentVal = 0;
  unsigned char RegMap = 0;
  const unsigned NumOperands = Op.getNumOperands();
  EVT VectorType = Op.getValueType();
  const unsigned VectorSize = VectorType.getVectorNumElements();
  const unsigned BitSize = VectorType.getScalarSizeInBits();
  uint64_t Mask = selectMask(BitSize);

  // Pack constants and mark non-constants indices
  for (unsigned i = 0; i < NumOperands; ++i) {
    uint64_t OpVal = 0;
    if (auto *OpConst = dyn_cast<ConstantSDNode>(Op.getOperand(i)))
      OpVal = OpConst->getZExtValue();
    else if (auto *OpConst = dyn_cast<ConstantFPSDNode>(Op.getOperand(i)))
      OpVal = OpConst->getValueAPF().bitcastToAPInt().getZExtValue();
    else
      RegMap |= 1 << i;

    CurrentVal |= (OpVal & Mask) << (i * BitSize);
  }

  SDValue PartSDVal = DAG.getConstant(CurrentVal, DL, MVT::i64);

  if (RegMap & 1) {
    if (RegMap == ((1 << VectorSize) - 1)) {
      PartSDVal = Op.getOperand(0);
    } else {
      SDValue ZESDVal = SDValue(DAG.getMachineNode(selectExtend(BitSize), DL,
                                                   MVT::i64, Op.getOperand(0)),
                                0);
      PartSDVal = DAG.getNode(ISD::OR, DL, MVT::i64, {ZESDVal, PartSDVal});
    }

    RegMap ^= 1;
  } else if (RegMap & (1 << (VectorSize - 1))) {
    RegMap ^= (1 << (VectorSize - 1));

    SDValue SHLSDVal = DAG.getNode(
        ISD::SHL, DL, MVT::i64,
        {SDValue(DAG.getMachineNode(TargetOpcode::COPY, DL, MVT::i64,
                                    Op.getOperand(NumOperands - 1)),
                 0),
         DAG.getConstant(64 - BitSize, DL, MVT::i64)});
    PartSDVal = DAG.getNode(ISD::OR, DL, MVT::i64, {SHLSDVal, PartSDVal});
  }

  for (unsigned i = 0; i < NumOperands; ++i) {
    if (RegMap & (1 << i))
      PartSDVal = getINSF(DL, PartSDVal, Op.getOperand(i), BitSize, i, DAG);
  }

  return DAG.getBitcast(Op.getValueType(), PartSDVal);
}

SDValue K1CTargetLowering::lowerBUILD_VECTOR(SDValue Op,
                                             SelectionDAG &DAG) const {
  switch (Op.getSimpleValueType().SimpleTy) {
  default:
    llvm_unreachable("Unsupported lowering for this type!");
  case MVT::v2i16:
  case MVT::v2f16:
    return lowerBUILD_VECTOR_V2_32bit(Op, DAG);
  case MVT::v4i16:
  case MVT::v4f16:
  case MVT::v8i8:
    return lowerBUILD_VECTORGeneric(Op, DAG);
  case MVT::v2i32:
  case MVT::v2f32:
    return lowerBUILD_VECTOR_V2_64bit(Op, DAG);
  case MVT::v4i32:
  case MVT::v4f32:
    return lowerBUILD_VECTOR_V4_128bit(Op, DAG);
  }
}

SDValue K1CTargetLowering::lowerBUILD_VECTOR_V2_32bit(SDValue Op,
                                                      SelectionDAG &DAG) const {
  SDLoc DL(Op);

  SDValue Op1 = Op.getOperand(0);
  SDValue Op2 = Op.getOperand(1);

  uint64_t Op1Val;
  uint64_t Op2Val;

  bool IsOp1Const = false;
  bool IsOp2Const = false;

  if (isa<ConstantSDNode>(Op1)) {
    auto *Op1Const = dyn_cast<ConstantSDNode>(Op1);
    Op1Val = Op1Const->getZExtValue();
    IsOp1Const = true;
  } else if (isa<ConstantFPSDNode>(Op1)) {
    auto *Op1Const = dyn_cast<ConstantFPSDNode>(Op1);
    Op1Val = Op1Const->getValueAPF().bitcastToAPInt().getZExtValue();
    IsOp1Const = true;
  }

  if (isa<ConstantSDNode>(Op2)) {
    auto *Op2Const = dyn_cast<ConstantSDNode>(Op2);
    Op2Val = Op2Const->getZExtValue();
    IsOp2Const = true;
  } else if (isa<ConstantFPSDNode>(Op2)) {
    auto *Op2Const = dyn_cast<ConstantFPSDNode>(Op2);
    Op2Val = Op2Const->getValueAPF().bitcastToAPInt().getZExtValue();
    IsOp2Const = true;
  }

  // imm imm
  if (IsOp1Const && IsOp2Const) {
    uint64_t R = ((Op2Val & 0xFFFF) << 16) | (Op1Val & 0xFFFF);

    return DAG.getBitcast(Op.getValueType(), DAG.getConstant(R, DL, MVT::i32));
  } else {
    return SDValue(
        DAG.getMachineNode(K1C::INSF, DL, Op.getValueType(),
                           {Op1, Op2, DAG.getTargetConstant(31, DL, MVT::i64),
                            DAG.getTargetConstant(16, DL, MVT::i64)}),
        0);
  }
}

SDValue K1CTargetLowering::lowerBUILD_VECTOR_V2_64bit(SDValue Op,
                                                      SelectionDAG &DAG,
                                                      bool useINSF) const {
  SDLoc DL(Op);

  if (Op.isUndef())
    return Op;

  SDValue Op1 = Op.getOperand(0);
  SDValue Op2 = Op.getOperand(1);

  bool IsOp1Const = false;
  bool IsOp2Const = false;

  uint64_t Op1Val;
  uint64_t Op2Val;

  if (isa<ConstantSDNode>(Op1)) {
    auto *Op1Const = dyn_cast<ConstantSDNode>(Op1);
    Op1Val = Op1Const->getZExtValue();
    IsOp1Const = true;
  } else if (isa<ConstantFPSDNode>(Op1)) {
    auto *Op1Const = dyn_cast<ConstantFPSDNode>(Op1);
    Op1Val = Op1Const->getValueAPF().bitcastToAPInt().getZExtValue();
    IsOp1Const = true;
  }

  if (isa<ConstantSDNode>(Op2)) {
    auto *Op2Const = dyn_cast<ConstantSDNode>(Op2);
    Op2Val = Op2Const->getZExtValue();
    IsOp2Const = true;
  } else if (isa<ConstantFPSDNode>(Op2)) {
    auto *Op2Const = dyn_cast<ConstantFPSDNode>(Op2);
    Op2Val = Op2Const->getValueAPF().bitcastToAPInt().getZExtValue();
    IsOp2Const = true;
  }

  if (Op1.isUndef()) {
    Op1Val = 0;
    IsOp1Const = true;
  }

  if (Op2.isUndef()) {
    Op2Val = 0;
    IsOp2Const = true;
  }

  if (IsOp1Const && IsOp2Const) { // imm imm
    uint64_t R = ((Op2Val << 32) | (Op1Val & 0xFFFFFFFF));

    return DAG.getBitcast(Op.getValueType(), DAG.getConstant(R, DL, MVT::i64));
  } else if (IsOp1Const) { // imm reg
    Op1Val &= 0xFFFFFFFF;

    SDValue ShiftedOp2 = DAG.getNode(
        ISD::SHL, DL, MVT::i64,
        {SDValue(DAG.getMachineNode(TargetOpcode::COPY, DL, MVT::i64,
                                    DAG.getBitcast(MVT::i32, Op2)),
                 0),
         DAG.getConstant(32, DL, MVT::i32)});

    return DAG.getBitcast(
        Op.getValueType(),
        DAG.getNode(ISD::OR, DL, MVT::i64,
                    {ShiftedOp2, DAG.getConstant(Op1Val, DL, MVT::i64)}));
  } else if (IsOp2Const) { // reg imm
    Op2Val <<= 32;

    SDValue Op1Val = DAG.getNode(ISD::ZERO_EXTEND, DL, MVT::i64,
                                 DAG.getBitcast(MVT::i32, Op1));

    return DAG.getBitcast(
        Op.getValueType(),
        DAG.getNode(ISD::OR, DL, MVT::i64,
                    {Op1Val, DAG.getConstant(Op2Val, DL, MVT::i64)}));
  } else { // reg reg
    if (Op1.isUndef() && Op2.isUndef())
      return SDValue(
          DAG.getMachineNode(TargetOpcode::IMPLICIT_DEF, DL, Op.getValueType()),
          0);
    if (useINSF) {
      return SDValue(
          DAG.getMachineNode(K1C::INSF, DL, Op.getValueType(),
                             {Op1, Op2, DAG.getTargetConstant(63, DL, MVT::i64),
                              DAG.getTargetConstant(32, DL, MVT::i64)}),
          0);
    } else {
      SDValue ShiftedOp2 = DAG.getNode(
          ISD::SHL, DL, MVT::i64,
          {SDValue(DAG.getMachineNode(TargetOpcode::COPY, DL, MVT::i64,
                                      DAG.getBitcast(MVT::i32, Op2)),
                   0),
           DAG.getConstant(32, DL, MVT::i32)});
      SDValue Op1Val = DAG.getNode(ISD::ZERO_EXTEND, DL, MVT::i64,
                                   DAG.getBitcast(MVT::i32, Op1));

      return DAG.getBitcast(
          Op.getValueType(),
          DAG.getNode(ISD::OR, DL, MVT::i64, {Op1Val, ShiftedOp2}));
    }
  }
}

SDValue
K1CTargetLowering::lowerBUILD_VECTOR_V4_128bit(SDValue Op,
                                               SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue V1 = Op.getOperand(0);
  SDValue V2 = Op.getOperand(1);
  SDValue V3 = Op.getOperand(2);
  SDValue V4 = Op.getOperand(3);

  EVT VT = Op.getValueType();
  LLVMContext &Ctx = *DAG.getContext();
  EVT HalfVT = VT.getHalfNumVectorElementsVT(Ctx);

  SDValue VecLow = DAG.getNode(ISD::BUILD_VECTOR, DL, HalfVT, {V1, V2});
  // if BUILD_VECTOR is still necessary after folding
  if (!VecLow.isUndef() && VecLow.getOpcode() == ISD::BUILD_VECTOR) {
    VecLow = lowerBUILD_VECTOR_V2_64bit(VecLow, DAG, false);
  }

  SDValue VecHi = DAG.getNode(ISD::BUILD_VECTOR, DL, HalfVT, {V3, V4});
  // if BUILD_VECTOR is still necessary after folding
  if (!VecHi.isUndef() && VecHi.getOpcode() == ISD::BUILD_VECTOR) {
    VecHi = lowerBUILD_VECTOR_V2_64bit(VecHi, DAG, false);
  }

  SDValue ImpV =
      SDValue(DAG.getMachineNode(TargetOpcode::IMPLICIT_DEF, DL, VT), 0);
  SDValue InsLow =
      SDValue(DAG.getMachineNode(
                  TargetOpcode::INSERT_SUBREG, DL, VT,
                  {ImpV, VecLow, DAG.getTargetConstant(1, DL, MVT::i64)}),
              0);
  if (VecHi.isUndef())
    return InsLow;
  return SDValue(DAG.getMachineNode(
                     TargetOpcode::INSERT_SUBREG, DL, VT,
                     {InsLow, VecHi, DAG.getTargetConstant(2, DL, MVT::i64)}),
                 0);
}

static SDValue makeInsertConst(const SDLoc &DL, SDValue Vec, MVT Type,
                               uint64_t Mask, uint64_t ImmVal,
                               unsigned StartIdx, SelectionDAG &DAG) {
  Vec = DAG.getNode(
      ISD::AND, DL, Type,
      SDValue(DAG.getMachineNode(TargetOpcode::COPY, DL, Type, Vec), 0),
      DAG.getConstant(~(Mask << StartIdx), DL, Type));

  return DAG.getNode(
      ISD::OR, DL, Type,
      {Vec, DAG.getConstant((ImmVal & Mask) << StartIdx, DL, Type)});
}

static MVT selectType(unsigned Size) {
  switch (Size) {
  default:
    llvm_unreachable("Unsupported type for this size!");
  case 8:
  case 16:
  case 32:
    return MVT::i32;
  case 64:
  case 128:
    return MVT::i64;
  }
}

SDValue K1CTargetLowering::lowerINSERT_VECTOR_ELT(SDValue Op,
                                                  SelectionDAG &DAG) const {
  SDValue Vec = Op.getOperand(0);
  SDValue Val = Op.getOperand(1);
  SDValue Idx = Op.getOperand(2);
  SDLoc DL(Op);
  EVT VecVT = Op.getValueType();

  if (ConstantSDNode *InsertPos = dyn_cast<ConstantSDNode>(Idx)) {
    unsigned ElementBitSize = VecVT.getScalarSizeInBits();
    uint64_t Mask = selectMask(ElementBitSize);
    auto SizeType = selectType(VecVT.getSizeInBits());

    if (Vec.getValueType() == MVT::v4f32 || Vec.getValueType() == MVT::v4i32)
      return lowerINSERT_VECTOR_ELT_V4_128bit(DL, DAG, Vec, Val,
                                              InsertPos->getZExtValue());
    if (Vec.getValueType() == MVT::v2f64 || Vec.getValueType() == MVT::v2i64)
      return lowerINSERT_VECTOR_ELT_V2_128bit(DL, DAG, Vec, Val,
                                              InsertPos->getZExtValue());

    uint64_t ImmVal;
    if (auto *OpConst = dyn_cast<ConstantSDNode>(Val))
      ImmVal = OpConst->getZExtValue();
    else if (auto *OpConst = dyn_cast<ConstantFPSDNode>(Val))
      ImmVal = OpConst->getValueAPF().bitcastToAPInt().getZExtValue();
    else
      return getINSF(DL, Vec, Val, ElementBitSize, InsertPos->getZExtValue(),
                     DAG);

    const unsigned StartBit = ElementBitSize * InsertPos->getZExtValue();
    return DAG.getBitcast(
        Op.getValueType(),
        makeInsertConst(DL, Vec, SizeType, Mask, ImmVal, StartBit, DAG));
  }

  EVT EltVT = VecVT.getVectorElementType();

  SDValue StackPtr = DAG.CreateStackTemporary(VecVT);
  int SPFI = cast<FrameIndexSDNode>(StackPtr.getNode())->getIndex();

  // Store the vector.
  SDValue Ch = DAG.getStore(
      DAG.getEntryNode(), DL, Vec, StackPtr,
      MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), SPFI));

  // Calculate insert location.
  SDValue StackPtr2 = getVectorElementPointer(DAG, StackPtr, VecVT, Idx);

  // Store the scalar value.
  Ch = DAG.getTruncStore(Ch, DL, Val, StackPtr2, MachinePointerInfo(), EltVT);

  // Load the updated vector.
  return DAG.getLoad(
      VecVT, DL, Ch, StackPtr,
      MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), SPFI));
}

SDValue K1CTargetLowering::lowerINSERT_VECTOR_ELT_V4_128bit(
    SDLoc &dl, SelectionDAG &DAG, SDValue Vec, SDValue Val,
    uint64_t index) const {
  SDValue v1, subRegIdx, mask;
  if (index % 2 == 0) {
    subRegIdx = DAG.getTargetConstant(index == 0 ? 1 : 2, dl, MVT::i32);
    v1 = DAG.getNode(ISD::ZERO_EXTEND, dl, MVT::i64,
                     DAG.getBitcast(MVT::i32, Val));
    mask = DAG.getConstant(0xffffffff00000000, dl, MVT::i64);
  } else {
    subRegIdx = DAG.getTargetConstant(index == 1 ? 1 : 2, dl, MVT::i32);
    SDValue val32 = SDValue(DAG.getMachineNode(TargetOpcode::COPY, dl, MVT::i64,
                                               DAG.getBitcast(MVT::i32, Val)),
                            0);
    v1 = DAG.getNode(ISD::SHL, dl, MVT::i64,
                     {val32, DAG.getConstant(32, dl, MVT::i32)});
    mask = DAG.getConstant(0xffffffff, dl, MVT::i64);
  }
  SDValue subreg = SDValue(DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, dl,
                                              MVT::i64, {Vec, subRegIdx}),
                           0);

  SDValue v2 = DAG.getNode(ISD::AND, dl, MVT::i64, {subreg, mask});

  SDValue orResult = DAG.getNode(ISD::OR, dl, MVT::i64, {v1, v2});
  return SDValue(DAG.getMachineNode(TargetOpcode::INSERT_SUBREG, dl, MVT::v4f32,
                                    {Vec, orResult, subRegIdx}),
                 0);
}

SDValue K1CTargetLowering::lowerINSERT_VECTOR_ELT_V2_128bit(
    SDLoc &dl, SelectionDAG &DAG, SDValue Vec, SDValue Val,
    uint64_t index) const {
  SDValue subRegIdx = DAG.getTargetConstant(index + 1, dl, MVT::i32);
  return SDValue(DAG.getMachineNode(TargetOpcode::INSERT_SUBREG, dl,
                                    Vec.getValueType(), {Vec, Val, subRegIdx}),
                 0);
}

SDValue
K1CTargetLowering::lowerEXTRACT_VECTOR_ELT_REGISTER(SDValue Op,
                                                    SelectionDAG &DAG) const {
  SDValue Vec = Op.getOperand(0);
  SDValue Idx = Op.getOperand(1);
  SDLoc dl(Op);

  EVT VecVT = Vec.getValueType();
  SDValue StackPtr = DAG.CreateStackTemporary(VecVT);
  SDValue Ch =
      DAG.getStore(DAG.getEntryNode(), dl, Vec, StackPtr, MachinePointerInfo());

  StackPtr = getVectorElementPointer(DAG, StackPtr, VecVT, Idx);

  SDValue NewLoad;

  if (Op.getValueType().isVector())
    NewLoad =
        DAG.getLoad(Op.getValueType(), dl, Ch, StackPtr, MachinePointerInfo());
  else
    NewLoad =
        DAG.getExtLoad(ISD::EXTLOAD, dl, Op.getValueType(), Ch, StackPtr,
                       MachinePointerInfo(), VecVT.getVectorElementType());

  DAG.ReplaceAllUsesOfValueWith(Ch, SDValue(NewLoad.getNode(), 1));

  SmallVector<SDValue, 6> NewLoadOperands(NewLoad->op_begin(),
                                          NewLoad->op_end());
  NewLoadOperands[0] = Ch;
  NewLoad =
      SDValue(DAG.UpdateNodeOperands(NewLoad.getNode(), NewLoadOperands), 0);
  return NewLoad;
}

SDValue K1CTargetLowering::lowerEXTRACT_VECTOR_ELT(SDValue Op,
                                                   SelectionDAG &DAG) const {
  SDValue Idx = Op.getOperand(1);

  if (isa<ConstantSDNode>(Idx)) {
    // use patterns in td
    return Op;
  }

  return lowerEXTRACT_VECTOR_ELT_REGISTER(Op, DAG);
}

SDValue K1CTargetLowering::lowerCONCAT_VECTORS(SDValue Op,
                                               SelectionDAG &DAG) const {
  auto OperandSize = Op.getOperand(0).getValueSizeInBits();
  auto ResultSize = Op.getValueSizeInBits();

  if (!((OperandSize == 32 && ResultSize == 64) ||
        (OperandSize == 64 && ResultSize == 128)))
    llvm_unreachable("Unsupported concat for these types");

  SDLoc DL(Op);
  SDValue Out = SDValue(
      DAG.getMachineNode(TargetOpcode::IMPLICIT_DEF, DL, Op->getValueType(0)),
      0);
  EVT VT = Op.getValueType();

  for (unsigned i = 0, e = Op->getNumOperands(); i != e; ++i) {
    if (Op->getOperand(i).isUndef())
      continue;
    if (OperandSize == 32)
      Out = getINSF(DL, Out, Op->getOperand(i), OperandSize, i, DAG);
    if (OperandSize == 64)
      Out = DAG.getTargetInsertSubreg(i + 1, DL, VT, Out, Op.getOperand(i));
  }

  return Out;
}
