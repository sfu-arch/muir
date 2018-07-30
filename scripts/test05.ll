; ModuleID = 'test05.bc'
source_filename = "test05.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@main.a = private unnamed_addr constant [8 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7], align 4
@.str = private unnamed_addr constant [4 x i8] c"%u,\00", align 1
@.str.1 = private unnamed_addr constant [15 x i8] c"\0Areturned: %u\0A\00", align 1

; Function Attrs: noinline nounwind
define i32 @test05(i32* %a, i32 %n) #0 !dbg !7 {
entry:
  call void @llvm.dbg.value(metadata i32* %a, i64 0, metadata !12, metadata !13), !dbg !14
  call void @llvm.dbg.value(metadata i32 %n, i64 0, metadata !15, metadata !13), !dbg !16
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !17, metadata !13), !dbg !19
  br label %for.cond, !dbg !20

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  call void @llvm.dbg.value(metadata i32 %i.0, i64 0, metadata !17, metadata !13), !dbg !19
  %cmp = icmp ult i32 %i.0, %n, !dbg !21
  br i1 %cmp, label %for.body, label %for.end, !dbg !24

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds i32, i32* %a, i32 %i.0, !dbg !26
  %0 = load i32, i32* %arrayidx, align 4, !dbg !26
  %mul = mul i32 2, %0, !dbg !28
  %arrayidx1 = getelementptr inbounds i32, i32* %a, i32 %i.0, !dbg !29
  store i32 %mul, i32* %arrayidx1, align 4, !dbg !30
  br label %for.inc, !dbg !31

for.inc:                                          ; preds = %for.body
  %inc = add i32 %i.0, 1, !dbg !32
  call void @llvm.dbg.value(metadata i32 %inc, i64 0, metadata !17, metadata !13), !dbg !19
  br label %for.cond, !dbg !34, !llvm.loop !35

for.end:                                          ; preds = %for.cond
  %sub = sub i32 %n, 1, !dbg !38
  %arrayidx2 = getelementptr inbounds i32, i32* %a, i32 %sub, !dbg !39
  %1 = load i32, i32* %arrayidx2, align 4, !dbg !40
  %inc3 = add i32 %1, 1, !dbg !40
  store i32 %inc3, i32* %arrayidx2, align 4, !dbg !40
  %sub4 = sub i32 %n, 1, !dbg !41
  %arrayidx5 = getelementptr inbounds i32, i32* %a, i32 %sub4, !dbg !42
  %2 = load i32, i32* %arrayidx5, align 4, !dbg !42
  ret i32 %2, !dbg !43
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind
define i32 @test05b(i32* %a, i32 %n) #0 !dbg !44 {
entry:
  call void @llvm.dbg.value(metadata i32* %a, i64 0, metadata !45, metadata !13), !dbg !46
  call void @llvm.dbg.value(metadata i32 %n, i64 0, metadata !47, metadata !13), !dbg !48
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !49, metadata !13), !dbg !51
  br label %for.cond, !dbg !52

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  call void @llvm.dbg.value(metadata i32 %i.0, i64 0, metadata !49, metadata !13), !dbg !51
  %cmp = icmp ult i32 %i.0, %n, !dbg !53
  br i1 %cmp, label %for.body, label %for.end, !dbg !56

for.body:                                         ; preds = %for.cond
  %call = call i32 @test05(i32* %a, i32 %n), !dbg !58
  br label %for.inc, !dbg !60

for.inc:                                          ; preds = %for.body
  %inc = add i32 %i.0, 1, !dbg !61
  call void @llvm.dbg.value(metadata i32 %inc, i64 0, metadata !49, metadata !13), !dbg !51
  br label %for.cond, !dbg !63, !llvm.loop !64

for.end:                                          ; preds = %for.cond
  %sub = sub i32 %n, 1, !dbg !67
  %arrayidx = getelementptr inbounds i32, i32* %a, i32 %sub, !dbg !68
  %0 = load i32, i32* %arrayidx, align 4, !dbg !69
  %inc1 = add i32 %0, 1, !dbg !69
  store i32 %inc1, i32* %arrayidx, align 4, !dbg !69
  %sub2 = sub i32 %n, 1, !dbg !70
  %arrayidx3 = getelementptr inbounds i32, i32* %a, i32 %sub2, !dbg !71
  %1 = load i32, i32* %arrayidx3, align 4, !dbg !71
  ret i32 %1, !dbg !72
}

