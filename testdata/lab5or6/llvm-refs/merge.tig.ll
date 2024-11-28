; ModuleID = 'lightir'
source_filename = "lightir"
target triple = "x86_64-pc-linux-gnu"

%string = type { i32, i8* }
%MyStruct.0 = type { i32, %MyStruct.0* }
%MyStruct = type { i32 }

@tigermain_framesize_global = constant i64 56
@readint_framesize_global = constant i64 32
@isdigit_framesize_global = constant i64 24
@0 = private unnamed_addr constant [2 x i8] c"0\00", align 1
@str = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @0, i32 0, i32 0) }
@1 = private unnamed_addr constant [2 x i8] c"9\00", align 1
@str.1 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @1, i32 0, i32 0) }
@skipto_framesize_global = constant i64 16
@2 = private unnamed_addr constant [2 x i8] c" \00", align 1
@str.2 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @2, i32 0, i32 0) }
@3 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.3 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @3, i32 0, i32 0) }
@4 = private unnamed_addr constant [2 x i8] c"0\00", align 1
@str.4 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @4, i32 0, i32 0) }
@readlist_framesize_global = constant i64 40
@merge_framesize_global = constant i64 32
@printint_framesize_global = constant i64 24
@f_framesize_global = constant i64 24
@5 = private unnamed_addr constant [2 x i8] c"0\00", align 1
@str.5 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @5, i32 0, i32 0) }
@6 = private unnamed_addr constant [2 x i8] c"-\00", align 1
@str.6 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @6, i32 0, i32 0) }
@7 = private unnamed_addr constant [2 x i8] c"0\00", align 1
@str.7 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @7, i32 0, i32 0) }
@printlist_framesize_global = constant i64 24
@8 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.8 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @8, i32 0, i32 0) }
@9 = private unnamed_addr constant [2 x i8] c" \00", align 1
@str.9 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @9, i32 0, i32 0) }

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
  %3 = call %string* @__wrap_getchar()
  %4 = ptrtoint %string* %3 to i64
  %5 = load i64, i64* @tigermain_framesize_global, align 4
  %6 = add i64 %5, -8
  %7 = add i64 %tigermain_sp, %6
  %8 = inttoptr i64 %7 to i64*
  store i64 %4, i64* %8, align 4
  %9 = call %MyStruct.0* @readlist(i64 %tigermain_sp, i64 %tigermain_sp)
  %10 = ptrtoint %MyStruct.0* %9 to i64
  %11 = load i64, i64* @tigermain_framesize_global, align 4
  %12 = add i64 %11, -16
  %13 = add i64 %tigermain_sp, %12
  %14 = inttoptr i64 %13 to i64*
  store i64 %10, i64* %14, align 4
  %15 = load i64, i64* @tigermain_framesize_global, align 4
  %16 = add i64 %15, -8
  %17 = add i64 %tigermain_sp, %16
  %tigermain_buffer_ptr = inttoptr i64 %17 to %string**
  %18 = call %string* @__wrap_getchar()
  store %string* %18, %string** %tigermain_buffer_ptr, align 8
  %19 = call %MyStruct.0* @readlist(i64 %tigermain_sp, i64 %tigermain_sp)
  %20 = ptrtoint %MyStruct.0* %19 to i64
  %21 = load i64, i64* @tigermain_framesize_global, align 4
  %22 = add i64 %21, -24
  %23 = add i64 %tigermain_sp, %22
  %24 = inttoptr i64 %23 to i64*
  store i64 %20, i64* %24, align 4
  %25 = load i64, i64* @tigermain_framesize_global, align 4
  %26 = add i64 %25, -16
  %27 = add i64 %tigermain_sp, %26
  %tigermain_list1_ptr = inttoptr i64 %27 to %MyStruct.0**
  %actual_parm = load %MyStruct.0*, %MyStruct.0** %tigermain_list1_ptr, align 8
  %28 = load i64, i64* @tigermain_framesize_global, align 4
  %29 = add i64 %28, -24
  %30 = add i64 %tigermain_sp, %29
  %tigermain_list2_ptr = inttoptr i64 %30 to %MyStruct.0**
  %actual_parm1 = load %MyStruct.0*, %MyStruct.0** %tigermain_list2_ptr, align 8
  %31 = call %MyStruct.0* @merge(i64 %tigermain_sp, i64 %tigermain_sp, %MyStruct.0* %actual_parm, %MyStruct.0* %actual_parm1)
  call void @printlist(i64 %tigermain_sp, i64 %tigermain_sp, %MyStruct.0* %31)
  ret i32 0
}

