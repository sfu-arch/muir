; ModuleID = 'trivial_for.bc'
source_filename = "trivial_for.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: noinline nounwind
define i32 @trivial_for() #0 !dbg !6 {
entry:
  %j = alloca i32, align 4
  call void @llvm.dbg.declare(metadata i32* %j, metadata !10, metadata !11), !dbg !12
  store i32 0, i32* %j, align 4, !dbg !12
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !13, metadata !11), !dbg !16
  call void @llvm.dbg.value(metadata i32 5, i64 0, metadata !17, metadata !11), !dbg !18
  br label %pfor.cond, !dbg !19

pfor.cond:                                        ; preds = %pfor.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %pfor.inc ]
  call void @llvm.dbg.value(metadata i32 %i.0, i64 0, metadata !13, metadata !11), !dbg !16
  %cmp = icmp slt i32 %i.0, 5, !dbg !20
  br i1 %cmp, label %pfor.detach, label %pfor.end, !dbg !23

pfor.detach:                                      ; preds = %pfor.cond
  detach label %pfor.body, label %pfor.inc, !dbg !25

pfor.body:                                        ; preds = %pfor.detach
  %0 = bitcast i32 undef to i32
  %sub = sub nsw i32 %i.0, 1, !dbg !27
  store i32 %sub, i32* %j, align 4, !dbg !29
  br label %pfor.preattach, !dbg !30

pfor.preattach:                                   ; preds = %pfor.body
  reattach label %pfor.inc, !dbg !31

pfor.inc:                                         ; preds = %pfor.preattach, %pfor.detach
  %inc = add nsw i32 %i.0, 1, !dbg !33
  call void @llvm.dbg.value(metadata i32 %inc, i64 0, metadata !13, metadata !11), !dbg !16
  br label %pfor.cond, !dbg !35, !llvm.loop !36

pfor.end:                                         ; preds = %pfor.cond
  sync label %pfor.end.continue, !dbg !39

pfor.end.continue:                                ; preds = %pfor.end
  %1 = load i32, i32* %j, align 4, !dbg !41
  ret i32 %1, !dbg !42
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind
define i32 @main() #0 !dbg !43 {
entry:
  %call = call i32 @trivial_for(), !dbg !46
  ret i32 0, !dbg !47
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!llvm.ident = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.0 ", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "trivial_for.c", directory: "/home/smargerm/amoeba/xketch-generator/test/c/cilk_for")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{!"clang version 4.0.0 "}
!6 = distinct !DISubprogram(name: "trivial_for", scope: !1, file: !1, line: 4, type: !7, isLocal: false, isDefinition: true, scopeLine: 4, isOptimized: false, unit: !0, variables: !2)
!7 = !DISubroutineType(types: !8)
!8 = !{!9}
!9 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!10 = !DILocalVariable(name: "j", scope: !6, file: !1, line: 5, type: !9)
!11 = !DIExpression()
!12 = !DILocation(line: 5, column: 12, scope: !6)
!13 = !DILocalVariable(name: "i", scope: !14, file: !1, line: 6, type: !15)
!14 = distinct !DILexicalBlock(scope: !6, file: !1, line: 6, column: 3)
!15 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!16 = !DILocation(line: 6, column: 17, scope: !14)
!17 = !DILocalVariable(name: "__end", scope: !14, type: !15, flags: DIFlagArtificial)
!18 = !DILocation(line: 0, scope: !14)
!19 = !DILocation(line: 6, column: 3, scope: !14)
!20 = !DILocation(line: 6, column: 26, scope: !21)
!21 = !DILexicalBlockFile(scope: !22, file: !1, discriminator: 1)
!22 = distinct !DILexicalBlock(scope: !14, file: !1, line: 6, column: 3)
!23 = !DILocation(line: 6, column: 3, scope: !24)
!24 = !DILexicalBlockFile(scope: !14, file: !1, discriminator: 1)
!25 = !DILocation(line: 6, column: 3, scope: !26)
!26 = !DILexicalBlockFile(scope: !14, file: !1, discriminator: 2)
!27 = !DILocation(line: 8, column: 8, scope: !28)
!28 = distinct !DILexicalBlock(scope: !22, file: !1, line: 6, column: 36)
!29 = !DILocation(line: 8, column: 6, scope: !28)
!30 = !DILocation(line: 9, column: 3, scope: !28)
!31 = !DILocation(line: 9, column: 3, scope: !32)
!32 = !DILexicalBlockFile(scope: !28, file: !1, discriminator: 1)
!33 = !DILocation(line: 6, column: 31, scope: !34)
!34 = !DILexicalBlockFile(scope: !22, file: !1, discriminator: 3)
!35 = !DILocation(line: 6, column: 3, scope: !34)
!36 = distinct !{!36, !19, !37, !38}
!37 = !DILocation(line: 9, column: 3, scope: !14)
!38 = !{!"tapir.loop.spawn.strategy", i32 1}
!39 = !DILocation(line: 6, column: 3, scope: !40)
!40 = !DILexicalBlockFile(scope: !22, file: !1, discriminator: 4)
!41 = !DILocation(line: 10, column: 10, scope: !6)
!42 = !DILocation(line: 10, column: 3, scope: !6)
!43 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 13, type: !44, isLocal: false, isDefinition: true, scopeLine: 13, isOptimized: false, unit: !0, variables: !2)
!44 = !DISubroutineType(types: !45)
!45 = !{!15}
!46 = !DILocation(line: 14, column: 3, scope: !43)
!47 = !DILocation(line: 15, column: 1, scope: !43)
