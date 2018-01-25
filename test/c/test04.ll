; ModuleID = 'test04.bc'
source_filename = "test04.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: noinline nounwind
define i32 @foo(i32 %a, i32 %b, i32 %n) #0 !dbg !7 {
entry:
  call void @llvm.dbg.value(metadata i32 %a, i64 0, metadata !12, metadata !13), !dbg !14
  call void @llvm.dbg.value(metadata i32 %b, i64 0, metadata !15, metadata !13), !dbg !16
  call void @llvm.dbg.value(metadata i32 %n, i64 0, metadata !17, metadata !13), !dbg !18
  call void @llvm.dbg.value(metadata i32 %a, i64 0, metadata !19, metadata !13), !dbg !20
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !21, metadata !13), !dbg !22
  br label %for.cond, !dbg !23

for.cond:                                         ; preds = %for.inc4, %entry
  %sum.0 = phi i32 [ %a, %entry ], [ %sum.1, %for.inc4 ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc5, %for.inc4 ]
  call void @llvm.dbg.value(metadata i32 %i.0, i64 0, metadata !21, metadata !13), !dbg !22
  call void @llvm.dbg.value(metadata i32 %sum.0, i64 0, metadata !19, metadata !13), !dbg !20
  %cmp = icmp ult i32 %i.0, 10, !dbg !25
  br i1 %cmp, label %for.body, label %for.end6, !dbg !28

for.body:                                         ; preds = %for.cond
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !30, metadata !13), !dbg !31
  br label %for.cond1, !dbg !32

for.cond1:                                        ; preds = %for.inc, %for.body
  %sum.1 = phi i32 [ %sum.0, %for.body ], [ %mul, %for.inc ]
  %j.0 = phi i32 [ 0, %for.body ], [ %inc, %for.inc ]
  call void @llvm.dbg.value(metadata i32 %j.0, i64 0, metadata !30, metadata !13), !dbg !31
  call void @llvm.dbg.value(metadata i32 %sum.1, i64 0, metadata !19, metadata !13), !dbg !20
  %cmp2 = icmp ult i32 %j.0, 10, !dbg !35
  br i1 %cmp2, label %for.body3, label %for.end, !dbg !38

for.body3:                                        ; preds = %for.cond1
  %add = add i32 %sum.1, %a, !dbg !40
  %mul = mul i32 %add, %b, !dbg !42
  call void @llvm.dbg.value(metadata i32 %mul, i64 0, metadata !19, metadata !13), !dbg !20
  br label %for.inc, !dbg !43

for.inc:                                          ; preds = %for.body3
  %inc = add i32 %j.0, 1, !dbg !44
  call void @llvm.dbg.value(metadata i32 %inc, i64 0, metadata !30, metadata !13), !dbg !31
  br label %for.cond1, !dbg !46, !llvm.loop !47

for.end:                                          ; preds = %for.cond1
  br label %for.inc4, !dbg !50

for.inc4:                                         ; preds = %for.end
  %inc5 = add i32 %i.0, 1, !dbg !51
  call void @llvm.dbg.value(metadata i32 %inc5, i64 0, metadata !21, metadata !13), !dbg !22
  br label %for.cond, !dbg !53, !llvm.loop !54