define i32 @readint(i64 %0, i64 %1, %MyStruct* %2) {
readint:
  %readint_local_framesize = load i64, i64* @readint_framesize_global, align 4
  %readint_sp = sub i64 %0, %readint_local_framesize
  %3 = load i64, i64* @readint_framesize_global, align 4
  %4 = add i64 %3, 8
  %5 = add i64 %readint_sp, %4
  %sl_ptr = inttoptr i64 %5 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %6 = load i64, i64* @readint_framesize_global, align 4
  %7 = add i64 %6, 16
  %8 = add i64 %readint_sp, %7
  %9 = inttoptr i64 %8 to %MyStruct**
  store %MyStruct* %2, %MyStruct** %9, align 8
  %10 = load i64, i64* @readint_framesize_global, align 4
  %11 = add i64 %10, -8
  %12 = add i64 %readint_sp, %11
  %13 = inttoptr i64 %12 to i32*
  store i32 0, i32* %13, align 4
  call void @skipto(i64 %readint_sp, i64 %readint_sp)
  %14 = load i64, i64* @readint_framesize_global, align 4
  %15 = add i64 %14, 16
  %16 = add i64 %readint_sp, %15
  %readint_any_ptr = inttoptr i64 %16 to %MyStruct**
  %17 = load %MyStruct*, %MyStruct** %readint_any_ptr, align 8
  %18 = getelementptr %MyStruct, %MyStruct* %17, i32 0, i32 0
  %19 = load i64, i64* @readint_framesize_global, align 4
  %20 = add i64 %19, 8
  %21 = add i64 %readint_sp, %20
  %22 = inttoptr i64 %21 to i64*
  %23 = load i64, i64* %22, align 4
  %24 = load i64, i64* @tigermain_framesize_global, align 4
  %25 = add i64 %24, -8
  %26 = add i64 %23, %25
  %tigermain_buffer_ptr = inttoptr i64 %26 to %string**
  %actual_parm = load %string*, %string** %tigermain_buffer_ptr, align 8
  %27 = call i32 @isdigit(i64 %readint_sp, i64 %readint_sp, %string* %actual_parm)
  store i32 %27, i32* %18, align 4
  br label %while_test

while_test:                                       ; preds = %while_body, %readint
  %28 = load i64, i64* @readint_framesize_global, align 4
  %29 = add i64 %28, 8
  %30 = add i64 %readint_sp, %29
  %31 = inttoptr i64 %30 to i64*
  %32 = load i64, i64* %31, align 4
  %33 = load i64, i64* @tigermain_framesize_global, align 4
  %34 = add i64 %33, -8
  %35 = add i64 %32, %34
  %tigermain_buffer_ptr1 = inttoptr i64 %35 to %string**
  %actual_parm2 = load %string*, %string** %tigermain_buffer_ptr1, align 8
  %36 = call i32 @isdigit(i64 %readint_sp, i64 %readint_sp, %string* %actual_parm2)
  %37 = icmp ne i32 %36, 0
  br i1 %37, label %while_body, label %while_next

while_body:                                       ; preds = %while_test
  %38 = load i64, i64* @readint_framesize_global, align 4
  %39 = add i64 %38, -8
  %40 = add i64 %readint_sp, %39
  %readint_i_ptr = inttoptr i64 %40 to i32*
  %41 = load i64, i64* @readint_framesize_global, align 4
  %42 = add i64 %41, -8
  %43 = add i64 %readint_sp, %42
  %readint_i_ptr3 = inttoptr i64 %43 to i32*
  %44 = load i32, i32* %readint_i_ptr3, align 4
  %45 = mul i32 %44, 10
  %46 = load i64, i64* @readint_framesize_global, align 4
  %47 = add i64 %46, 8
  %48 = add i64 %readint_sp, %47
  %49 = inttoptr i64 %48 to i64*
  %50 = load i64, i64* %49, align 4
  %51 = load i64, i64* @tigermain_framesize_global, align 4
  %52 = add i64 %51, -8
  %53 = add i64 %50, %52
  %tigermain_buffer_ptr4 = inttoptr i64 %53 to %string**
  %actual_parm5 = load %string*, %string** %tigermain_buffer_ptr4, align 8
  %54 = call i32 @ord(%string* %actual_parm5)
  %55 = add i32 %45, %54
  %56 = call i32 @ord(%string* @str.4)
  %57 = sub i32 %55, %56
  store i32 %57, i32* %readint_i_ptr, align 4
  %58 = load i64, i64* @readint_framesize_global, align 4
  %59 = add i64 %58, 8
  %60 = add i64 %readint_sp, %59
  %61 = inttoptr i64 %60 to i64*
  %62 = load i64, i64* %61, align 4
  %63 = load i64, i64* @tigermain_framesize_global, align 4
  %64 = add i64 %63, -8
  %65 = add i64 %62, %64
  %tigermain_buffer_ptr6 = inttoptr i64 %65 to %string**
  %66 = call %string* @__wrap_getchar()
  store %string* %66, %string** %tigermain_buffer_ptr6, align 8
  br label %while_test

while_next:                                       ; preds = %while_test
  %67 = load i64, i64* @readint_framesize_global, align 4
  %68 = add i64 %67, -8
  %69 = add i64 %readint_sp, %68
  %readint_i_ptr7 = inttoptr i64 %69 to i32*
  %70 = load i32, i32* %readint_i_ptr7, align 4
  ret i32 %70
}

