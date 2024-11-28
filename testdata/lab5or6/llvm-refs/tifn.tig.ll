; ModuleID = 'lightir'
source_filename = "lightir"
target triple = "x86_64-pc-linux-gnu"

%string = type { i32, i8* }

@tigermain_framesize_global = constant i64 32
@g_framesize_global = constant i64 24
@0 = private unnamed_addr constant [21 x i8] c"hey! Bigger than 3!\0A\00", align 1
@str = constant %string { i32 20, i8* getelementptr inbounds ([21 x i8], [21 x i8]* @0, i32 0, i32 0) }
@1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.1 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @1, i32 0, i32 0) }

declare void @flush()

declare void @exit(i32)

declare %string* @chr(i32)

declare %string* @__wrap_getchar()

declare void @print(%string*)

declare void @printi(i32)

declare i32 @ord(%string*)

declare i32 @size(%string*)

declare %string* @concat(%string*, %string*)

declare %string* @substring(%string*, i32, i32)

declare i64 @alloc_record(i32)

declare i64 @init_array(i32, i64)

declare i1 @string_equal(%string*, %string*)

define i32 @tigermain(i64 %0, i64 %1) {
tigermain:
  %2 = load i64, i64* @tigermain_framesize_global, align 4
  %tigermain_sp = sub i64 %0, %2
  %3 = load i64, i64* @tigermain_framesize_global, align 4
  %4 = add i64 %3, -8
  %5 = add i64 %tigermain_sp, %4
  %6 = inttoptr i64 %5 to i32*
  store i32 5, i32* %6, align 4
  %7 = load i64, i64* @tigermain_framesize_global, align 4
  %8 = add i64 %7, -8
  %9 = add i64 %tigermain_sp, %8
  %tigermain_b_ptr = inttoptr i64 %9 to i32*
  %actual_parm = load i32, i32* %tigermain_b_ptr, align 4
  call void @printi(i32 %actual_parm)
  call void @print(%string* @str.1)
  call void @g(i64 %tigermain_sp, i64 %tigermain_sp, i32 2)
  %10 = load i64, i64* @tigermain_framesize_global, align 4
  %11 = add i64 %10, -8
  %12 = add i64 %tigermain_sp, %11
  %tigermain_b_ptr1 = inttoptr i64 %12 to i32*
  %actual_parm2 = load i32, i32* %tigermain_b_ptr1, align 4
  call void @printi(i32 %actual_parm2)
  ret i32 0
}

define void @g(i64 %0, i64 %1, i32 %2) {
g:
  %g_local_framesize = load i64, i64* @g_framesize_global, align 4
  %g_sp = sub i64 %0, %g_local_framesize
  %3 = load i64, i64* @g_framesize_global, align 4
  %4 = add i64 %3, 8
  %5 = add i64 %g_sp, %4
  %sl_ptr = inttoptr i64 %5 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %6 = load i64, i64* @g_framesize_global, align 4
  %7 = add i64 %6, 16
  %8 = add i64 %g_sp, %7
  %9 = inttoptr i64 %8 to i32*
  store i32 %2, i32* %9, align 4
  br label %if_test

if_test:                                          ; preds = %g
  %10 = load i64, i64* @g_framesize_global, align 4
  %11 = add i64 %10, 16
  %12 = add i64 %g_sp, %11
  %g_a_ptr = inttoptr i64 %12 to i32*
  %13 = load i32, i32* %g_a_ptr, align 4
  %14 = icmp sgt i32 %13, 3
  br i1 %14, label %if_then, label %if_else

if_then:                                          ; preds = %if_test
  call void @print(%string* @str)
  br label %if_next

if_else:                                          ; preds = %if_test
  %15 = load i64, i64* @g_framesize_global, align 4
  %16 = add i64 %15, 8
  %17 = add i64 %g_sp, %16
  %18 = inttoptr i64 %17 to i64*
  %19 = load i64, i64* %18, align 4
  %20 = load i64, i64* @tigermain_framesize_global, align 4
  %21 = add i64 %20, -8
  %22 = add i64 %19, %21
  %tigermain_b_ptr = inttoptr i64 %22 to i32*
  store i32 4, i32* %tigermain_b_ptr, align 4
  br label %if_next

if_next:                                          ; preds = %if_else, %if_then
  ret void
}

; ===== Frame Information =====
; tigermain -8 32
; g 0 24
