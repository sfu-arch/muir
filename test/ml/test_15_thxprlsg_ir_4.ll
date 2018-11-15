; ModuleID = './test_15_thxprlsg_ir_4.bc'
source_filename = "./15_thxprlsg_ir_4.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux_gnu"

define void @test_15_thxprlsg_ir_4(i8* nocapture align 8 dereferenceable(8) %retval, i8* noalias nocapture readnone %run_options, i8** noalias nocapture readonly %params, i8** noalias nocapture readonly %temps, i64* noalias nocapture readnone %prof_counters) #0 {
fusion.loop_body.dim.0.lr.ph:
  %arg0.untyped = load i8*, i8** %params, align 8, !dereferenceable !0, !align !1
  %0 = bitcast i8* %arg0.untyped to [8 x float]*
  %1 = load i8*, i8** %temps, align 8, !dereferenceable !0, !align !1
  %fusion = bitcast i8* %1 to [8 x float]*
  br label %fusion.loop_body.dim.0

fusion.loop_body.dim.0:                           ; preds = %fusion.loop_body.dim.0, %fusion.loop_body.dim.0.lr.ph
  %fusion.indvar.dim.02 = phi i64 [ 0, %fusion.loop_body.dim.0.lr.ph ], [ %invar.inc, %fusion.loop_body.dim.0 ]
  %2 = getelementptr inbounds [8 x float], [8 x float]* %0, i64 0, i64 %fusion.indvar.dim.02
  %3 = load float, float* %2, align 4
  %4 = call float @tanhf(float %3)
  %5 = call float @llvm.exp.f32(float %4)
  %6 = fcmp ole float %5, 0.000000e+00
  %.op = fmul float %5, 5.000000e-01
  %7 = select i1 %6, float 0.000000e+00, float %.op
  %8 = call float @tanhf(float %7)
  %9 = fmul float %8, 5.000000e-01
  %10 = fadd float %9, 5.000000e-01
  %11 = getelementptr inbounds [8 x float], [8 x float]* %fusion, i64 0, i64 %fusion.indvar.dim.02
  store float %10, float* %11, align 4
  %invar.inc = add nuw nsw i64 %fusion.indvar.dim.02, 1
  %12 = icmp ugt i64 %invar.inc, 7
  br i1 %12, label %fusion.loop_exit.dim.0, label %fusion.loop_body.dim.0

fusion.loop_exit.dim.0:                           ; preds = %fusion.loop_body.dim.0
  %13 = bitcast i8* %retval to i8**
  store i8* %1, i8** %13, align 8
  ret void
}

; Function Attrs: nounwind readnone
declare float @tanhf(float) #1

; Function Attrs: nounwind readnone
declare float @llvm.exp.f32(float) #1

attributes #0 = { "no-frame-pointer-elim"="false" }
attributes #1 = { nounwind readnone }

!0 = !{i64 32}
!1 = !{i64 8}