; Function Attrs: noinline nounwind
define i32 @test05c(i32* %a, i32 %n) #0 !dbg !73 {
entry:
  call void @llvm.dbg.value(metadata i32* %a, i64 0, metadata !74, metadata !13), !dbg !75
  call void @llvm.dbg.value(metadata i32 %n, i64 0, metadata !76, metadata !13), !dbg !77
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !78, metadata !13), !dbg !80
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !81, metadata !13), !dbg !83
  br label %for.cond, !dbg !84

for.cond:                                         ; preds = %for.inc, %entry
  %j.0 = phi i32 [ 0, %entry ], [ %add, %for.inc ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  call void @llvm.dbg.value(metadata i32 %i.0, i64 0, metadata !81, metadata !13), !dbg !83
  call void @llvm.dbg.value(metadata i32 %j.0, i64 0, metadata !78, metadata !13), !dbg !80
  %cmp = icmp ult i32 %i.0, 3, !dbg !85
  br i1 %cmp, label %for.body, label %for.end, !dbg !88

for.body:                                         ; preds = %for.cond
  %call = call i32 @test05b(i32* %a, i32 %n), !dbg !90
  %add = add i32 %j.0, %call, !dbg !92
  call void @llvm.dbg.value(metadata i32 %add, i64 0, metadata !78, metadata !13), !dbg !80
  br label %for.inc, !dbg !93

for.inc:                                          ; preds = %for.body
  %inc = add i32 %i.0, 1, !dbg !94
  call void @llvm.dbg.value(metadata i32 %inc, i64 0, metadata !81, metadata !13), !dbg !83
  br label %for.cond, !dbg !96, !llvm.loop !97

for.end:                                          ; preds = %for.cond
  %div = sdiv i32 %j.0, 2, !dbg !100
  call void @llvm.dbg.value(metadata i32 %div, i64 0, metadata !78, metadata !13), !dbg !80
  ret i32 %div, !dbg !101
}

; Function Attrs: noinline nounwind
define i32 @main() #0 !dbg !102 {
entry:
  %a = alloca [8 x i32], align 4
  call void @llvm.dbg.declare(metadata [8 x i32]* %a, metadata !105, metadata !13), !dbg !109
  %0 = bitcast [8 x i32]* %a to i8*, !dbg !109
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %0, i8* bitcast ([8 x i32]* @main.a to i8*), i32 32, i32 4, i1 false), !dbg !109
  %arraydecay = getelementptr inbounds [8 x i32], [8 x i32]* %a, i32 0, i32 0, !dbg !110
  %call = call i32 @test05c(i32* %arraydecay, i32 8), !dbg !111
  call void @llvm.dbg.value(metadata i32 %call, i64 0, metadata !112, metadata !13), !dbg !113
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !114, metadata !13), !dbg !116
  br label %for.cond, !dbg !117

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  call void @llvm.dbg.value(metadata i32 %i.0, i64 0, metadata !114, metadata !13), !dbg !116
  %cmp = icmp slt i32 %i.0, 8, !dbg !118
  br i1 %cmp, label %for.body, label %for.end, !dbg !121

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds [8 x i32], [8 x i32]* %a, i32 0, i32 %i.0, !dbg !123
  %1 = load i32, i32* %arrayidx, align 4, !dbg !123
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %1), !dbg !125
  br label %for.inc, !dbg !126

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1, !dbg !127
  call void @llvm.dbg.value(metadata i32 %inc, i64 0, metadata !114, metadata !13), !dbg !116
  br label %for.cond, !dbg !129, !llvm.loop !130

for.end:                                          ; preds = %for.cond
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.1, i32 0, i32 0), i32 %call), !dbg !133
  ret i32 0, !dbg !134
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture writeonly, i8* nocapture readonly, i32, i32, i1) #2

