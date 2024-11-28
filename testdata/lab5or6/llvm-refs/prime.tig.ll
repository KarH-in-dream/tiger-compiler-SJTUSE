; ModuleID = 'lightir'
source_filename = "lightir"
target triple = "x86_64-pc-linux-gnu"

%string = type { i32, i8* }

@tigermain_framesize_global = constant i64 16
@check_framesize_global = constant i64 32
@try_framesize_global = constant i64 24
@0 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @0, i32 0, i32 0) }
@1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.1 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @1, i32 0, i32 0) }
@2 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.2 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @2, i32 0, i32 0) }
@3 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.3 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @3, i32 0, i32 0) }
@4 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.4 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @4, i32 0, i32 0) }
@5 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.5 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @5, i32 0, i32 0) }
@6 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.6 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @6, i32 0, i32 0) }
@7 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.7 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @7, i32 0, i32 0) }
@8 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.8 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @8, i32 0, i32 0) }
@9 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.9 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @9, i32 0, i32 0) }
@10 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.10 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @10, i32 0, i32 0) }

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

define i32 @check(i64 %0, i64 %1, i32 %2) {
check:
  %check_local_framesize = load i64, i64* @check_framesize_global, align 4
  %check_sp = sub i64 %0, %check_local_framesize
  %3 = load i64, i64* @check_framesize_global, align 4
  %4 = add i64 %3, 8
  %5 = add i64 %check_sp, %4
  %sl_ptr = inttoptr i64 %5 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %6 = load i64, i64* @check_framesize_global, align 4
  %7 = add i64 %6, 16
  %8 = add i64 %check_sp, %7
  %9 = inttoptr i64 %8 to i32*
  store i32 %2, i32* %9, align 4
  %10 = load i64, i64* @check_framesize_global, align 4
  %11 = add i64 %10, -8
  %12 = add i64 %check_sp, %11
  %13 = inttoptr i64 %12 to i32*
  store i32 1, i32* %13, align 4
  %14 = load i64, i64* @check_framesize_global, align 4
  %15 = add i64 %14, 16
  %16 = add i64 %check_sp, %15
  %check_num_ptr = inttoptr i64 %16 to i32*
  %17 = load i32, i32* %check_num_ptr, align 4
  %18 = sdiv i32 %17, 2
  %19 = load i64, i64* @check_framesize_global, align 4
  %20 = add i64 %19, -16
  %21 = add i64 %check_sp, %20
  %22 = inttoptr i64 %21 to i32*
  store i32 2, i32* %22, align 4
  br label %for_body

for_incre:                                        ; preds = %if_next
  %23 = load i32, i32* %22, align 4
  %24 = add i32 1, %23
  store i32 %24, i32* %22, align 4
  %25 = icmp sle i32 %24, %18
  br i1 %25, label %for_body, label %for_next

for_body:                                         ; preds = %for_incre, %check
  br label %if_test

for_next:                                         ; preds = %for_incre, %if_then
  %26 = load i64, i64* @check_framesize_global, align 4
  %27 = add i64 %26, -8
  %28 = add i64 %check_sp, %27
  %check_flag_ptr4 = inttoptr i64 %28 to i32*
  %29 = load i32, i32* %check_flag_ptr4, align 4
  ret i32 %29

if_test:                                          ; preds = %for_body
  %30 = load i64, i64* @check_framesize_global, align 4
  %31 = add i64 %30, 16
  %32 = add i64 %check_sp, %31
  %check_num_ptr1 = inttoptr i64 %32 to i32*
  %33 = load i64, i64* @check_framesize_global, align 4
  %34 = add i64 %33, -16
  %35 = add i64 %check_sp, %34
  %check_i_ptr = inttoptr i64 %35 to i32*
  %36 = load i32, i32* %check_num_ptr1, align 4
  %37 = load i32, i32* %check_i_ptr, align 4
  %38 = sdiv i32 %36, %37
  %39 = load i64, i64* @check_framesize_global, align 4
  %40 = add i64 %39, -16
  %41 = add i64 %check_sp, %40
  %check_i_ptr2 = inttoptr i64 %41 to i32*
  %42 = load i32, i32* %check_i_ptr2, align 4
  %43 = mul i32 %38, %42
  %44 = load i64, i64* @check_framesize_global, align 4
  %45 = add i64 %44, 16
  %46 = add i64 %check_sp, %45
  %check_num_ptr3 = inttoptr i64 %46 to i32*
  %47 = load i32, i32* %check_num_ptr3, align 4
  %48 = icmp eq i32 %43, %47
  br i1 %48, label %if_then, label %if_next

if_then:                                          ; preds = %if_test
  %49 = load i64, i64* @check_framesize_global, align 4
  %50 = add i64 %49, -8
  %51 = add i64 %check_sp, %50
  %check_flag_ptr = inttoptr i64 %51 to i32*
  store i32 0, i32* %check_flag_ptr, align 4
  br label %for_next

if_next:                                          ; preds = %if_test
  br label %for_incre
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
  %5 = load i64, i64* @try_framesize_global, align 4
  %6 = add i64 %5, 8
  %7 = add i64 %try_sp, %6
  %8 = inttoptr i64 %7 to i64*
  %9 = load i64, i64* %8, align 4
  %10 = call i32 @check(i64 %try_sp, i64 %9, i32 56)
  call void @printi(i32 %10)
  call void @print(%string* @str)
  %11 = load i64, i64* @try_framesize_global, align 4
  %12 = add i64 %11, 8
  %13 = add i64 %try_sp, %12
  %14 = inttoptr i64 %13 to i64*
  %15 = load i64, i64* %14, align 4
  %16 = call i32 @check(i64 %try_sp, i64 %15, i32 23)
  call void @printi(i32 %16)
  call void @print(%string* @str.1)
  %17 = load i64, i64* @try_framesize_global, align 4
  %18 = add i64 %17, 8
  %19 = add i64 %try_sp, %18
  %20 = inttoptr i64 %19 to i64*
  %21 = load i64, i64* %20, align 4
  %22 = call i32 @check(i64 %try_sp, i64 %21, i32 71)
  call void @printi(i32 %22)
  call void @print(%string* @str.2)
  %23 = load i64, i64* @try_framesize_global, align 4
  %24 = add i64 %23, 8
  %25 = add i64 %try_sp, %24
  %26 = inttoptr i64 %25 to i64*
  %27 = load i64, i64* %26, align 4
  %28 = call i32 @check(i64 %try_sp, i64 %27, i32 72)
  call void @printi(i32 %28)
  call void @print(%string* @str.3)
  %29 = load i64, i64* @try_framesize_global, align 4
  %30 = add i64 %29, 8
  %31 = add i64 %try_sp, %30
  %32 = inttoptr i64 %31 to i64*
  %33 = load i64, i64* %32, align 4
  %34 = call i32 @check(i64 %try_sp, i64 %33, i32 173)
  call void @printi(i32 %34)
  call void @print(%string* @str.4)
  %35 = load i64, i64* @try_framesize_global, align 4
  %36 = add i64 %35, 8
  %37 = add i64 %try_sp, %36
  %38 = inttoptr i64 %37 to i64*
  %39 = load i64, i64* %38, align 4
  %40 = call i32 @check(i64 %try_sp, i64 %39, i32 181)
  call void @printi(i32 %40)
  call void @print(%string* @str.5)
  %41 = load i64, i64* @try_framesize_global, align 4
  %42 = add i64 %41, 8
  %43 = add i64 %try_sp, %42
  %44 = inttoptr i64 %43 to i64*
  %45 = load i64, i64* %44, align 4
  %46 = call i32 @check(i64 %try_sp, i64 %45, i32 281)
  call void @printi(i32 %46)
  call void @print(%string* @str.6)
  %47 = load i64, i64* @try_framesize_global, align 4
  %48 = add i64 %47, 8
  %49 = add i64 %try_sp, %48
  %50 = inttoptr i64 %49 to i64*
  %51 = load i64, i64* %50, align 4
  %52 = call i32 @check(i64 %try_sp, i64 %51, i32 659)
  call void @printi(i32 %52)
  call void @print(%string* @str.7)
  %53 = load i64, i64* @try_framesize_global, align 4
  %54 = add i64 %53, 8
  %55 = add i64 %try_sp, %54
  %56 = inttoptr i64 %55 to i64*
  %57 = load i64, i64* %56, align 4
  %58 = call i32 @check(i64 %try_sp, i64 %57, i32 729)
  call void @printi(i32 %58)
  call void @print(%string* @str.8)
  %59 = load i64, i64* @try_framesize_global, align 4
  %60 = add i64 %59, 8
  %61 = add i64 %try_sp, %60
  %62 = inttoptr i64 %61 to i64*
  %63 = load i64, i64* %62, align 4
  %64 = call i32 @check(i64 %try_sp, i64 %63, i32 947)
  call void @printi(i32 %64)
  call void @print(%string* @str.9)
  %65 = load i64, i64* @try_framesize_global, align 4
  %66 = add i64 %65, 8
  %67 = add i64 %try_sp, %66
  %68 = inttoptr i64 %67 to i64*
  %69 = load i64, i64* %68, align 4
  %70 = call i32 @check(i64 %try_sp, i64 %69, i32 945)
  call void @printi(i32 %70)
  call void @print(%string* @str.10)
  ret void
}

; ===== Frame Information =====
; tigermain 0 16
; check -16 32
; try 0 24
