; ModuleID = 'cilk_for_test01.bc'
source_filename = "cilk_for_test01.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@main.a = private unnamed_addr constant [5 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4], align 4
@.str = private unnamed_addr constant [10 x i8] c"b[%d]=%d\0A\00", align 1

; Function Attrs: noinline nounwind
define i32 @cilk_for_test01(i32* %a, i32* %b) #0 {
entry:
  br label %pfor.cond

pfor.cond:                                        ; preds = %pfor.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %pfor.inc ]
  %cmp = icmp slt i32 %i.0, 5
  br i1 %cmp, label %pfor.detach, label %pfor.end

pfor.detach:                                      ; preds = %pfor.cond
  detach label %pfor.body, label %pfor.inc

pfor.body:                                        ; preds = %pfor.detach
  %arrayidx = getelementptr inbounds i32, i32* %a, i32 %i.0
  %0 = load i32, i32* %arrayidx, align 4
  %mul = mul i32 %0, 2
  %arrayidx1 = getelementptr inbounds i32, i32* %b, i32 %i.0
  store i32 %mul, i32* %arrayidx1, align 4
  br label %pfor.preattach

pfor.preattach:                                   ; preds = %pfor.body
  reattach label %pfor.inc

pfor.inc:                                         ; preds = %pfor.preattach, %pfor.detach
  %inc = add nsw i32 %i.0, 1
  br label %pfor.cond, !llvm.loop !1

pfor.end:                                         ; preds = %pfor.cond
  sync label %pfor.end.continue

pfor.end.continue:                                ; preds = %pfor.end
  ret i32 1
}

; Function Attrs: noinline nounwind
define i32 @main() #0 {
entry:
  %a = alloca [5 x i32], align 4
  %b = alloca [5 x i32], align 4
  %0 = bitcast [5 x i32]* %a to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %0, i8* bitcast ([5 x i32]* @main.a to i8*), i32 20, i32 4, i1 false)
  %1 = bitcast [5 x i32]* %b to i8*
  call void @llvm.memset.p0i8.i32(i8* %1, i8 0, i32 20, i32 4, i1 false)
  %arraydecay = getelementptr inbounds [5 x i32], [5 x i32]* %a, i32 0, i32 0
  %arraydecay1 = getelementptr inbounds [5 x i32], [5 x i32]* %b, i32 0, i32 0
  %call = call i32 @cilk_for_test01(i32* %arraydecay, i32* %arraydecay1)
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, 5
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds [5 x i32], [5 x i32]* %b, i32 0, i32 %i.0
  %2 = load i32, i32* %arrayidx, align 4
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i32 0, i32 0), i32 %i.0, i32 %2)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture writeonly, i8* nocapture readonly, i32, i32, i1) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i32(i8* nocapture writeonly, i8, i32, i32, i1) #1

declare i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 "}
!1 = distinct !{!1, !2}
!2 = !{!"tapir.loop.spawn.strategy", i32 1}
