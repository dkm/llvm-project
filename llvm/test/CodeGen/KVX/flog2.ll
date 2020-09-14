; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=kvx-kalray-cos | FileCheck %s

define float @log2f32(float %x) {
; CHECK-LABEL: log2f32:
; CHECK:       # %bb.0:
; CHECK-NEXT:    addd $r12 = $r12, -32
; CHECK-NEXT:    get $r16 = $ra
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_def_cfa_offset 32
; CHECK-NEXT:    sd 0[$r12] = $r16
; CHECK-NEXT:    call log2f
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_offset 67, -32
; CHECK-NEXT:    ld $r16 = 0[$r12]
; CHECK-NEXT:    ;;
; CHECK-NEXT:    set $ra = $r16
; CHECK-NEXT:    addd $r12 = $r12, 32
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_def_cfa_offset 0
; CHECK-NEXT:    ret
; CHECK-NEXT:    ;;
  %tmp = call float @llvm.log2.f32(float %x)
  ret float %tmp
}

define double @log2f64(double %x) {
; CHECK-LABEL: log2f64:
; CHECK:       # %bb.0:
; CHECK-NEXT:    addd $r12 = $r12, -32
; CHECK-NEXT:    get $r16 = $ra
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_def_cfa_offset 32
; CHECK-NEXT:    sd 0[$r12] = $r16
; CHECK-NEXT:    call log2
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_offset 67, -32
; CHECK-NEXT:    ld $r16 = 0[$r12]
; CHECK-NEXT:    ;;
; CHECK-NEXT:    set $ra = $r16
; CHECK-NEXT:    addd $r12 = $r12, 32
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_def_cfa_offset 0
; CHECK-NEXT:    ret
; CHECK-NEXT:    ;;
  %tmp = call double @llvm.log2.f64(double %x)
  ret double %tmp
}

define <2 x float> @log2v2f32(<2 x float> %x) {
; CHECK-LABEL: log2v2f32:
; CHECK:       # %bb.0:
; CHECK-NEXT:    addd $r12 = $r12, -32
; CHECK-NEXT:    get $r16 = $ra
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_def_cfa_offset 32
; CHECK-NEXT:    sd 16[$r12] = $r16
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_offset 67, -16
; CHECK-NEXT:    sq 0[$r12] = $r18r19
; CHECK-NEXT:    copyd $r18 = $r0
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_offset 19, -24
; CHECK-NEXT:    .cfi_offset 18, -32
; CHECK-NEXT:    srad $r0 = $r18, 32
; CHECK-NEXT:    call log2f
; CHECK-NEXT:    ;;
; CHECK-NEXT:    copyd $r19 = $r0
; CHECK-NEXT:    copyd $r0 = $r18
; CHECK-NEXT:    call log2f
; CHECK-NEXT:    ;;
; CHECK-NEXT:    insf $r0 = $r19, 63, 32
; CHECK-NEXT:    lq $r18r19 = 0[$r12]
; CHECK-NEXT:    ;;
; CHECK-NEXT:    ld $r16 = 16[$r12]
; CHECK-NEXT:    ;;
; CHECK-NEXT:    set $ra = $r16
; CHECK-NEXT:    addd $r12 = $r12, 32
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_def_cfa_offset 0
; CHECK-NEXT:    ret
; CHECK-NEXT:    ;;
  %tmp = call <2 x float> @llvm.log2.v2f32(<2 x float> %x)
  ret <2 x float> %tmp
}

define <2 x double> @log2v2f64(<2 x double> %x) {
; CHECK-LABEL: log2v2f64:
; CHECK:       # %bb.0:
; CHECK-NEXT:    addd $r12 = $r12, -32
; CHECK-NEXT:    get $r16 = $ra
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_def_cfa_offset 32
; CHECK-NEXT:    sd 24[$r12] = $r16
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_offset 67, -8
; CHECK-NEXT:    sq 8[$r12] = $r20r21
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_offset 21, -16
; CHECK-NEXT:    .cfi_offset 20, -24
; CHECK-NEXT:    sd 0[$r12] = $r18
; CHECK-NEXT:    copyd $r18 = $r1
; CHECK-NEXT:    call log2
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_offset 18, -32
; CHECK-NEXT:    copyd $r20 = $r0
; CHECK-NEXT:    copyd $r0 = $r18
; CHECK-NEXT:    call log2
; CHECK-NEXT:    ;;
; CHECK-NEXT:    copyd $r21 = $r0
; CHECK-NEXT:    copyd $r0 = $r20
; CHECK-NEXT:    ;;
; CHECK-NEXT:    copyd $r1 = $r21
; CHECK-NEXT:    ld $r18 = 0[$r12]
; CHECK-NEXT:    ;;
; CHECK-NEXT:    lq $r20r21 = 8[$r12]
; CHECK-NEXT:    ;;
; CHECK-NEXT:    ld $r16 = 24[$r12]
; CHECK-NEXT:    ;;
; CHECK-NEXT:    set $ra = $r16
; CHECK-NEXT:    addd $r12 = $r12, 32
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_def_cfa_offset 0
; CHECK-NEXT:    ret
; CHECK-NEXT:    ;;
  %tmp = call <2 x double> @llvm.log2.v2f64(<2 x double> %x)
  ret <2 x double> %tmp
}

declare float @llvm.log2.f32(float)
declare double @llvm.log2.f64(double)
declare <2x float> @llvm.log2.v2f32(<2 x float>)
declare <2x double> @llvm.log2.v2f64(<2 x double>)
