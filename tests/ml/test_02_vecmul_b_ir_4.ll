; ModuleID = './test_02_vecmul_b_ir_4.bc'
source_filename = "./02_vecmul_b_ir_4.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux_gnu"

define void @test_02_vecmul_b_ir_4(i8* nocapture align 8 dereferenceable(8) %retval, i8* noalias nocapture readnone %run_options, i8** noalias nocapture readonly %params, i8** noalias nocapture readonly %temps, i64* noalias nocapture readnone %prof_counters) #0 {
multiply.loop_body.dim.0.lr.ph:
  %0 = getelementptr inbounds i8*, i8** %params, i64 1
  %arg1.untyped = load i8*, i8** %0, align 8, !dereferenceable !0, !align !1
  %1 = bitcast i8* %arg1.untyped to [64 x float]*
  %arg0.untyped = load i8*, i8** %params, align 8, !dereferenceable !0, !align !1
  %2 = bitcast i8* %arg0.untyped to [64 x float]*
  %3 = load i8*, i8** %temps, align 8, !dereferenceable !0, !align !1
  %multiply = bitcast i8* %3 to [64 x float]*
  br label %multiply.loop_body.dim.0

multiply.loop_body.dim.0:                         ; preds = %multiply.loop_body.dim.0, %multiply.loop_body.dim.0.lr.ph
  %multiply.indvar.dim.02 = phi i64 [ 0, %multiply.loop_body.dim.0.lr.ph ], [ %invar.inc, %multiply.loop_body.dim.0 ]
  %4 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 %multiply.indvar.dim.02
  %5 = load float, float* %4, align 4
  %6 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 %multiply.indvar.dim.02
  %7 = load float, float* %6, align 4
  %8 = fmul float %5, %7
  %9 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 %multiply.indvar.dim.02
  store float %8, float* %9, align 4
  %invar.inc = add nuw nsw i64 %multiply.indvar.dim.02, 1
  %10 = icmp ugt i64 %invar.inc, 63
  br i1 %10, label %multiply.loop_exit.dim.0, label %multiply.loop_body.dim.0

multiply.loop_exit.dim.0:                         ; preds = %multiply.loop_body.dim.0
  %11 = bitcast i8* %retval to i8**
  store i8* %3, i8** %11, align 8
  ret void
}

attributes #0 = { "no-frame-pointer-elim"="false" }

!0 = !{i64 256}
!1 = !{i64 8}
