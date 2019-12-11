//===-- KVXTargetStreamer.h - KVX Target Streamer --------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_KVX_KVXTARGETSTREAMER_H
#define LLVM_LIB_TARGET_KVX_KVXTARGETSTREAMER_H

#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/FormattedStream.h"

namespace llvm {

class KVXTargetStreamer : public MCTargetStreamer {

public:
  KVXTargetStreamer(MCStreamer &S);
};

// This part is for ascii assembly output
class K1CTargetAsmStreamer : public K1CTargetStreamer {
  formatted_raw_ostream &OS;

public:
  K1CTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);
};

} // namespace llvm
#endif
