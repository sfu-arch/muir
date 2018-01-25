; ModuleID = 'test05.bc'
source_filename = "test05.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: noinline nounwind
define i32 @foo(i32* %a) #0 !dbg !7 {
entry:
  call void @llvm.dbg.value(metadata i32* %a, i64 0, metadata !12, metadata !13), !dbg !14
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !15, metadata !13), !dbg !17
  br label %for.cond, !dbg !18

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  call void @llvm.dbg.value(metadata i32 %i.0, i64 0, metadata !15, metadata !13), !dbg !17
  %cmp = icmp ult i32 %i.0, 5, !dbg !19
  br i1 %cmp, label %for.body, label %for.end, !dbg !22

for.body:                                         ; preds = %for.cond
  call void @llvm.dbg.value(metadata i32 %i.0, i64 0, metadata !24, metadata !13), !dbg !28
  call void @llvm.dbg.value(metadata i32* %a, i64 0, metadata !31, metadata !13), !dbg !32
  %arrayidx.i = getelementptr inbounds i32, i32* %a, i32 %i.0, !dbg !33
  %0 = load i32, i32* %arrayidx.i, align 4, !dbg !33
  %mul.i = mul i32 2, %0, !dbg !34
  %add.i = add i32 %i.0, 5, !dbg !35
  %arrayidx1.i = getelementptr inbounds i32, i32* %a, i32 %add.i, !dbg !36
  store i32 %mul.i, i32* %arrayidx1.i, align 4, !dbg !37
  br label %for.inc, !dbg !38

for.inc:                                          ; preds = %for.body
  %inc = add i32 %i.0, 1, !dbg !39
  call void @llvm.dbg.value(metadata i32 %inc, i64 0, metadata !15, metadata !13), !dbg !17
  br label %for.cond, !dbg !41, !llvm.loop !42

for.end:                                          ; preds = %for.cond
  %arrayidx = getelementptr inbounds i32, i32* %a, i32 5, !dbg !45
  %1 = load i32, i32* %arrayidx, align 4, !dbg !45
  ret i32 %1, !dbg !46
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind
define i32 @main() #0 !dbg !47 {
entry:
  %a = alloca [10 x i32], align 4
  call void @llvm.dbg.declare(metadata [10 x i32]* %a, metadata !51, metadata !13), !dbg !55
  %0 = bitcast [10 x i32]* %a to i8*, !dbg !55
  call void @llvm.memset.p0i8.i32(i8* %0, i8 0, i32 40, i32 4, i1 false), !dbg !55
  %1 = bitcast i8* %0 to [10 x i32]*, !dbg !55
  %2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 1, !dbg !55
  store i32 1, i32* %2, !dbg !55
  %3 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 2, !dbg !55
  store i32 2, i32* %3, !dbg !55
  %4 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 3, !dbg !55
  store i32 3, i32* %4, !dbg !55
  %5 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 4, !dbg !55
  store i32 4, i32* %5, !dbg !55
  %arraydecay = getelementptr inbounds [10 x i32], [10 x i32]* %a, i32 0, i32 0, !dbg !56
  %call = call i32 @foo(i32* %arraydecay), !dbg !57
  ret i32 0, !dbg !58
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i32(i8* nocapture writeonly, i8, i32, i32, i1) #2

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test05.c", directory: "/home/amiralis/git/xketch-generator/test/c")
!2 = !{}
!3 = !{i32 1, !"NumRegisterParameters", i32 0}
!4 = !{i32 2, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 8, type: !8, isLocal: false, isDefinition: true, scopeLine: 8, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !11}
!10 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 32)
!12 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 8, type: !11)
!13 = !DIExpression()
!14 = !DILocation(line: 8, column: 24, scope: !7)
!15 = !DILocalVariable(name: "i", scope: !16, file: !1, line: 9, type: !10)
!16 = distinct !DILexicalBlock(scope: !7, file: !1, line: 9, column: 3)
!17 = !DILocation(line: 9, column: 17, scope: !16)
!18 = !DILocation(line: 9, column: 8, scope: !16)
!19 = !DILocation(line: 9, column: 26, scope: !20)
!20 = !DILexicalBlockFile(scope: !21, file: !1, discriminator: 1)
!21 = distinct !DILexicalBlock(scope: !16, file: !1, line: 9, column: 3)
!22 = !DILocation(line: 9, column: 3, scope: !23)
!23 = !DILexicalBlockFile(scope: !16, file: !1, discriminator: 1)
!24 = !DILocalVariable(name: "i", arg: 1, scope: !25, file: !1, line: 4, type: !10)
!25 = distinct !DISubprogram(name: "do_work", scope: !1, file: !1, line: 4, type: !26, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!26 = !DISubroutineType(types: !27)
!27 = !{null, !10, !11}
!28 = !DILocation(line: 4, column: 39, scope: !25, inlinedAt: !29)
!29 = distinct !DILocation(line: 10, column: 5, scope: !30)
!30 = distinct !DILexicalBlock(scope: !21, file: !1, line: 9, column: 36)
!31 = !DILocalVariable(name: "a", arg: 2, scope: !25, file: !1, line: 4, type: !11)
!32 = !DILocation(line: 4, column: 52, scope: !25, inlinedAt: !29)
!33 = !DILocation(line: 5, column: 14, scope: !25, inlinedAt: !29)
!34 = !DILocation(line: 5, column: 13, scope: !25, inlinedAt: !29)
!35 = !DILocation(line: 5, column: 6, scope: !25, inlinedAt: !29)
!36 = !DILocation(line: 5, column: 3, scope: !25, inlinedAt: !29)
!37 = !DILocation(line: 5, column: 10, scope: !25, inlinedAt: !29)
!38 = !DILocation(line: 11, column: 3, scope: !30)
!39 = !DILocation(line: 9, column: 31, scope: !40)
!40 = !DILexicalBlockFile(scope: !21, file: !1, discriminator: 2)
!41 = !DILocation(line: 9, column: 3, scope: !40)
!42 = distinct !{!42, !43, !44}
!43 = !DILocation(line: 9, column: 3, scope: !16)
!44 = !DILocation(line: 11, column: 3, scope: !16)
!45 = !DILocation(line: 13, column: 10, scope: !7)
!46 = !DILocation(line: 13, column: 3, scope: !7)
!47 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 16, type: !48, isLocal: false, isDefinition: true, scopeLine: 16, isOptimized: false, unit: !0, variables: !2)
!48 = !DISubroutineType(types: !49)
!49 = !{!50}
!50 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!51 = !DILocalVariable(name: "a", scope: !47, file: !1, line: 18, type: !52)
!52 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, size: 320, elements: !53)
!53 = !{!54}
!54 = !DISubrange(count: 10)
!55 = !DILocation(line: 18, column: 12, scope: !47)
!56 = !DILocation(line: 20, column: 7, scope: !47)
!57 = !DILocation(line: 20, column: 3, scope: !47)
!58 = !DILocation(line: 22, column: 1, scope: !47)