define i32 @isdigit(i64 %0, i64 %1, %string* %2) {
isdigit:
  %isdigit_local_framesize = load i64, i64* @isdigit_framesize_global, align 4
  %isdigit_sp = sub i64 %0, %isdigit_local_framesize
  %3 = load i64, i64* @isdigit_framesize_global, align 4
  %4 = add i64 %3, 8
  %5 = add i64 %isdigit_sp, %4
  %sl_ptr = inttoptr i64 %5 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %6 = load i64, i64* @isdigit_framesize_global, align 4
  %7 = add i64 %6, 16
  %8 = add i64 %isdigit_sp, %7
  %9 = inttoptr i64 %8 to %string**
  store %string* %2, %string** %9, align 8
  %10 = load i64, i64* @isdigit_framesize_global, align 4
  %11 = add i64 %10, 8
  %12 = add i64 %isdigit_sp, %11
  %13 = inttoptr i64 %12 to i64*
  %14 = load i64, i64* %13, align 4
  %15 = load i64, i64* @readint_framesize_global, align 4
  %16 = add i64 %15, 8
  %17 = add i64 %14, %16
  %18 = inttoptr i64 %17 to i64*
  %19 = load i64, i64* %18, align 4
  %20 = load i64, i64* @tigermain_framesize_global, align 4
  %21 = add i64 %20, -8
  %22 = add i64 %19, %21
  %tigermain_buffer_ptr = inttoptr i64 %22 to %string**
  %actual_parm = load %string*, %string** %tigermain_buffer_ptr, align 8
  %23 = call i32 @ord(%string* %actual_parm)
  %24 = call i32 @ord(%string* @str)
  %25 = icmp sge i32 %23, %24
  br i1 %25, label %opand_right_test, label %opand_next

opand_right_test:                                 ; preds = %isdigit
  %26 = load i64, i64* @isdigit_framesize_global, align 4
  %27 = add i64 %26, 8
  %28 = add i64 %isdigit_sp, %27
  %29 = inttoptr i64 %28 to i64*
  %30 = load i64, i64* %29, align 4
  %31 = load i64, i64* @readint_framesize_global, align 4
  %32 = add i64 %31, 8
  %33 = add i64 %30, %32
  %34 = inttoptr i64 %33 to i64*
  %35 = load i64, i64* %34, align 4
  %36 = load i64, i64* @tigermain_framesize_global, align 4
  %37 = add i64 %36, -8
  %38 = add i64 %35, %37
  %tigermain_buffer_ptr1 = inttoptr i64 %38 to %string**
  %actual_parm2 = load %string*, %string** %tigermain_buffer_ptr1, align 8
  %39 = call i32 @ord(%string* %actual_parm2)
  %40 = call i32 @ord(%string* @str.1)
  %41 = icmp sle i32 %39, %40
  br label %opand_next

opand_next:                                       ; preds = %opand_right_test, %isdigit
  %42 = phi i1 [ false, %isdigit ], [ %41, %opand_right_test ]
  %43 = zext i1 %42 to i32
  ret i32 %43
}

