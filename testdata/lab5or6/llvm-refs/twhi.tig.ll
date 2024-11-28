; ModuleID = 'lightir'
source_filename = "lightir"
target triple = "x86_64-pc-linux-gnu"

%string = type { i32, i8* }

@tigermain_framesize_global = constant i64 32

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
  store i32 10, i32* %6, align 4
  br label %while_test

while_test:                                       ; preds = %if_next, %tigermain
  %7 = load i64, i64* @tigermain_framesize_global, align 4
  %8 = add i64 %7, -8
  %9 = add i64 %tigermain_sp, %8
  %tigermain_a_ptr = inttoptr i64 %9 to i32*
  %10 = load i32, i32* %tigermain_a_ptr, align 4
  %11 = icmp sge i32 %10, 0
  br i1 %11, label %while_body, label %while_next

while_body:                                       ; preds = %while_test
  %12 = load i64, i64* @tigermain_framesize_global, align 4
  %13 = add i64 %12, -8
  %14 = add i64 %tigermain_sp, %13
  %tigermain_a_ptr1 = inttoptr i64 %14 to i32*
  %actual_parm = load i32, i32* %tigermain_a_ptr1, align 4
  call void @printi(i32 %actual_parm)
  %15 = load i64, i64* @tigermain_framesize_global, align 4
  %16 = add i64 %15, -8
  %17 = add i64 %tigermain_sp, %16
  %tigermain_a_ptr2 = inttoptr i64 %17 to i32*
  %18 = load i64, i64* @tigermain_framesize_global, align 4
  %19 = add i64 %18, -8
  %20 = add i64 %tigermain_sp, %19
  %tigermain_a_ptr3 = inttoptr i64 %20 to i32*
  %21 = load i32, i32* %tigermain_a_ptr3, align 4
  %22 = sub i32 %21, 1
  store i32 %22, i32* %tigermain_a_ptr2, align 4
  br label %if_test

while_next:                                       ; preds = %if_then, %while_test
  ret i32 0

if_test:                                          ; preds = %while_body
  %23 = load i64, i64* @tigermain_framesize_global, align 4
  %24 = add i64 %23, -8
  %25 = add i64 %tigermain_sp, %24
  %tigermain_a_ptr4 = inttoptr i64 %25 to i32*
  %26 = load i32, i32* %tigermain_a_ptr4, align 4
  %27 = icmp eq i32 %26, 2
  br i1 %27, label %if_then, label %if_next

if_then:                                          ; preds = %if_test
  br label %while_next

if_next:                                          ; preds = %if_test
  br label %while_test
}

; ===== Frame Information =====
; tigermain -8 32
