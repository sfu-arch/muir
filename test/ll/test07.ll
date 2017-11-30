; ModuleID = 'test07.bc'
source_filename = "test07.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: noinline nounwind
define i32 @test07() #0 !dbg !7 {
entry:
  call void @llvm.dbg.value(metadata i32 100, i64 0, metadata !11, metadata !12), !dbg !13
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !14, metadata !12), !dbg !17
  br label %for.cond, !dbg !18

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %j.0 = phi i32 [ 100, %entry ], [ %sub, %for.inc ]
  call void @llvm.dbg.value(metadata i32 %j.0, i64 0, metadata !11, metadata !12), !dbg !13
  call void @llvm.dbg.value(metadata i32 %i.0, i64 0, metadata !14, metadata !12), !dbg !17
  %cmp = icmp slt i32 %i.0, 5, !dbg !19
  br i1 %cmp, label %for.body, label %for.end, !dbg !22

for.body:                                         ; preds = %for.cond
  %sub = sub i32 %j.0, 1, !dbg !24
  call void @llvm.dbg.value(metadata i32 %sub, i64 0, metadata !11, metadata !12), !dbg !13
  br label %for.inc, !dbg !26

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1, !dbg !27
  call void @llvm.dbg.value(metadata i32 %inc, i64 0, metadata !14, metadata !12), !dbg !17
  br label %for.cond, !dbg !29, !llvm.loop !30

for.end:                                          ; preds = %for.cond
  ret i32 %j.0, !dbg !33
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind
define i32 @main() #0 !dbg !34 {
entry:
  %call = call i32 @test07(), !dbg !37
  ret i32 0, !dbg !38
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test07.c", directory: "/home/smargerm/amoeba/xketch-generator/test/c")
!2 = !{}
!3 = !{i32 1, !"NumRegisterParameters", i32 0}
!4 = !{i32 2, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "test07", scope: !1, file: !1, line: 1, type: !8, isLocal: false, isDefinition: true, scopeLine: 1, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10}
!10 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!11 = !DILocalVariable(name: "j", scope: !7, file: !1, line: 2, type: !10)
!12 = !DIExpression()
!13 = !DILocation(line: 2, column: 12, scope: !7)
!14 = !DILocalVariable(name: "i", scope: !15, file: !1, line: 3, type: !16)
!15 = distinct !DILexicalBlock(scope: !7, file: !1, line: 3, column: 3)
!16 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!17 = !DILocation(line: 3, column: 12, scope: !15)
!18 = !DILocation(line: 3, column: 8, scope: !15)
!19 = !DILocation(line: 3, column: 21, scope: !20)
!20 = !DILexicalBlockFile(scope: !21, file: !1, discriminator: 1)
!21 = distinct !DILexicalBlock(scope: !15, file: !1, line: 3, column: 3)
!22 = !DILocation(line: 3, column: 3, scope: !23)
!23 = !DILexicalBlockFile(scope: !15, file: !1, discriminator: 1)
!24 = !DILocation(line: 4, column: 8, scope: !25)
!25 = distinct !DILexicalBlock(scope: !21, file: !1, line: 3, column: 31)
!26 = !DILocation(line: 5, column: 3, scope: !25)
!27 = !DILocation(line: 3, column: 26, scope: !28)
!28 = !DILexicalBlockFile(scope: !21, file: !1, discriminator: 2)
!29 = !DILocation(line: 3, column: 3, scope: !28)
!30 = distinct !{!30, !31, !32}
!31 = !DILocation(line: 3, column: 3, scope: !15)
!32 = !DILocation(line: 5, column: 3, scope: !15)
!33 = !DILocation(line: 6, column: 3, scope: !7)
!34 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 9, type: !35, isLocal: false, isDefinition: true, scopeLine: 9, isOptimized: false, unit: !0, variables: !2)
!35 = !DISubroutineType(types: !36)
!36 = !{!16}
!37 = !DILocation(line: 10, column: 3, scope: !34)
!38 = !DILocation(line: 11, column: 1, scope: !34)