define void @skipto(i64 %0, i64 %1) {
skipto:
  %skipto_local_framesize = load i64, i64* @skipto_framesize_global, align 4
  %skipto_sp = sub i64 %0, %skipto_local_framesize
  %2 = load i64, i64* @skipto_framesize_global, align 4
  %3 = add i64 %2, 8
  %4 = add i64 %skipto_sp, %3
  %sl_ptr = inttoptr i64 %4 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  br label %while_test

while_test:                                       ; preds = %while_body, %skipto
  %5 = load i64, i64* @skipto_framesize_global, align 4
  %6 = add i64 %5, 8
  %7 = add i64 %skipto_sp, %6
  %8 = inttoptr i64 %7 to i64*
  %9 = load i64, i64* %8, align 4
  %10 = load i64, i64* @readint_framesize_global, align 4
  %11 = add i64 %10, 8
  %12 = add i64 %9, %11
  %13 = inttoptr i64 %12 to i64*
  %14 = load i64, i64* %13, align 4
  %15 = load i64, i64* @tigermain_framesize_global, align 4
  %16 = add i64 %15, -8
  %17 = add i64 %14, %16
  %tigermain_buffer_ptr = inttoptr i64 %17 to %string**
  %18 = load %string*, %string** %tigermain_buffer_ptr, align 8
  %19 = call i1 @string_equal(%string* %18, %string* @str.2)
  br i1 %19, label %opor_next, label %opor_right_test

while_body:                                       ; preds = %opor_next
  %20 = load i64, i64* @skipto_framesize_global, align 4
  %21 = add i64 %20, 8
  %22 = add i64 %skipto_sp, %21
  %23 = inttoptr i64 %22 to i64*
  %24 = load i64, i64* %23, align 4
  %25 = load i64, i64* @readint_framesize_global, align 4
  %26 = add i64 %25, 8
  %27 = add i64 %24, %26
  %28 = inttoptr i64 %27 to i64*
  %29 = load i64, i64* %28, align 4
  %30 = load i64, i64* @tigermain_framesize_global, align 4
  %31 = add i64 %30, -8
  %32 = add i64 %29, %31
  %tigermain_buffer_ptr2 = inttoptr i64 %32 to %string**
  %33 = call %string* @__wrap_getchar()
  store %string* %33, %string** %tigermain_buffer_ptr2, align 8
  br label %while_test

while_next:                                       ; preds = %opor_next
  ret void

opor_right_test:                                  ; preds = %while_test
  %34 = load i64, i64* @skipto_framesize_global, align 4
  %35 = add i64 %34, 8
  %36 = add i64 %skipto_sp, %35
  %37 = inttoptr i64 %36 to i64*
  %38 = load i64, i64* %37, align 4
  %39 = load i64, i64* @readint_framesize_global, align 4
  %40 = add i64 %39, 8
  %41 = add i64 %38, %40
  %42 = inttoptr i64 %41 to i64*
  %43 = load i64, i64* %42, align 4
  %44 = load i64, i64* @tigermain_framesize_global, align 4
  %45 = add i64 %44, -8
  %46 = add i64 %43, %45
  %tigermain_buffer_ptr1 = inttoptr i64 %46 to %string**
  %47 = load %string*, %string** %tigermain_buffer_ptr1, align 8
  %48 = call i1 @string_equal(%string* %47, %string* @str.3)
  br label %opor_next

opor_next:                                        ; preds = %opor_right_test, %while_test
  %49 = phi i1 [ true, %while_test ], [ %48, %opor_right_test ]
  br i1 %49, label %while_body, label %while_next
}

define %MyStruct.0* @readlist(i64 %0, i64 %1) {
readlist:
  %readlist_local_framesize = load i64, i64* @readlist_framesize_global, align 4
  %readlist_sp = sub i64 %0, %readlist_local_framesize
  %2 = load i64, i64* @readlist_framesize_global, align 4
  %3 = add i64 %2, 8
  %4 = add i64 %readlist_sp, %3
  %sl_ptr = inttoptr i64 %4 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %5 = call i64 @alloc_record(i32 8)
  %6 = inttoptr i64 %5 to %MyStruct*
  %7 = getelementptr %MyStruct, %MyStruct* %6, i32 0, i32 0
  store i32 0, i32* %7, align 4
  %8 = ptrtoint %MyStruct* %6 to i64
  %9 = load i64, i64* @readlist_framesize_global, align 4
  %10 = add i64 %9, -8
  %11 = add i64 %readlist_sp, %10
  %12 = inttoptr i64 %11 to i64*
  store i64 %8, i64* %12, align 4
  %13 = load i64, i64* @readlist_framesize_global, align 4
  %14 = add i64 %13, -8
  %15 = add i64 %readlist_sp, %14
  %readlist_any_ptr = inttoptr i64 %15 to %MyStruct**
  %actual_parm = load %MyStruct*, %MyStruct** %readlist_any_ptr, align 8
  %16 = load i64, i64* @readlist_framesize_global, align 4
  %17 = add i64 %16, 8
  %18 = add i64 %readlist_sp, %17
  %19 = inttoptr i64 %18 to i64*
  %20 = load i64, i64* %19, align 4
  %21 = call i32 @readint(i64 %readlist_sp, i64 %20, %MyStruct* %actual_parm)
  %22 = load i64, i64* @readlist_framesize_global, align 4
  %23 = add i64 %22, -16
  %24 = add i64 %readlist_sp, %23
  %25 = inttoptr i64 %24 to i32*
  store i32 %21, i32* %25, align 4
  br label %if_test

if_test:                                          ; preds = %readlist
  %26 = load i64, i64* @readlist_framesize_global, align 4
  %27 = add i64 %26, -8
  %28 = add i64 %readlist_sp, %27
  %readlist_any_ptr1 = inttoptr i64 %28 to %MyStruct**
  %29 = load %MyStruct*, %MyStruct** %readlist_any_ptr1, align 8
  %30 = getelementptr %MyStruct, %MyStruct* %29, i32 0, i32 0
  %31 = load i32, i32* %30, align 4
  %32 = icmp ne i32 %31, 0
  br i1 %32, label %if_then, label %if_else

if_then:                                          ; preds = %if_test
  %33 = call i64 @alloc_record(i32 16)
  %34 = inttoptr i64 %33 to %MyStruct.0*
  %35 = load i64, i64* @readlist_framesize_global, align 4
  %36 = add i64 %35, -16
  %37 = add i64 %readlist_sp, %36
  %readlist_i_ptr = inttoptr i64 %37 to i32*
  %38 = getelementptr %MyStruct.0, %MyStruct.0* %34, i32 0, i32 0
  %39 = load i32, i32* %readlist_i_ptr, align 4
  store i32 %39, i32* %38, align 4
  %40 = load i64, i64* @readlist_framesize_global, align 4
  %41 = add i64 %40, 8
  %42 = add i64 %readlist_sp, %41
  %43 = inttoptr i64 %42 to i64*
  %44 = load i64, i64* %43, align 4
  %45 = call %MyStruct.0* @readlist(i64 %readlist_sp, i64 %44)
  %46 = getelementptr %MyStruct.0, %MyStruct.0* %34, i32 0, i32 1
  store %MyStruct.0* %45, %MyStruct.0** %46, align 8
  br label %if_next

if_else:                                          ; preds = %if_test
  br label %if_next

if_next:                                          ; preds = %if_else, %if_then
  %47 = phi %MyStruct.0* [ %34, %if_then ], [ null, %if_else ]
  ret %MyStruct.0* %47
}

