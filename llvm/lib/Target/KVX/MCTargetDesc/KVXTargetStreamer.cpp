//===-- KVXTargetStreamer.h - KVX Target Streamer --------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "KVXTargetStreamer.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

KVXTargetStreamer::KVXTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

// This part is for ascii assembly output
KVXTargetAsmStreamer::KVXTargetAsmStreamer(MCStreamer &S,
					   formatted_raw_ostream &OS)
    : KVXTargetStreamer(S), OS(OS) {}
