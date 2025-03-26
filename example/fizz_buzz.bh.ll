; ModuleID = 'main'
source_filename = "main"

@0 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@1 = private unnamed_addr constant [5 x i8] c"Fizz\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@3 = private unnamed_addr constant [5 x i8] c"Buzz\00", align 1

declare i32 @printf(ptr, ...)

declare i32 @printf.1(ptr, ...)

define void @tampilkan(ptr %0, i32 %1) {
entry:
  %2 = call i32 (ptr, ...) @printf.1(ptr %0, i32 %1)
  ret void
}

define i32 @fizz_buzz(i32 %n_iter) {
entry:
  %modtmp = srem i32 %n_iter, 3
  %eqtmp = icmp eq i32 %modtmp, 0
  %0 = zext i1 %eqtmp to i32
  %ifcond = icmp ne i32 %0, 0
  br i1 %ifcond, label %then, label %ifcont

then:                                             ; preds = %entry
  call void @tampilkan(ptr @0, ptr @1)
  br label %ifcont

ifcont:                                           ; preds = %then, %entry
  %modtmp1 = srem i32 %n_iter, 5
  %eqtmp2 = icmp eq i32 %modtmp1, 0
  %1 = zext i1 %eqtmp2 to i32
  %ifcond3 = icmp ne i32 %1, 0
  br i1 %ifcond3, label %then4, label %ifcont5

then4:                                            ; preds = %ifcont
  call void @tampilkan(ptr @2, ptr @3)
  br label %ifcont5

ifcont5:                                          ; preds = %then4, %ifcont
  %eqtmp6 = icmp eq i32 %n_iter, 0
  %2 = zext i1 %eqtmp6 to i32
  %ifcond7 = icmp ne i32 %2, 0
  br i1 %ifcond7, label %then8, label %ifcont9

then8:                                            ; preds = %ifcont5
  ret i32 0

ifcont9:                                          ; preds = %ifcont5
  %subtmp = sub i32 %n_iter, 1
  %calltmp = call i32 @fizz_buzz(i32 %subtmp)
  ret i32 %calltmp
}

define i32 @main() {
entry:
  %calltmp = call i32 @fizz_buzz(i32 10)
  ret i32 0
}