define %MyStruct.0* @merge(i64 %0, i64 %1, %MyStruct.0* %2, %MyStruct.0* %3) {
merge:
  %merge_local_framesize = load i64, i64* @merge_framesize_global, align 4
  %merge_sp = sub i64 %0, %merge_local_framesize
  %4 = load i64, i64* @merge_framesize_global, align 4
  %5 = add i64 %4, 8
  %6 = add i64 %merge_sp, %5
  %sl_ptr = inttoptr i64 %6 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %7 = load i64, i64* @merge_framesize_global, align 4
  %8 = add i64 %7, 16
  %9 = add i64 %merge_sp, %8
  %10 = inttoptr i64 %9 to %MyStruct.0**
  store %MyStruct.0* %2, %MyStruct.0** %10, align 8
  %11 = load i64, i64* @merge_framesize_global, align 4
  %12 = add i64 %11, 24
  %13 = add i64 %merge_sp, %12
  %14 = inttoptr i64 %13 to %MyStruct.0**
  store %MyStruct.0* %3, %MyStruct.0** %14, align 8
  br label %if_test

if_test:                                          ; preds = %merge
  %15 = load i64, i64* @merge_framesize_global, align 4
  %16 = add i64 %15, 16
  %17 = add i64 %merge_sp, %16
  %merge_a_ptr = inttoptr i64 %17 to %MyStruct.0**
  %18 = load %MyStruct.0*, %MyStruct.0** %merge_a_ptr, align 8
  %19 = ptrtoint %MyStruct.0* %18 to i64
  %20 = icmp eq i64 %19, 0
  br i1 %20, label %if_then, label %if_else

if_then:                                          ; preds = %if_test
  %21 = load i64, i64* @merge_framesize_global, align 4
  %22 = add i64 %21, 24
  %23 = add i64 %merge_sp, %22
  %merge_b_ptr = inttoptr i64 %23 to %MyStruct.0**
  %24 = load %MyStruct.0*, %MyStruct.0** %merge_b_ptr, align 8
  br label %if_next

if_else:                                          ; preds = %if_test
  br label %if_test1

if_next:                                          ; preds = %if_next4, %if_then
  %25 = phi %MyStruct.0* [ %24, %if_then ], [ %36, %if_next4 ]
  ret %MyStruct.0* %25

if_test1:                                         ; preds = %if_else
  %26 = load i64, i64* @merge_framesize_global, align 4
  %27 = add i64 %26, 24
  %28 = add i64 %merge_sp, %27
  %merge_b_ptr5 = inttoptr i64 %28 to %MyStruct.0**
  %29 = load %MyStruct.0*, %MyStruct.0** %merge_b_ptr5, align 8
  %30 = ptrtoint %MyStruct.0* %29 to i64
  %31 = icmp eq i64 %30, 0
  br i1 %31, label %if_then2, label %if_else3

if_then2:                                         ; preds = %if_test1
  %32 = load i64, i64* @merge_framesize_global, align 4
  %33 = add i64 %32, 16
  %34 = add i64 %merge_sp, %33
  %merge_a_ptr6 = inttoptr i64 %34 to %MyStruct.0**
  %35 = load %MyStruct.0*, %MyStruct.0** %merge_a_ptr6, align 8
  br label %if_next4

if_else3:                                         ; preds = %if_test1
  br label %if_test7

if_next4:                                         ; preds = %if_next10, %if_then2
  %36 = phi %MyStruct.0* [ %35, %if_then2 ], [ %98, %if_next10 ]
  br label %if_next

if_test7:                                         ; preds = %if_else3
  %37 = load i64, i64* @merge_framesize_global, align 4
  %38 = add i64 %37, 16
  %39 = add i64 %merge_sp, %38
  %merge_a_ptr11 = inttoptr i64 %39 to %MyStruct.0**
  %40 = load %MyStruct.0*, %MyStruct.0** %merge_a_ptr11, align 8
  %41 = getelementptr %MyStruct.0, %MyStruct.0* %40, i32 0, i32 0
  %42 = load i64, i64* @merge_framesize_global, align 4
  %43 = add i64 %42, 24
  %44 = add i64 %merge_sp, %43
  %merge_b_ptr12 = inttoptr i64 %44 to %MyStruct.0**
  %45 = load %MyStruct.0*, %MyStruct.0** %merge_b_ptr12, align 8
  %46 = getelementptr %MyStruct.0, %MyStruct.0* %45, i32 0, i32 0
  %47 = load i32, i32* %41, align 4
  %48 = load i32, i32* %46, align 4
  %49 = icmp slt i32 %47, %48
  br i1 %49, label %if_then8, label %if_else9

if_then8:                                         ; preds = %if_test7
  %50 = call i64 @alloc_record(i32 16)
  %51 = inttoptr i64 %50 to %MyStruct.0*
  %52 = load i64, i64* @merge_framesize_global, align 4
  %53 = add i64 %52, 16
  %54 = add i64 %merge_sp, %53
  %merge_a_ptr13 = inttoptr i64 %54 to %MyStruct.0**
  %55 = load %MyStruct.0*, %MyStruct.0** %merge_a_ptr13, align 8
  %56 = getelementptr %MyStruct.0, %MyStruct.0* %55, i32 0, i32 0
  %57 = getelementptr %MyStruct.0, %MyStruct.0* %51, i32 0, i32 0
  %58 = load i32, i32* %56, align 4
  store i32 %58, i32* %57, align 4
  %59 = load i64, i64* @merge_framesize_global, align 4
  %60 = add i64 %59, 16
  %61 = add i64 %merge_sp, %60
  %merge_a_ptr14 = inttoptr i64 %61 to %MyStruct.0**
  %62 = load %MyStruct.0*, %MyStruct.0** %merge_a_ptr14, align 8
  %63 = getelementptr %MyStruct.0, %MyStruct.0* %62, i32 0, i32 1
  %actual_parm = load %MyStruct.0*, %MyStruct.0** %63, align 8
  %64 = load i64, i64* @merge_framesize_global, align 4
  %65 = add i64 %64, 24
  %66 = add i64 %merge_sp, %65
  %merge_b_ptr15 = inttoptr i64 %66 to %MyStruct.0**
  %actual_parm16 = load %MyStruct.0*, %MyStruct.0** %merge_b_ptr15, align 8
  %67 = load i64, i64* @merge_framesize_global, align 4
  %68 = add i64 %67, 8
  %69 = add i64 %merge_sp, %68
  %70 = inttoptr i64 %69 to i64*
  %71 = load i64, i64* %70, align 4
  %72 = call %MyStruct.0* @merge(i64 %merge_sp, i64 %71, %MyStruct.0* %actual_parm, %MyStruct.0* %actual_parm16)
  %73 = getelementptr %MyStruct.0, %MyStruct.0* %51, i32 0, i32 1
  store %MyStruct.0* %72, %MyStruct.0** %73, align 8
  br label %if_next10

if_else9:                                         ; preds = %if_test7
  %74 = call i64 @alloc_record(i32 16)
  %75 = inttoptr i64 %74 to %MyStruct.0*
  %76 = load i64, i64* @merge_framesize_global, align 4
  %77 = add i64 %76, 24
  %78 = add i64 %merge_sp, %77
  %merge_b_ptr17 = inttoptr i64 %78 to %MyStruct.0**
  %79 = load %MyStruct.0*, %MyStruct.0** %merge_b_ptr17, align 8
  %80 = getelementptr %MyStruct.0, %MyStruct.0* %79, i32 0, i32 0
  %81 = getelementptr %MyStruct.0, %MyStruct.0* %75, i32 0, i32 0
  %82 = load i32, i32* %80, align 4
  store i32 %82, i32* %81, align 4
  %83 = load i64, i64* @merge_framesize_global, align 4
  %84 = add i64 %83, 16
  %85 = add i64 %merge_sp, %84
  %merge_a_ptr18 = inttoptr i64 %85 to %MyStruct.0**
  %actual_parm19 = load %MyStruct.0*, %MyStruct.0** %merge_a_ptr18, align 8
  %86 = load i64, i64* @merge_framesize_global, align 4
  %87 = add i64 %86, 24
  %88 = add i64 %merge_sp, %87
  %merge_b_ptr20 = inttoptr i64 %88 to %MyStruct.0**
  %89 = load %MyStruct.0*, %MyStruct.0** %merge_b_ptr20, align 8
  %90 = getelementptr %MyStruct.0, %MyStruct.0* %89, i32 0, i32 1
  %actual_parm21 = load %MyStruct.0*, %MyStruct.0** %90, align 8
  %91 = load i64, i64* @merge_framesize_global, align 4
  %92 = add i64 %91, 8
  %93 = add i64 %merge_sp, %92
  %94 = inttoptr i64 %93 to i64*
  %95 = load i64, i64* %94, align 4
  %96 = call %MyStruct.0* @merge(i64 %merge_sp, i64 %95, %MyStruct.0* %actual_parm19, %MyStruct.0* %actual_parm21)
  %97 = getelementptr %MyStruct.0, %MyStruct.0* %75, i32 0, i32 1
  store %MyStruct.0* %96, %MyStruct.0** %97, align 8
  br label %if_next10

if_next10:                                        ; preds = %if_else9, %if_then8
  %98 = phi %MyStruct.0* [ %51, %if_then8 ], [ %75, %if_else9 ]
  br label %if_next4
}

