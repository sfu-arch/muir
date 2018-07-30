; ModuleID = 'cilk_for_test12.bc'
source_filename = "cilk_for_test12.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@main.a = private unnamed_addr constant [8 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7], align 4
@.str = private unnamed_addr constant [4 x i8] c"%u,\00", align 1
@.str.1 = private unnamed_addr constant [15 x i8] c"\0Areturned: %u\0A\00", align 1

; Function Attrs: noinline nounwind
define i32 @cilk_for_test12(i32* %a, i32 %n) #0 {
entry:
  %result = alloca i32, align 4
  store i32 0, i32* %result, align 4
  br label %pfor.cond

pfor.cond:                                        ; preds = %pfor.inc23, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc24, %pfor.inc23 ]
  %cmp = icmp ult i32 %i.0, 3
  br i1 %cmp, label %pfor.detach, label %pfor.end25

pfor.detach:                                      ; preds = %pfor.cond
  detach label %pfor.body, label %pfor.inc23

pfor.body:                                        ; preds = %pfor.detach
  br label %pfor.cond2

pfor.cond2:                                       ; preds = %pfor.inc15, %pfor.body
  %j.0 = phi i32 [ 0, %pfor.body ], [ %inc16, %pfor.inc15 ]
  %cmp3 = icmp ult i32 %j.0, %n
  br i1 %cmp3, label %pfor.detach4, label %pfor.end17

pfor.detach4:                                     ; preds = %pfor.cond2
  detach label %pfor.body5, label %pfor.inc15

pfor.body5:                                       ; preds = %pfor.detach4
  br label %pfor.cond7

pfor.cond7:                                       ; preds = %pfor.inc, %pfor.body5
  %k.0 = phi i32 [ 0, %pfor.body5 ], [ %inc, %pfor.inc ]
  %cmp8 = icmp ult i32 %k.0, %n
  br i1 %cmp8, label %pfor.detach9, label %pfor.end

pfor.detach9:                                     ; preds = %pfor.cond7
  detach label %pfor.body10, label %pfor.inc

pfor.body10:                                      ; preds = %pfor.detach9
  %arrayidx = getelementptr inbounds i32, i32* %a, i32 %k.0
  %0 = load i32, i32* %arrayidx, align 4
  %mul = mul i32 2, %0
  %arrayidx11 = getelementptr inbounds i32, i32* %a, i32 %k.0
  store i32 %mul, i32* %arrayidx11, align 4
  br label %pfor.preattach

pfor.preattach:                                   ; preds = %pfor.body10
  reattach label %pfor.inc

pfor.inc:                                         ; preds = %pfor.preattach, %pfor.detach9
  %inc = add i32 %k.0, 1
  br label %pfor.cond7, !llvm.loop !1

pfor.end:                                         ; preds = %pfor.cond7
  sync label %pfor.end.continue

pfor.end.continue:                                ; preds = %pfor.end
  %sub = sub i32 %n, 1
  %arrayidx12 = getelementptr inbounds i32, i32* %a, i32 %sub
  %1 = load i32, i32* %arrayidx12, align 4
  %inc13 = add i32 %1, 1
  store i32 %inc13, i32* %arrayidx12, align 4
  br label %pfor.preattach14

pfor.preattach14:                                 ; preds = %pfor.end.continue
  reattach label %pfor.inc15

pfor.inc15:                                       ; preds = %pfor.preattach14, %pfor.detach4
  %inc16 = add i32 %j.0, 1
  br label %pfor.cond2, !llvm.loop !3

pfor.end17:                                       ; preds = %pfor.cond2
  sync label %pfor.end.continue18

pfor.end.continue18:                              ; preds = %pfor.end17
  %sub19 = sub i32 %n, 1
  %arrayidx20 = getelementptr inbounds i32, i32* %a, i32 %sub19
  %2 = load i32, i32* %arrayidx20, align 4
  %inc21 = add i32 %2, 1
  store i32 %inc21, i32* %arrayidx20, align 4
  %3 = load i32, i32* %result, align 4
  %add = add i32 %3, %2
  store i32 %add, i32* %result, align 4
  br label %pfor.preattach22

pfor.preattach22:                                 ; preds = %pfor.end.continue18
  reattach label %pfor.inc23

pfor.inc23:                                       ; preds = %pfor.preattach22, %pfor.detach
  %inc24 = add i32 %i.0, 1
  br label %pfor.cond, !llvm.loop !4

pfor.end25:                                       ; preds = %pfor.cond
  sync label %pfor.end.continue26

pfor.end.continue26:                              ; preds = %pfor.end25
  %4 = load i32, i32* %result, align 4
  %div = sdiv i32 %4, 2
  store i32 %div, i32* %result, align 4
  %5 = load i32, i32* %result, align 4
  ret i32 %5
}

; Function Attrs: noinline nounwind
define i32 @main() #0 {
entry:
  %a = alloca [8 x i32], align 4
  %0 = bitcast [8 x i32]* %a to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %0, i8* bitcast ([8 x i32]* @main.a to i8*), i32 32, i32 4, i1 false)
  %arraydecay = getelementptr inbounds [8 x i32], [8 x i32]* %a, i32 0, i32 0
  %call = call i32 @cilk_for_test12(i32* %arraydecay, i32 8)
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, 8
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds [8 x i32], [8 x i32]* %a, i32 0, i32 %i.0
  %1 = load i32, i32* %arrayidx, align 4
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %1)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.1, i32 0, i32 0), i32 %call)
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture writeonly, i8* nocapture readonly, i32, i32, i1) #1

declare i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 "}
!1 = distinct !{!1, !2}
!2 = !{!"tapir.loop.spawn.strategy", i32 1}
!3 = distinct !{!3, !2}
!4 = distinct !{!4, !2}
