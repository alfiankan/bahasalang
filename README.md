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