define void @printint(i64 %0, i64 %1, i32 %2) {
printint:
  %printint_local_framesize = load i64, i64* @printint_framesize_global, align 4
  %printint_sp = sub i64 %0, %printint_local_framesize
  %3 = load i64, i64* @printint_framesize_global, align 4
  %4 = add i64 %3, 8
  %5 = add i64 %printint_sp, %4
  %sl_ptr = inttoptr i64 %5 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %6 = load i64, i64* @printint_framesize_global, align 4
  %7 = add i64 %6, 16
  %8 = add i64 %printint_sp, %7
  %9 = inttoptr i64 %8 to i32*
  store i32 %2, i32* %9, align 4
  br label %if_test

if_test:                                          ; preds = %printint
  %10 = load i64, i64* @printint_framesize_global, align 4
  %11 = add i64 %10, 16
  %12 = add i64 %printint_sp, %11
  %printint_i_ptr = inttoptr i64 %12 to i32*
  %13 = load i32, i32* %printint_i_ptr, align 4
  %14 = icmp slt i32 %13, 0
  br i1 %14, label %if_then, label %if_else

if_then:                                          ; preds = %if_test
  call void @print(%string* @str.6)
  %15 = load i64, i64* @printint_framesize_global, align 4
  %16 = add i64 %15, 16
  %17 = add i64 %printint_sp, %16
  %printint_i_ptr1 = inttoptr i64 %17 to i32*
  %18 = load i32, i32* %printint_i_ptr1, align 4
  %19 = sub i32 0, %18
  call void @f(i64 %printint_sp, i64 %printint_sp, i32 %19)
  br label %if_next

if_else:                                          ; preds = %if_test
  br label %if_test2

if_next:                                          ; preds = %if_next5, %if_then
  ret void

if_test2:                                         ; preds = %if_else
  %20 = load i64, i64* @printint_framesize_global, align 4
  %21 = add i64 %20, 16
  %22 = add i64 %printint_sp, %21
  %printint_i_ptr6 = inttoptr i64 %22 to i32*
  %23 = load i32, i32* %printint_i_ptr6, align 4
  %24 = icmp sgt i32 %23, 0
  br i1 %24, label %if_then3, label %if_else4

if_then3:                                         ; preds = %if_test2
  %25 = load i64, i64* @printint_framesize_global, align 4
  %26 = add i64 %25, 16
  %27 = add i64 %printint_sp, %26
  %printint_i_ptr7 = inttoptr i64 %27 to i32*
  %actual_parm = load i32, i32* %printint_i_ptr7, align 4
  call void @f(i64 %printint_sp, i64 %printint_sp, i32 %actual_parm)
  br label %if_next5

if_else4:                                         ; preds = %if_test2
  call void @print(%string* @str.7)
  br label %if_next5

if_next5:                                         ; preds = %if_else4, %if_then3
  br label %if_next
}

