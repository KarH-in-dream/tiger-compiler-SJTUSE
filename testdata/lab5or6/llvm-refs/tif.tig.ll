; ModuleID = 'lightir'
source_filename = "lightir"
target triple = "x86_64-pc-linux-gnu"

%string = type { i32, i8* }

@tigermain_framesize_global = constant i64 32
@g_framesize_global = constant i64 16

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
  %3 = call i32 @g(i64 %tigermain_sp, i64 %tigermain_sp, i32 9, i32 4)
  call void @printi(i32 %3)
  ret i32 0
}

define i32 @g(i64 %0, i64 %1, i32 %2, i32 %3) {
g:
  %g_local_framesize = load i64, i64* @g_framesize_global, align 4
  %g_sp = sub i64 %0, %g_local_framesize
  %4 = load i64, i64* @g_framesize_global, align 4
  %5 = add i64 %4, 8
  %6 = add i64 %g_sp, %5
  %sl_ptr = inttoptr i64 %6 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %7 = load i64, i64* @g_framesize_global, align 4
  %8 = add i64 %7, 16
  %9 = add i64 %g_sp, %8
  %10 = inttoptr i64 %9 to i32*
  store i32 %2, i32* %10, align 4
  %11 = load i64, i64* @g_framesize_global, align 4
  %12 = add i64 %11, 24
  %13 = add i64 %g_sp, %12
  %14 = inttoptr i64 %13 to i32*
  store i32 %3, i32* %14, align 4
  br label %if_test

if_test:                                          ; preds = %g
  %15 = load i64, i64* @g_framesize_global, align 4
  %16 = add i64 %15, 16
  %17 = add i64 %g_sp, %16
  %g_a_ptr = inttoptr i64 %17 to i32*
  %18 = load i64, i64* @g_framesize_global, align 4
  %19 = add i64 %18, 24
  %20 = add i64 %g_sp, %19
  %g_b_ptr = inttoptr i64 %20 to i32*
  %21 = load i32, i32* %g_a_ptr, align 4
  %22 = load i32, i32* %g_b_ptr, align 4
  %23 = icmp sgt i32 %21, %22
  br i1 %23, label %if_then, label %if_else

if_then:                                          ; preds = %if_test
  %24 = load i64, i64* @g_framesize_global, align 4
  %25 = add i64 %24, 16
  %26 = add i64 %g_sp, %25
  %g_a_ptr1 = inttoptr i64 %26 to i32*
  br label %if_next

if_else:                                          ; preds = %if_test
  %27 = load i64, i64* @g_framesize_global, align 4
  %28 = add i64 %27, 24
  %29 = add i64 %g_sp, %28
  %g_b_ptr2 = inttoptr i64 %29 to i32*
  br label %if_next

if_next:                                          ; preds = %if_else, %if_then
  %30 = phi i32* [ %g_a_ptr1, %if_then ], [ %g_b_ptr2, %if_else ]
  %31 = load i32, i32* %30, align 4
  ret i32 %31
}

; ===== Frame Information =====
; tigermain 0 32
; g 0 16
