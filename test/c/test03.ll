; ModuleID = 'test03.bc'
source_filename = "test03.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"sum=%u\0A\00", align 1

; Function Attrs: noinline nounwind
define i32 @test03(i32 %a, i32 %b, i32 %n) #0 !dbg !7 {
entry:
  call void @llvm.dbg.value(metadata i32 %a, i64 0, metadata !12, metadata !13), !dbg !14
  call void @llvm.dbg.value(metadata i32 %b, i64 0, metadata !15, metadata !13), !dbg !16
  call void @llvm.dbg.value(metadata i32 %n, i64 0, metadata !17, metadata !13), !dbg !18
  call void @llvm.dbg.value(metadata i32 %a, i64 0, metadata !19, metadata !13), !dbg !20
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !21, metadata !13), !dbg !22
  br label %for.cond, !dbg !23

for.cond:                                         ; preds = %for.inc, %entry
  %sum.0 = phi i32 [ %a, %entry ], [ %mul, %for.inc ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  call void @llvm.dbg.value(metadata i32 %i.0, i64 0, metadata !21, metadata !13), !dbg !22
  call void @llvm.dbg.value(metadata i32 %sum.0, i64 0, metadata !19, metadata !13), !dbg !20
  %cmp = icmp slt i32 %i.0, %n, !dbg !25
  br i1 %cmp, label %for.body, label %for.end, !dbg !28

for.body:                                         ; preds = %for.cond
  %add = add i32 %sum.0, %a, !dbg !30
  %mul = mul i32 %add, %b, !dbg !31
  call void @llvm.dbg.value(metadata i32 %mul, i64 0, metadata !19, metadata !13), !dbg !20
  br label %for.inc, !dbg !32

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1, !dbg !33
  call void @llvm.dbg.value(metadata i32 %inc, i64 0, metadata !21, metadata !13), !dbg !22
  br label %for.cond, !dbg !35, !llvm.loop !36

for.end:                                          ; preds = %for.cond
  ret i32 %sum.0, !dbg !39
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind
define i32 @main() #0 !dbg !40 {
entry:
  call void @llvm.dbg.value(metadata i32 5, i64 0, metadata !43, metadata !13), !dbg !44
  call void @llvm.dbg.value(metadata i32 3, i64 0, metadata !45, metadata !13), !dbg !46
  %call = call i32 @test03(i32 5, i32 3, i32 2), !dbg !47
  call void @llvm.dbg.value(metadata i32 %call, i64 0, metadata !48, metadata !13), !dbg !49
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 %call), !dbg !50
  ret i32 0, !dbg !51
}

declare i32 @printf(i8*, ...) #2

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test03.c", directory: "/home/amiralis/git/xketch-generator/test/c")
!2 = !{}
!3 = !{i32 1, !"NumRegisterParameters", i32 0}
!4 = !{i32 2, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "test03", scope: !1, file: !1, line: 3, type: !8, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !11, !11, !10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!12 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 3, type: !11)
!13 = !DIExpression()
!14 = !DILocation(line: 3, column: 25, scope: !7)
!15 = !DILocalVariable(name: "b", arg: 2, scope: !7, file: !1, line: 3, type: !11)
!16 = !DILocation(line: 3, column: 41, scope: !7)
!17 = !DILocalVariable(name: "n", arg: 3, scope: !7, file: !1, line: 3, type: !10)
!18 = !DILocation(line: 3, column: 48, scope: !7)
!19 = !DILocalVariable(name: "sum", scope: !7, file: !1, line: 4, type: !11)
!20 = !DILocation(line: 4, column: 18, scope: !7)
!21 = !DILocalVariable(name: "i", scope: !7, file: !1, line: 5, type: !10)
!22 = !DILocation(line: 5, column: 9, scope: !7)
!23 = !DILocation(line: 7, column: 9, scope: !24)
!24 = distinct !DILexicalBlock(scope: !7, file: !1, line: 7, column: 5)
!25 = !DILocation(line: 7, column: 19, scope: !26)
!26 = !DILexicalBlockFile(scope: !27, file: !1, discriminator: 1)
!27 = distinct !DILexicalBlock(scope: !24, file: !1, line: 7, column: 5)
!28 = !DILocation(line: 7, column: 5, scope: !29)
!29 = !DILexicalBlockFile(scope: !24, file: !1, discriminator: 1)
!30 = !DILocation(line: 8, column: 19, scope: !27)
!31 = !DILocation(line: 8, column: 22, scope: !27)
!32 = !DILocation(line: 8, column: 9, scope: !27)
!33 = !DILocation(line: 7, column: 25, scope: !34)
!34 = !DILexicalBlockFile(scope: !27, file: !1, discriminator: 2)
!35 = !DILocation(line: 7, column: 5, scope: !34)
!36 = distinct !{!36, !37, !38}
!37 = !DILocation(line: 7, column: 5, scope: !24)
!38 = !DILocation(line: 8, column: 23, scope: !24)
!39 = !DILocation(line: 9, column: 5, scope: !7)
!40 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 12, type: !41, isLocal: false, isDefinition: true, scopeLine: 12, isOptimized: false, unit: !0, variables: !2)
!41 = !DISubroutineType(types: !42)
!42 = !{!10}
!43 = !DILocalVariable(name: "a", scope: !40, file: !1, line: 13, type: !11)
!44 = !DILocation(line: 13, column: 18, scope: !40)
!45 = !DILocalVariable(name: "b", scope: !40, file: !1, line: 14, type: !11)
!46 = !DILocation(line: 14, column: 18, scope: !40)
!47 = !DILocation(line: 15, column: 24, scope: !40)
!48 = !DILocalVariable(name: "sum", scope: !40, file: !1, line: 15, type: !11)
!49 = !DILocation(line: 15, column: 18, scope: !40)
!50 = !DILocation(line: 16, column: 5, scope: !40)
!51 = !DILocation(line: 17, column: 5, scope: !40)