define void @printlist(i64 %0, i64 %1, %MyStruct.0* %2) {
printlist:
  %printlist_local_framesize = load i64, i64* @printlist_framesize_global, align 4
  %printlist_sp = sub i64 %0, %printlist_local_framesize
  %3 = load i64, i64* @printlist_framesize_global, align 4
  %4 = add i64 %3, 8
  %5 = add i64 %printlist_sp, %4
  %sl_ptr = inttoptr i64 %5 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %6 = load i64, i64* @printlist_framesize_global, align 4
  %7 = add i64 %6, 16
  %8 = add i64 %printlist_sp, %7
  %9 = inttoptr i64 %8 to %MyStruct.0**
  store %MyStruct.0* %2, %MyStruct.0** %9, align 8
  br label %if_test

if_test:                                          ; preds = %printlist
  %10 = load i64, i64* @printlist_framesize_global, align 4
  %11 = add i64 %10, 16
  %12 = add i64 %printlist_sp, %11
  %printlist_l_ptr = inttoptr i64 %12 to %MyStruct.0**
  %13 = load %MyStruct.0*, %MyStruct.0** %printlist_l_ptr, align 8
  %14 = ptrtoint %MyStruct.0* %13 to i64
  %15 = icmp eq i64 %14, 0
  br i1 %15, label %if_then, label %if_else

if_then:                                          ; preds = %if_test
  call void @print(%string* @str.8)
  br label %if_next

if_else:                                          ; preds = %if_test
  %16 = load i64, i64* @printlist_framesize_global, align 4
  %17 = add i64 %16, 16
  %18 = add i64 %printlist_sp, %17
  %printlist_l_ptr1 = inttoptr i64 %18 to %MyStruct.0**
  %19 = load %MyStruct.0*, %MyStruct.0** %printlist_l_ptr1, align 8
  %20 = getelementptr %MyStruct.0, %MyStruct.0* %19, i32 0, i32 0
  %actual_parm = load i32, i32* %20, align 4
  %21 = load i64, i64* @printlist_framesize_global, align 4
  %22 = add i64 %21, 8
  %23 = add i64 %printlist_sp, %22
  %24 = inttoptr i64 %23 to i64*
  %25 = load i64, i64* %24, align 4
  call void @printint(i64 %printlist_sp, i64 %25, i32 %actual_parm)
  call void @print(%string* @str.9)
  %26 = load i64, i64* @printlist_framesize_global, align 4
  %27 = add i64 %26, 16
  %28 = add i64 %printlist_sp, %27
  %printlist_l_ptr2 = inttoptr i64 %28 to %MyStruct.0**
  %29 = load %MyStruct.0*, %MyStruct.0** %printlist_l_ptr2, align 8
  %30 = getelementptr %MyStruct.0, %MyStruct.0* %29, i32 0, i32 1
  %actual_parm3 = load %MyStruct.0*, %MyStruct.0** %30, align 8
  %31 = load i64, i64* @printlist_framesize_global, align 4
  %32 = add i64 %31, 8
  %33 = add i64 %printlist_sp, %32
  %34 = inttoptr i64 %33 to i64*
  %35 = load i64, i64* %34, align 4
  call void @printlist(i64 %printlist_sp, i64 %35, %MyStruct.0* %actual_parm3)
  br label %if_next

if_next:                                          ; preds = %if_else, %if_then
  ret void
}

