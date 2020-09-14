; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s | FileCheck %s
target triple = "kvx-kalray-cos"

define void @f() {
; CHECK-LABEL: f:
; CHECK:       # %bb.0:
; CHECK-NEXT:    addd $r12 = $r12, -32
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_def_cfa_offset 32
; CHECK-NEXT:    addd $r12 = $r12, 32
; CHECK-NEXT:    ret
; CHECK-NEXT:    ;;
  %1 = alloca i32, align 4
  ret void
}

define void @g() {
; CHECK-LABEL: g:
; CHECK:       # %bb.0:
; CHECK-NEXT:    addd $r12 = $r12, -32
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_def_cfa_offset 32
; CHECK-NEXT:    addd $r12 = $r12, 32
; CHECK-NEXT:    ret
; CHECK-NEXT:    ;;
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  ret void
}

define void @h() {
; CHECK-LABEL: h:
; CHECK:       # %bb.0:
; CHECK-NEXT:    addd $r12 = $r12, -32
; CHECK-NEXT:    ;;
; CHECK-NEXT:    .cfi_def_cfa_offset 32
; CHECK-NEXT:    addd $r12 = $r12, 32
; CHECK-NEXT:    ret
; CHECK-NEXT:    ;;
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  ret void
}