for.end6:                                         ; preds = %for.cond
  ret i32 %sum.0, !dbg !57
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind
define i32 @main() #0 !dbg !58 {
entry:
  call void @llvm.dbg.value(metadata i32 5, i64 0, metadata !61, metadata !13), !dbg !62
  call void @llvm.dbg.value(metadata i32 3, i64 0, metadata !63, metadata !13), !dbg !64
  %call = call i32 @foo(i32 5, i32 3, i32 2), !dbg !65
  call void @llvm.dbg.value(metadata i32 %call, i64 0, metadata !66, metadata !13), !dbg !67
  ret i32 0, !dbg !68
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test04.c", directory: "/home/amiralis/git/xketch-generator/test/c")
!2 = !{}
!3 = !{i32 1, !"NumRegisterParameters", i32 0}
!4 = !{i32 2, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 4, type: !8, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !11, !11, !11}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!12 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 4, type: !11)
!13 = !DIExpression()
!14 = !DILocation(line: 4, column: 22, scope: !7)
!15 = !DILocalVariable(name: "b", arg: 2, scope: !7, file: !1, line: 4, type: !11)
!16 = !DILocation(line: 4, column: 38, scope: !7)
!17 = !DILocalVariable(name: "n", arg: 3, scope: !7, file: !1, line: 4, type: !11)
!18 = !DILocation(line: 4, column: 54, scope: !7)
!19 = !DILocalVariable(name: "sum", scope: !7, file: !1, line: 5, type: !11)
!20 = !DILocation(line: 5, column: 18, scope: !7)
!21 = !DILocalVariable(name: "i", scope: !7, file: !1, line: 6, type: !11)
!22 = !DILocation(line: 6, column: 14, scope: !7)
!23 = !DILocation(line: 8, column: 9, scope: !24)
!24 = distinct !DILexicalBlock(scope: !7, file: !1, line: 8, column: 5)
!25 = !DILocation(line: 8, column: 19, scope: !26)
!26 = !DILexicalBlockFile(scope: !27, file: !1, discriminator: 1)
!27 = distinct !DILexicalBlock(scope: !24, file: !1, line: 8, column: 5)
!28 = !DILocation(line: 8, column: 5, scope: !29)
!29 = !DILexicalBlockFile(scope: !24, file: !1, discriminator: 1)
!30 = !DILocalVariable(name: "j", scope: !7, file: !1, line: 6, type: !11)
!31 = !DILocation(line: 6, column: 16, scope: !7)
!32 = !DILocation(line: 9, column: 13, scope: !33)
!33 = distinct !DILexicalBlock(scope: !34, file: !1, line: 9, column: 9)
!34 = distinct !DILexicalBlock(scope: !27, file: !1, line: 8, column: 35)
!35 = !DILocation(line: 9, column: 22, scope: !36)
!36 = !DILexicalBlockFile(scope: !37, file: !1, discriminator: 1)
!37 = distinct !DILexicalBlock(scope: !33, file: !1, line: 9, column: 9)
!38 = !DILocation(line: 9, column: 9, scope: !39)
!39 = !DILexicalBlockFile(scope: !33, file: !1, discriminator: 1)
!40 = !DILocation(line: 10, column: 23, scope: !41)
!41 = distinct !DILexicalBlock(scope: !37, file: !1, line: 9, column: 38)
!42 = !DILocation(line: 10, column: 26, scope: !41)
!43 = !DILocation(line: 11, column: 9, scope: !41)
!44 = !DILocation(line: 9, column: 35, scope: !45)
!45 = !DILexicalBlockFile(scope: !37, file: !1, discriminator: 2)
!46 = !DILocation(line: 9, column: 9, scope: !45)
!47 = distinct !{!47, !48, !49}
!48 = !DILocation(line: 9, column: 9, scope: !33)
!49 = !DILocation(line: 11, column: 9, scope: !33)
!50 = !DILocation(line: 12, column: 5, scope: !34)
!51 = !DILocation(line: 8, column: 32, scope: !52)
!52 = !DILexicalBlockFile(scope: !27, file: !1, discriminator: 2)
!53 = !DILocation(line: 8, column: 5, scope: !52)
!54 = distinct !{!54, !55, !56}
!55 = !DILocation(line: 8, column: 5, scope: !24)
!56 = !DILocation(line: 12, column: 5, scope: !24)
!57 = !DILocation(line: 13, column: 5, scope: !7)
!58 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 16, type: !59, isLocal: false, isDefinition: true, scopeLine: 16, isOptimized: false, unit: !0, variables: !2)
!59 = !DISubroutineType(types: !60)
!60 = !{!10}
!61 = !DILocalVariable(name: "a", scope: !58, file: !1, line: 17, type: !11)
!62 = !DILocation(line: 17, column: 18, scope: !58)
!63 = !DILocalVariable(name: "b", scope: !58, file: !1, line: 18, type: !11)
!64 = !DILocation(line: 18, column: 18, scope: !58)
!65 = !DILocation(line: 19, column: 24, scope: !58)
!66 = !DILocalVariable(name: "sum", scope: !58, file: !1, line: 19, type: !11)
!67 = !DILocation(line: 19, column: 18, scope: !58)
!68 = !DILocation(line: 20, column: 5, scope: !58)
