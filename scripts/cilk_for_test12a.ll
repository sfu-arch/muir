; ModuleID = 'final.ll'
source_filename = "cilk_for_test12.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@main.a = private unnamed_addr constant [8 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7], align 4
@.str = private unnamed_addr constant [4 x i8] c"%u,\00", align 1
@.str.1 = private unnamed_addr constant [15 x i8] c"\0Areturned: %u\0A\00", align 1

; Function Attrs: noinline nounwind
define i32 @cilk_for_test12(i32* %a, i32 %n) #0 !UID !1 {
entry:
  %result = alloca i32, align 4, !UID !2
  store i32 0, i32* %result, align 4, !UID !3
  br label %pfor.cond, !UID !4, !BB_UID !5

pfor.cond:                                        ; preds = %pfor.inc23, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc24, %pfor.inc23 ], !UID !6
  %cmp = icmp ult i32 %i.0, 3, !UID !7
  br i1 %cmp, label %pfor.detach, label %pfor.end25, !UID !8, !BB_UID !9

pfor.detach:                                      ; preds = %pfor.cond
  detach label %offload.pfor.body, label %pfor.inc23, !UID !10, !BB_UID !11

pfor.inc23:                                       ; preds = %offload.pfor.body, %pfor.detach
  %inc24 = add i32 %i.0, 1, !UID !12
  br label %pfor.cond, !llvm.loop !13, !UID !15, !BB_UID !16

pfor.end25:                                       ; preds = %pfor.cond
  sync label %pfor.end.continue26, !UID !17, !BB_UID !18

pfor.end.continue26:                              ; preds = %pfor.end25
  %0 = load i32, i32* %result, align 4, !UID !19
  %div = sdiv i32 %0, 2, !UID !20
  store i32 %div, i32* %result, align 4, !UID !21
  %1 = load i32, i32* %result, align 4, !UID !22
  ret i32 %1, !UID !23, !BB_UID !24

offload.pfor.body:                                ; preds = %pfor.detach
  call void @cilk_for_test12_detach1(i32 %n, i32* %a, i32* %result)
  reattach label %pfor.inc23
}

; Function Attrs: noinline nounwind
define i32 @main() #0 !UID !1 {
entry:
  %a = alloca [8 x i32], align 4, !UID !2
  %0 = bitcast [8 x i32]* %a to i8*, !UID !3
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %0, i8* bitcast ([8 x i32]* @main.a to i8*), i32 32, i32 4, i1 false)
  %arraydecay = getelementptr inbounds [8 x i32], [8 x i32]* %a, i32 0, i32 0, !UID !4
  %call = call i32 @cilk_for_test12(i32* %arraydecay, i32 8), !UID !9
  br label %for.cond, !UID !6, !BB_UID !5

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ], !UID !8
  %cmp = icmp slt i32 %i.0, 8, !UID !11
  br i1 %cmp, label %for.body, label %for.end, !UID !10, !BB_UID !7

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds [8 x i32], [8 x i32]* %a, i32 0, i32 %i.0, !UID !25
  %1 = load i32, i32* %arrayidx, align 4, !UID !26
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %1)
  br label %for.inc, !UID !27, !BB_UID !28

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1, !UID !29
  br label %for.cond, !UID !30, !BB_UID !31

for.end:                                          ; preds = %for.cond
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.1, i32 0, i32 0), i32 %call)
  ret i32 0, !UID !32, !BB_UID !33
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture writeonly, i8* nocapture readonly, i32, i32, i1) #1

declare i32 @printf(i8*, ...) #2

define void @cilk_for_test12_detach3(i32* %a.in, i32 %k.0.in) {
my_pfor.body10:
  %0 = getelementptr inbounds i32, i32* %a.in, i32 %k.0.in, !UID !34
  %1 = load i32, i32* %0, align 4, !UID !35
  %2 = mul i32 2, %1, !UID !36
  %3 = getelementptr inbounds i32, i32* %a.in, i32 %k.0.in, !UID !37
  store i32 %2, i32* %3, align 4, !UID !38
  br label %my_pfor.preattach, !UID !39, !BB_UID !40

my_pfor.preattach:                                ; preds = %my_pfor.body10
  ret void
}

define void @cilk_for_test12_detach2(i32 %n.in, i32* %a.in) {
my_pfor.body5:
  br label %my_pfor.cond7, !UID !41, !BB_UID !32

my_pfor.cond7:                                    ; preds = %my_pfor.inc, %my_pfor.body5
  %0 = phi i32 [ 0, %my_pfor.body5 ], [ %2, %my_pfor.inc ], !UID !42
  %1 = icmp ult i32 %0, %n.in, !UID !43
  br i1 %1, label %my_pfor.detach9, label %my_pfor.end, !UID !44, !BB_UID !45

my_pfor.detach9:                                  ; preds = %my_pfor.cond7
  detach label %my_offload.pfor.body10, label %my_pfor.inc, !UID !46, !BB_UID !47

my_pfor.inc:                                      ; preds = %my_offload.pfor.body10, %my_pfor.detach9
  %2 = add i32 %0, 1, !UID !48
  br label %my_pfor.cond7, !llvm.loop !49, !UID !50, !BB_UID !51

my_pfor.end:                                      ; preds = %my_pfor.cond7
  sync label %my_pfor.end.continue, !UID !52, !BB_UID !53

my_pfor.end.continue:                             ; preds = %my_pfor.end
  %3 = sub i32 %n.in, 1, !UID !54
  %4 = getelementptr inbounds i32, i32* %a.in, i32 %3, !UID !55
  %5 = load i32, i32* %4, align 4, !UID !56
  %6 = add i32 %5, 1, !UID !57
  store i32 %6, i32* %4, align 4, !UID !58
  br label %my_pfor.preattach14, !UID !59, !BB_UID !60

my_pfor.preattach14:                              ; preds = %my_pfor.end.continue
  ret void

my_offload.pfor.body10:                           ; preds = %my_pfor.detach9
  call void @cilk_for_test12_detach3(i32* %a.in, i32 %0)
  reattach label %my_pfor.inc
}

