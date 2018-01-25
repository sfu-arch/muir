; ModuleID = 'test08.bc'
source_filename = "test08.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline nounwind
define i32 @test08(i32 %j) #0 !dbg !7 {
entry:
  call void @llvm.dbg.value(metadata i32 %j, i64 0, metadata !11, metadata !12), !dbg !13
  call void @llvm.dbg.value(metadata i32 %j, i64 0, metadata !14, metadata !12), !dbg !15
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !16, metadata !12), !dbg !18
  br label %for.cond, !dbg !19

for.cond:                                         ; preds = %for.inc, %entry
  %foo.0 = phi i32 [ %j, %entry ], [ %inc, %for.inc ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc1, %for.inc ]
  call void @llvm.dbg.value(metadata i32 %i.0, i64 0, metadata !16, metadata !12), !dbg !18
  call void @llvm.dbg.value(metadata i32 %foo.0, i64 0, metadata !14, metadata !12), !dbg !15
  %cmp = icmp ult i32 %i.0, 5, !dbg !20
  br i1 %cmp, label %for.body, label %for.end, !dbg !23

for.body:                                         ; preds = %for.cond
  %inc = add i32 %foo.0, 1, !dbg !25
  call void @llvm.dbg.value(metadata i32 %inc, i64 0, metadata !14, metadata !12), !dbg !15
  br label %for.inc, !dbg !27

for.inc:                                          ; preds = %for.body
  %inc1 = add i32 %i.0, 1, !dbg !28
  call void @llvm.dbg.value(metadata i32 %inc1, i64 0, metadata !16, metadata !12), !dbg !18
  br label %for.cond, !dbg !30, !llvm.loop !31

for.end:                                          ; preds = %for.cond
  ret i32 %foo.0, !dbg !34
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind
define i32 @main() #0 !dbg !35 {
entry:
  %call = call i32 @test08(i32 100), !dbg !39
  call void @llvm.dbg.value(metadata i32 %call, i64 0, metadata !40, metadata !12), !dbg !41
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %call), !dbg !42
  ret i32 0, !dbg !43
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
!1 = !DIFile(filename: "test08.c", directory: "/home/amiralis/git/xketch-generator/test/c")
!2 = !{}
!3 = !{i32 1, !"NumRegisterParameters", i32 0}
!4 = !{i32 2, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "test08", scope: !1, file: !1, line: 3, type: !8, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !10}
!10 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!11 = !DILocalVariable(name: "j", arg: 1, scope: !7, file: !1, line: 3, type: !10)
!12 = !DIExpression()
!13 = !DILocation(line: 3, column: 26, scope: !7)
!14 = !DILocalVariable(name: "foo", scope: !7, file: !1, line: 4, type: !10)
!15 = !DILocation(line: 4, column: 12, scope: !7)
!16 = !DILocalVariable(name: "i", scope: !17, file: !1, line: 5, type: !10)
!17 = distinct !DILexicalBlock(scope: !7, file: !1, line: 5, column: 3)
!18 = !DILocation(line: 5, column: 17, scope: !17)
!19 = !DILocation(line: 5, column: 8, scope: !17)
!20 = !DILocation(line: 5, column: 26, scope: !21)
!21 = !DILexicalBlockFile(scope: !22, file: !1, discriminator: 1)
!22 = distinct !DILexicalBlock(scope: !17, file: !1, line: 5, column: 3)
!23 = !DILocation(line: 5, column: 3, scope: !24)
!24 = !DILexicalBlockFile(scope: !17, file: !1, discriminator: 1)
!25 = !DILocation(line: 6, column: 8, scope: !26)
!26 = distinct !DILexicalBlock(scope: !22, file: !1, line: 5, column: 36)
!27 = !DILocation(line: 7, column: 3, scope: !26)
!28 = !DILocation(line: 5, column: 31, scope: !29)
!29 = !DILexicalBlockFile(scope: !22, file: !1, discriminator: 2)
!30 = !DILocation(line: 5, column: 3, scope: !29)
!31 = distinct !{!31, !32, !33}
!32 = !DILocation(line: 5, column: 3, scope: !17)
!33 = !DILocation(line: 7, column: 3, scope: !17)
!34 = !DILocation(line: 8, column: 3, scope: !7)
!35 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 11, type: !36, isLocal: false, isDefinition: true, scopeLine: 11, isOptimized: false, unit: !0, variables: !2)
!36 = !DISubroutineType(types: !37)
!37 = !{!38}
!38 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!39 = !DILocation(line: 12, column: 16, scope: !35)
!40 = !DILocalVariable(name: "result", scope: !35, file: !1, line: 12, type: !38)
!41 = !DILocation(line: 12, column: 7, scope: !35)
!42 = !DILocation(line: 13, column: 3, scope: !35)
!43 = !DILocation(line: 14, column: 1, scope: !35)
