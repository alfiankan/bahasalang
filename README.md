## Bahasa Language



Bahasa is general purpose programming language that is designed in indonesian language just for fun not for production use yaaa.

```bash
modul main

fungsi fizz_buzz(jumlah: int) -> int {
    jika jumlah modulo 3 adalah 0 {
        tampilkan("%s\n", "Fizz")
    }
    jika jumlah modulo 5 adalah 0 {
        tampilkan("%s\n", "Buzz")
    }
    jika jumlah adalah 0 {
        <- 0
    }
    tidur(1)
    <- fizz_buzz(jumlah - 1)
}

fungsi main() -> int {
    fizz_buzz(10)

    mutasi angka_angka: koleksi[int] = [1,2,3,4,5]
    mutasi hasil: int = angka_angka.2
    tampilkan("KE 2: %d \n", hasil)

    mutasi angka_angka: koleksi[int] = [1,2,3,4,5]
    mutasi hasil: int = angka_angka.2
    tampilkan("KE 2: %d \n", hasil)
    tampilkan("KE 3: %d \n", angka_angka.3)

    abaikan {
        mutasi error: int = angka_angka.100
        tampilkan("ke 1: %d \n", error)
    }
    abaikan {
        mutasi tidak_error: int = angka_angka.1
        tampilkan("ke 1: %d \n", tidak_error)
    }
    tampilkan("Pesan: %s\n", "selesei")

    <- 0
}
```

### Components

- [x] Lexer
- [x] Parser
- [x] AST
- [x] Code Generator
- [-] Runtime GC

### Progress

- [x] function
- [x] variable
- [x] arithmetic
- [x] logical
- [x] control flow
- [x] Array

### How to build

```bash
mkdir build
cd build
cmake -G Ninja ..
ninja
```

### Prebuilt Toolchain
> just download and try at your PC


| Platform | Architecture | Download |
|----------|--------------|----------|
| Linux    | x86_64       | [bahasa-linux-amd64](/prebuilt/bahasa-linux-amd64) |
| macOS    | arm64        | [bahasa-darwin-arm64](prebuilt/bahasa-darwin-arm64) |



### How to

```bash
alfiankan@ubuntu-x86-x64:~$ ./bahasa-linux-amd64
Penggunaan: ./bahasa-linux-amd64 <perintah> [opsi] <berkas_sumber>

Perintah:
  ir       Kompilasi kode sumber ke LLVM IR
  susun    Kompilasi kode sumber ke program
  jalankan Kompilasi dan jalankan program
  ast      Tampilkan AST
  token    Tampilkan daftar token

Opsi:
  -o <berkas>   Berkas keluaran (default: a.out untuk susun/jalankan, <nama_modul>.ll untuk ir)
alfiankan@ubuntu-x86-x64:~$ vim main.bh
alfiankan@ubuntu-x86-x64:~$ ./bahasa-linux-amd64 ir main.bh
alfiankan@ubuntu-x86-x64:~$ cat main.ll
; ModuleID = 'main'
source_filename = "main"

@0 = private unnamed_addr constant [10 x i8] c"Hello %s\0A\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"World\00", align 1

declare i32 @printf(ptr, ...)

define void @tampilkan(ptr %0, i32 %1) {
entry:
  %2 = call i32 (ptr, ...) @printf(ptr %0, i32 %1)
  ret void
}

define i32 @main() {
entry:
  call void @tampilkan(ptr @0, ptr @1)
  ret i32 0
}
alfiankan@ubuntu-x86-x64:~$ ./bahasa-linux-amd64 ast main.bh
Abstract Syntax Tree:
└── Function: main
    ├── Call: tampilkan
    │   ├── String: "Hello %s"
    │   └── String: "World"
    └── Return
        └── Number: 0
alfiankan@ubuntu-x86-x64:~$ ./bahasa-linux-amd64 token main.bh
+--------------+-----------------+------+
| Type         | Lexeme          | Line |
+--------------+-----------------+------+
| MODUL        | modul           |     1|
| IDENTIFIER   | main            |     1|
| FUNCTION     | fungsi          |     5|
| IDENTIFIER   | main            |     5|
| LPAREN       | (               |     5|
| RPAREN       | )               |     5|
| ARROW        | ->              |     5|
| INT          | int             |     5|
| LBRACE       | {               |     5|
| IDENTIFIER   | tampilkan       |     6|
| LPAREN       | (               |     6|
| STRING       | Hello %s\n      |     6|
| COMMA        | ,               |     6|
| STRING       | World           |     6|
| RPAREN       | )               |     6|
| RETURN_ARROW | <-              |     7|
| NUMBER       | 0               |     7|
| RBRACE       | }               |     8|
| END          |                 |     9|
+--------------+-----------------+------+
alfiankan@ubuntu-x86-x64:~$ ./bahasa-linux-amd64 susun main.bh
alfiankan@ubuntu-x86-x64:~$ ./
.ssh/               a.out               bahasa-linux-amd64
alfiankan@ubuntu-x86-x64:~$ ./a.out
Hello World
alfiankan@ubuntu-x86-x64:~$ ./bahasa-linux-amd64 jalankan main.bh
Hello World
alfiankan@ubuntu-x86-x64:~$
```

#### Run

```bash
./Bahasa jalankan main.bh
```

#### Compile

```bash
./Bahasa susun main.bh -o programku
```

#### Generate LLVM IR
```bash
./Bahasa ir main.bh 
```

#### Generate AST
```bash
./Bahasa ast main.bh 
```
#### Generate TOKENS
```bash
./Bahasa token main.bh 
```

### Language Syntax and design

#### Primitive Types

- int (32bit)
- int array (32bit)

#### comment
```bash
fungsi main() -> int {
    -- this is comment 1
    -- this is coment 2
    -> 0
}
```

#### function

```bash
fungsi main() -> int {
    -> 0 -- return 0
}
```

#### variable decl mutable

```bash
mutasi a = 10
a = 70
```


#### arithmetic and logic

```bash
modul main

fungsi main() -> int {
    mutasi a: int = 10
    a = 90
    mutasi b: int = 20
    mutasi c: int = a + b
    tampilkan("Hasil: %d\n", c)
    mutasi d: int = a - b
    tampilkan("Hasil: %d\n", d)
    mutasi e: int = a * b
    tampilkan("Hasil: %d\n", e)
    mutasi f: int = a / b
    tampilkan("Hasil: %d\n", f)
    mutasi g: int = a modulo b
    tampilkan("Hasil: %d\n", g)
    mutasi h: int = a > b
    tampilkan("Hasil: %d\n", h)
    mutasi i: int = a < b
    tampilkan("Hasil: %d\n", i)

    mutasi j: int = a >= b
    tampilkan("Hasil: %d\n", j)
    mutasi k: int = a <= b
    tampilkan("Hasil: %d\n", k)
    mutasi l: int = a dan b
    tampilkan("Hasil: %d\n", l)
    mutasi m: int = a atau b
    tampilkan("Hasil: %d\n", m)
    <- 0
}

#### control flow

```bash
jika a > b {

}
```

#### try block

```bash
modul main

fungsi main() -> int {
    mutasi angka_angka: koleksi[int] = [1,2,3,4,5]
    mutasi hasil: int = angka_angka.2
    tampilkan("KE 2: %d \n", hasil)
    tampilkan("KE 3: %d \n", angka_angka.3)

    abaikan {
        mutasi error: int = angka_angka.100
        tampilkan("ke 1: %d \n", error)
    }
    tampilkan("Pesan: %s\n", "selesei")
    <- 0
}
```