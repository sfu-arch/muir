; ModuleID = 'test02.bc'
source_filename = "test02.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: noinline nounwind
define i32 @foo(i32 %a, i32 %b) #0 {
entry:
  %div = udiv i32 %a, 2
  %cmp = icmp eq i32 %div, 4
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %add = add i32 %a, %b
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %sum.0 = phi i32 [ %add, %if.then ], [ 0, %entry ]
  ret i32 %sum.0
}

; Function Attrs: noinline nounwind
define i32 @main() #0 {
entry:
  %call = call i32 @foo(i32 5, i32 3)
  ret i32 0
}

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 "}