define void @f(i64 %0, i64 %1, i32 %2) {
f:
  %f_local_framesize = load i64, i64* @f_framesize_global, align 4
  %f_sp = sub i64 %0, %f_local_framesize
  %3 = load i64, i64* @f_framesize_global, align 4
  %4 = add i64 %3, 8
  %5 = add i64 %f_sp, %4
  %sl_ptr = inttoptr i64 %5 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %6 = load i64, i64* @f_framesize_global, align 4
  %7 = add i64 %6, 16
  %8 = add i64 %f_sp, %7
  %9 = inttoptr i64 %8 to i32*
  store i32 %2, i32* %9, align 4
  br label %if_test

if_test:                                          ; preds = %f
  %10 = load i64, i64* @f_framesize_global, align 4
  %11 = add i64 %10, 16
  %12 = add i64 %f_sp, %11
  %f_i_ptr = inttoptr i64 %12 to i32*
  %13 = load i32, i32* %f_i_ptr, align 4
  %14 = icmp sgt i32 %13, 0
  br i1 %14, label %if_then, label %if_next

if_then:                                          ; preds = %if_test
  %15 = load i64, i64* @f_framesize_global, align 4
  %16 = add i64 %15, 16
  %17 = add i64 %f_sp, %16
  %f_i_ptr1 = inttoptr i64 %17 to i32*
  %18 = load i32, i32* %f_i_ptr1, align 4
  %19 = sdiv i32 %18, 10
  %20 = load i64, i64* @f_framesize_global, align 4
  %21 = add i64 %20, 8
  %22 = add i64 %f_sp, %21
  %23 = inttoptr i64 %22 to i64*
  %24 = load i64, i64* %23, align 4
  call void @f(i64 %f_sp, i64 %24, i32 %19)
  %25 = load i64, i64* @f_framesize_global, align 4
  %26 = add i64 %25, 16
  %27 = add i64 %f_sp, %26
  %f_i_ptr2 = inttoptr i64 %27 to i32*
  %28 = load i64, i64* @f_framesize_global, align 4
  %29 = add i64 %28, 16
  %30 = add i64 %f_sp, %29
  %f_i_ptr3 = inttoptr i64 %30 to i32*
  %31 = load i32, i32* %f_i_ptr3, align 4
  %32 = sdiv i32 %31, 10
  %33 = mul i32 %32, 10
  %34 = load i32, i32* %f_i_ptr2, align 4
  %35 = sub i32 %34, %33
  %36 = call i32 @ord(%string* @str.5)
  %37 = add i32 %35, %36
  %38 = call %string* @chr(i32 %37)
  call void @print(%string* %38)
  br label %if_next

if_next:                                          ; preds = %if_then, %if_test
  ret void
}

; ===== Frame Information =====
; tigermain -24 56
; readint -8 32
; isdigit 0 24
; skipto 0 16
; readlist -16 40
; merge 0 32
; printint 0 24
; printlist 0 24
; f 0 24
