; ModuleID = 'loop.bc'
source_filename = "loop.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: noinline nounwind
define void @foo(i32* %in, i32* %out1, i32* %out2, i32 %gain, i32 %nsamps) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %sum.0 = phi i32 [ 0, %entry ], [ %add, %for.inc ]
  %cmp = icmp slt i32 %i.0, %nsamps
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds i32, i32* %in, i32 %i.0
  %0 = load i32, i32* %arrayidx, align 4
  %mul = mul nsw i32 %0, %gain
  %add = add nsw i32 %sum.0, %mul
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %mul1 = mul nsw i32 %sum.0, 5
  %arrayidx2 = getelementptr inbounds i32, i32* %out1, i32 0
  store i32 %mul1, i32* %arrayidx2, align 4
  %mul3 = mul nsw i32 %sum.0, 8
  %arrayidx4 = getelementptr inbounds i32, i32* %out2, i32 0
  store i32 %mul3, i32* %arrayidx4, align 4
  ret void
}

; Function Attrs: noinline nounwind
define i32 @main() #0 {
entry:
  call void @foo(i32* undef, i32* undef, i32* undef, i32 undef, i32 undef)
  ret i32 0
}

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"NumRegisterParameters", i32 0}
!1 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