declare i32 @printf(i8*, ...) #3

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { argmemonly nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test05.c", directory: "/home/amiralis/git/xketch-master/test/c")
!2 = !{}
!3 = !{i32 1, !"NumRegisterParameters", i32 0}
!4 = !{i32 2, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "test05", scope: !1, file: !1, line: 4, type: !8, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !11, !10}
!10 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 32)
!12 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 4, type: !11)
!13 = !DIExpression()
!14 = !DILocation(line: 4, column: 27, scope: !7)
!15 = !DILocalVariable(name: "n", arg: 2, scope: !7, file: !1, line: 4, type: !10)
!16 = !DILocation(line: 4, column: 39, scope: !7)
!17 = !DILocalVariable(name: "i", scope: !18, file: !1, line: 5, type: !10)
!18 = distinct !DILexicalBlock(scope: !7, file: !1, line: 5, column: 3)
!19 = !DILocation(line: 5, column: 17, scope: !18)
!20 = !DILocation(line: 5, column: 8, scope: !18)
!21 = !DILocation(line: 5, column: 26, scope: !22)
!22 = !DILexicalBlockFile(scope: !23, file: !1, discriminator: 1)
!23 = distinct !DILexicalBlock(scope: !18, file: !1, line: 5, column: 3)
!24 = !DILocation(line: 5, column: 3, scope: !25)
!25 = !DILexicalBlockFile(scope: !18, file: !1, discriminator: 1)
!26 = !DILocation(line: 6, column: 14, scope: !27)
!27 = distinct !DILexicalBlock(scope: !23, file: !1, line: 5, column: 36)
!28 = !DILocation(line: 6, column: 13, scope: !27)
!29 = !DILocation(line: 6, column: 5, scope: !27)
!30 = !DILocation(line: 6, column: 10, scope: !27)
!31 = !DILocation(line: 7, column: 3, scope: !27)
!32 = !DILocation(line: 5, column: 31, scope: !33)
!33 = !DILexicalBlockFile(scope: !23, file: !1, discriminator: 2)
!34 = !DILocation(line: 5, column: 3, scope: !33)
!35 = distinct !{!35, !36, !37}
!36 = !DILocation(line: 5, column: 3, scope: !18)
!37 = !DILocation(line: 7, column: 3, scope: !18)
!38 = !DILocation(line: 8, column: 6, scope: !7)
!39 = !DILocation(line: 8, column: 3, scope: !7)
!40 = !DILocation(line: 8, column: 9, scope: !7)
!41 = !DILocation(line: 9, column: 13, scope: !7)
!42 = !DILocation(line: 9, column: 10, scope: !7)
!43 = !DILocation(line: 9, column: 3, scope: !7)
!44 = distinct !DISubprogram(name: "test05b", scope: !1, file: !1, line: 12, type: !8, isLocal: false, isDefinition: true, scopeLine: 12, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!45 = !DILocalVariable(name: "a", arg: 1, scope: !44, file: !1, line: 12, type: !11)
!46 = !DILocation(line: 12, column: 28, scope: !44)
!47 = !DILocalVariable(name: "n", arg: 2, scope: !44, file: !1, line: 12, type: !10)
!48 = !DILocation(line: 12, column: 40, scope: !44)
!49 = !DILocalVariable(name: "i", scope: !50, file: !1, line: 13, type: !10)
!50 = distinct !DILexicalBlock(scope: !44, file: !1, line: 13, column: 3)
!51 = !DILocation(line: 13, column: 17, scope: !50)
!52 = !DILocation(line: 13, column: 8, scope: !50)
!53 = !DILocation(line: 13, column: 26, scope: !54)
!54 = !DILexicalBlockFile(scope: !55, file: !1, discriminator: 1)
!55 = distinct !DILexicalBlock(scope: !50, file: !1, line: 13, column: 3)
!56 = !DILocation(line: 13, column: 3, scope: !57)
!57 = !DILexicalBlockFile(scope: !50, file: !1, discriminator: 1)
!58 = !DILocation(line: 14, column: 5, scope: !59)
!59 = distinct !DILexicalBlock(scope: !55, file: !1, line: 13, column: 36)
!60 = !DILocation(line: 15, column: 3, scope: !59)
!61 = !DILocation(line: 13, column: 31, scope: !62)
!62 = !DILexicalBlockFile(scope: !55, file: !1, discriminator: 2)
!63 = !DILocation(line: 13, column: 3, scope: !62)
!64 = distinct !{!64, !65, !66}
!65 = !DILocation(line: 13, column: 3, scope: !50)
!66 = !DILocation(line: 15, column: 3, scope: !50)
!67 = !DILocation(line: 16, column: 6, scope: !44)
!68 = !DILocation(line: 16, column: 3, scope: !44)
!69 = !DILocation(line: 16, column: 9, scope: !44)
!70 = !DILocation(line: 17, column: 13, scope: !44)
!71 = !DILocation(line: 17, column: 10, scope: !44)
!72 = !DILocation(line: 17, column: 3, scope: !44)
!73 = distinct !DISubprogram(name: "test05c", scope: !1, file: !1, line: 20, type: !8, isLocal: false, isDefinition: true, scopeLine: 20, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!74 = !DILocalVariable(name: "a", arg: 1, scope: !73, file: !1, line: 20, type: !11)
!75 = !DILocation(line: 20, column: 28, scope: !73)
!76 = !DILocalVariable(name: "n", arg: 2, scope: !73, file: !1, line: 20, type: !10)
!77 = !DILocation(line: 20, column: 40, scope: !73)
!78 = !DILocalVariable(name: "j", scope: !73, file: !1, line: 21, type: !79)
!79 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!80 = !DILocation(line: 21, column: 7, scope: !73)
!81 = !DILocalVariable(name: "i", scope: !82, file: !1, line: 22, type: !10)
!82 = distinct !DILexicalBlock(scope: !73, file: !1, line: 22, column: 3)
!83 = !DILocation(line: 22, column: 17, scope: !82)
!84 = !DILocation(line: 22, column: 8, scope: !82)
!85 = !DILocation(line: 22, column: 26, scope: !86)
!86 = !DILexicalBlockFile(scope: !87, file: !1, discriminator: 1)
!87 = distinct !DILexicalBlock(scope: !82, file: !1, line: 22, column: 3)
!88 = !DILocation(line: 22, column: 3, scope: !89)
!89 = !DILexicalBlockFile(scope: !82, file: !1, discriminator: 1)
!90 = !DILocation(line: 23, column: 10, scope: !91)
!91 = distinct !DILexicalBlock(scope: !87, file: !1, line: 22, column: 36)
!92 = !DILocation(line: 23, column: 7, scope: !91)
!93 = !DILocation(line: 24, column: 3, scope: !91)
!94 = !DILocation(line: 22, column: 31, scope: !95)
!95 = !DILexicalBlockFile(scope: !87, file: !1, discriminator: 2)
!96 = !DILocation(line: 22, column: 3, scope: !95)
!97 = distinct !{!97, !98, !99}
!98 = !DILocation(line: 22, column: 3, scope: !82)
!99 = !DILocation(line: 24, column: 3, scope: !82)
!100 = !DILocation(line: 25, column: 8, scope: !73)
!101 = !DILocation(line: 26, column: 3, scope: !73)
!102 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 30, type: !103, isLocal: false, isDefinition: true, scopeLine: 30, isOptimized: false, unit: !0, variables: !2)
!103 = !DISubroutineType(types: !104)
!104 = !{!79}
!105 = !DILocalVariable(name: "a", scope: !102, file: !1, line: 31, type: !106)
!106 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, size: 256, elements: !107)
!107 = !{!108}
!108 = !DISubrange(count: 8)
!109 = !DILocation(line: 31, column: 12, scope: !102)
!110 = !DILocation(line: 32, column: 21, scope: !102)
!111 = !DILocation(line: 32, column: 13, scope: !102)
!112 = !DILocalVariable(name: "foo", scope: !102, file: !1, line: 32, type: !79)
!113 = !DILocation(line: 32, column: 7, scope: !102)
!114 = !DILocalVariable(name: "i", scope: !115, file: !1, line: 33, type: !79)
!115 = distinct !DILexicalBlock(scope: !102, file: !1, line: 33, column: 3)
!116 = !DILocation(line: 33, column: 11, scope: !115)
!117 = !DILocation(line: 33, column: 7, scope: !115)
!118 = !DILocation(line: 33, column: 16, scope: !119)
!119 = !DILexicalBlockFile(scope: !120, file: !1, discriminator: 1)
!120 = distinct !DILexicalBlock(scope: !115, file: !1, line: 33, column: 3)
!121 = !DILocation(line: 33, column: 3, scope: !122)
!122 = !DILexicalBlockFile(scope: !115, file: !1, discriminator: 1)
!123 = !DILocation(line: 34, column: 19, scope: !124)
!124 = distinct !DILexicalBlock(scope: !120, file: !1, line: 33, column: 24)
!125 = !DILocation(line: 34, column: 5, scope: !124)
!126 = !DILocation(line: 35, column: 3, scope: !124)
!127 = !DILocation(line: 33, column: 20, scope: !128)
!128 = !DILexicalBlockFile(scope: !120, file: !1, discriminator: 2)
!129 = !DILocation(line: 33, column: 3, scope: !128)
!130 = distinct !{!130, !131, !132}
!131 = !DILocation(line: 33, column: 3, scope: !115)
!132 = !DILocation(line: 35, column: 3, scope: !115)
!133 = !DILocation(line: 36, column: 3, scope: !102)
!134 = !DILocation(line: 37, column: 3, scope: !102)
