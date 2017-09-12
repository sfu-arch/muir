; ModuleID = 'alias.bc'
source_filename = "alias.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(float*, float*, float, i32) #0 {
  br label %5

; <label>:5:                                      ; preds = %14, %4
  %.0 = phi i32 [ 0, %4 ], [ %15, %14 ]
  %6 = icmp slt i32 %.0, %3
  br i1 %6, label %7, label %16

; <label>:7:                                      ; preds = %5
  %8 = sext i32 %.0 to i64
  %9 = getelementptr inbounds float, float* %0, i64 %8
  %10 = load float, float* %9, align 4
  %11 = fmul float %10, %2
  %12 = sext i32 %.0 to i64
  %13 = getelementptr inbounds float, float* %1, i64 %12
  store float %11, float* %13, align 4
  br label %14

; <label>:14:                                     ; preds = %7
  %15 = add nsw i32 %.0, 1
  br label %5

; <label>:16:                                     ; preds = %5
  ret void
}

; Function Attrs: noinline nounwind uwtable
define i32 @main() #0 {
  call void @foo(float* undef, float* undef, float undef, i32 undef)
  ret i32 0
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