define void @cilk_for_test12_detach1(i32 %n.in, i32* %a.in, i32* %result.in) {
my_pfor.body:
  br label %my_pfor.cond2, !UID !25, !BB_UID !28

my_pfor.cond2:                                    ; preds = %my_pfor.inc15, %my_pfor.body
  %0 = phi i32 [ 0, %my_pfor.body ], [ %2, %my_pfor.inc15 ], !UID !27
  %1 = icmp ult i32 %0, %n.in, !UID !31
  br i1 %1, label %my_pfor.detach4, label %my_pfor.end17, !UID !29, !BB_UID !26

my_pfor.detach4:                                  ; preds = %my_pfor.cond2
  detach label %my_offload.pfor.body5, label %my_pfor.inc15, !UID !33, !BB_UID !30

my_pfor.inc15:                                    ; preds = %my_offload.pfor.body5, %my_pfor.detach4
  %2 = add i32 %0, 1, !UID !61
  br label %my_pfor.cond2, !llvm.loop !62, !UID !63, !BB_UID !64

my_pfor.end17:                                    ; preds = %my_pfor.cond2
  sync label %my_pfor.end.continue18, !UID !65, !BB_UID !66

my_pfor.end.continue18:                           ; preds = %my_pfor.end17
  %3 = sub i32 %n.in, 1, !UID !67
  %4 = getelementptr inbounds i32, i32* %a.in, i32 %3, !UID !68
  %5 = load i32, i32* %4, align 4, !UID !69
  %6 = add i32 %5, 1, !UID !70
  store i32 %6, i32* %4, align 4, !UID !71
  %7 = load i32, i32* %result.in, align 4, !UID !72
  %8 = add i32 %7, %5, !UID !73
  store i32 %8, i32* %result.in, align 4, !UID !74
  br label %my_pfor.preattach22, !UID !75, !BB_UID !76

my_pfor.preattach22:                              ; preds = %my_pfor.end.continue18
  ret void

my_offload.pfor.body5:                            ; preds = %my_pfor.detach4
  call void @cilk_for_test12_detach2(i32 %n.in, i32* %a.in)
  reattach label %my_pfor.inc15
}

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 "}
!1 = !{!"0"}
!2 = !{!"2"}
!3 = !{!"3"}
!4 = !{!"4"}
!5 = !{!"1"}
!6 = !{!"6"}
!7 = !{!"7"}
!8 = !{!"8"}
!9 = !{!"5"}
!10 = !{!"10"}
!11 = !{!"9"}
!12 = !{!"68"}
!13 = distinct !{!13, !14}
!14 = !{!"tapir.loop.spawn.strategy", i32 1}
!15 = !{!"69"}
!16 = !{!"67"}
!17 = !{!"71"}
!18 = !{!"70"}
!19 = !{!"73"}
!20 = !{!"74"}
!21 = !{!"75"}
!22 = !{!"76"}
!23 = !{!"77"}
!24 = !{!"72"}
!25 = !{!"12"}
!26 = !{!"13"}
!27 = !{!"14"}
!28 = !{!"11"}
!29 = !{!"16"}
!30 = !{!"17"}
!31 = !{!"15"}
!32 = !{!"19"}
!33 = !{!"18"}
!34 = !{!"28"}
!35 = !{!"29"}
!36 = !{!"30"}
!37 = !{!"31"}
!38 = !{!"32"}
!39 = !{!"33"}
!40 = !{!"27"}
!41 = !{!"20"}
!42 = !{!"22"}
!43 = !{!"23"}
!44 = !{!"24"}
!45 = !{!"21"}
!46 = !{!"26"}
!47 = !{!"25"}
!48 = !{!"37"}
!49 = distinct !{!49, !14}
!50 = !{!"38"}
!51 = !{!"36"}
!52 = !{!"40"}
!53 = !{!"39"}
!54 = !{!"42"}
!55 = !{!"43"}
!56 = !{!"44"}
!57 = !{!"45"}
!58 = !{!"46"}
!59 = !{!"47"}
!60 = !{!"41"}
!61 = !{!"51"}
!62 = distinct !{!62, !14}
!63 = !{!"52"}
!64 = !{!"50"}
!65 = !{!"54"}
!66 = !{!"53"}
!67 = !{!"56"}
!68 = !{!"57"}
!69 = !{!"58"}
!70 = !{!"59"}
!71 = !{!"60"}
!72 = !{!"61"}
!73 = !{!"62"}
!74 = !{!"63"}
!75 = !{!"64"}
!76 = !{!"55"}
