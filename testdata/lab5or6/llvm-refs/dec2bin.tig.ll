; ModuleID = 'lightir'
source_filename = "lightir"
target triple = "x86_64-pc-linux-gnu"

%string = type { i32, i8* }

@tigermain_framesize_global = constant i64 16
@dec2bin_framesize_global = constant i64 24
@try_framesize_global = constant i64 24
@0 = private unnamed_addr constant [5 x i8] c"\09->\09\00", align 1
@str = constant %string { i32 4, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @0, i32 0, i32 0) }
@1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.1 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @1, i32 0, i32 0) }
@2 = private unnamed_addr constant [5 x i8] c"\09->\09\00", align 1
@str.2 = constant %string { i32 4, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @2, i32 0, i32 0) }
@3 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.3 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @3, i32 0, i32 0) }
@4 = private unnamed_addr constant [5 x i8] c"\09->\09\00", align 1
@str.4 = constant %string { i32 4, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @4, i32 0, i32 0) }
@5 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.5 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @5, i32 0, i32 0) }
@6 = private unnamed_addr constant [5 x i8] c"\09->\09\00", align 1
@str.6 = constant %string { i32 4, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @6, i32 0, i32 0) }
@7 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.7 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @7, i32 0, i32 0) }

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
  call void @try(i64 %tigermain_sp, i64 %tigermain_sp)
  ret i32 0
}

define void @dec2bin(i64 %0, i64 %1, i32 %2) {
dec2bin:
  %dec2bin_local_framesize = load i64, i64* @dec2bin_framesize_global, align 4
  %dec2bin_sp = sub i64 %0, %dec2bin_local_framesize
  %3 = load i64, i64* @dec2bin_framesize_global, align 4
  %4 = add i64 %3, 8
  %5 = add i64 %dec2bin_sp, %4
  %sl_ptr = inttoptr i64 %5 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %6 = load i64, i64* @dec2bin_framesize_global, align 4
  %7 = add i64 %6, 16
  %8 = add i64 %dec2bin_sp, %7
  %9 = inttoptr i64 %8 to i32*
  store i32 %2, i32* %9, align 4
  br label %if_test

if_test:                                          ; preds = %dec2bin
  %10 = load i64, i64* @dec2bin_framesize_global, align 4
  %11 = add i64 %10, 16
  %12 = add i64 %dec2bin_sp, %11
  %dec2bin_num_ptr = inttoptr i64 %12 to i32*
  %13 = load i32, i32* %dec2bin_num_ptr, align 4
  %14 = icmp sgt i32 %13, 0
  br i1 %14, label %if_then, label %if_next

if_then:                                          ; preds = %if_test
  %15 = load i64, i64* @dec2bin_framesize_global, align 4
  %16 = add i64 %15, 16
  %17 = add i64 %dec2bin_sp, %16
  %dec2bin_num_ptr1 = inttoptr i64 %17 to i32*
  %18 = load i32, i32* %dec2bin_num_ptr1, align 4
  %19 = sdiv i32 %18, 2
  %20 = load i64, i64* @dec2bin_framesize_global, align 4
  %21 = add i64 %20, 8
  %22 = add i64 %dec2bin_sp, %21
  %23 = inttoptr i64 %22 to i64*
  %24 = load i64, i64* %23, align 4
  call void @dec2bin(i64 %dec2bin_sp, i64 %24, i32 %19)
  %25 = load i64, i64* @dec2bin_framesize_global, align 4
  %26 = add i64 %25, 16
  %27 = add i64 %dec2bin_sp, %26
  %dec2bin_num_ptr2 = inttoptr i64 %27 to i32*
  %28 = load i64, i64* @dec2bin_framesize_global, align 4
  %29 = add i64 %28, 16
  %30 = add i64 %dec2bin_sp, %29
  %dec2bin_num_ptr3 = inttoptr i64 %30 to i32*
  %31 = load i32, i32* %dec2bin_num_ptr3, align 4
  %32 = sdiv i32 %31, 2
  %33 = mul i32 %32, 2
  %34 = load i32, i32* %dec2bin_num_ptr2, align 4
  %35 = sub i32 %34, %33
  call void @printi(i32 %35)
  br label %if_next

if_next:                                          ; preds = %if_then, %if_test
  ret void
}

define void @try(i64 %0, i64 %1) {
try:
  %try_local_framesize = load i64, i64* @try_framesize_global, align 4
  %try_sp = sub i64 %0, %try_local_framesize
  %2 = load i64, i64* @try_framesize_global, align 4
  %3 = add i64 %2, 8
  %4 = add i64 %try_sp, %3
  %sl_ptr = inttoptr i64 %4 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  call void @printi(i32 100)
  call void @print(%string* @str)
  %5 = load i64, i64* @try_framesize_global, align 4
  %6 = add i64 %5, 8
  %7 = add i64 %try_sp, %6
  %8 = inttoptr i64 %7 to i64*
  %9 = load i64, i64* %8, align 4
  call void @dec2bin(i64 %try_sp, i64 %9, i32 100)
  call void @print(%string* @str.1)
  call void @printi(i32 200)
  call void @print(%string* @str.2)
  %10 = load i64, i64* @try_framesize_global, align 4
  %11 = add i64 %10, 8
  %12 = add i64 %try_sp, %11
  %13 = inttoptr i64 %12 to i64*
  %14 = load i64, i64* %13, align 4
  call void @dec2bin(i64 %try_sp, i64 %14, i32 200)
  call void @print(%string* @str.3)
  call void @printi(i32 789)
  call void @print(%string* @str.4)
  %15 = load i64, i64* @try_framesize_global, align 4
  %16 = add i64 %15, 8
  %17 = add i64 %try_sp, %16
  %18 = inttoptr i64 %17 to i64*
  %19 = load i64, i64* %18, align 4
  call void @dec2bin(i64 %try_sp, i64 %19, i32 789)
  call void @print(%string* @str.5)
  call void @printi(i32 567)
  call void @print(%string* @str.6)
  %20 = load i64, i64* @try_framesize_global, align 4
  %21 = add i64 %20, 8
  %22 = add i64 %try_sp, %21
  %23 = inttoptr i64 %22 to i64*
  %24 = load i64, i64* %23, align 4
  call void @dec2bin(i64 %try_sp, i64 %24, i32 567)
  call void @print(%string* @str.7)
  ret void
}

; ===== Frame Information =====
; tigermain 0 16
; dec2bin 0 24
; try 0 24
