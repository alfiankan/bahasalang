## Bahasa Language

Bahasa is general purpose programming language that is designed in indonesian language just for fun not for production use yaaa.

### Components

- [ ] Lexer
- [ ] Parser
- [ ] AST
- [ ] Code Generator
- [ ] Runtime GC

### Progress

- [ ] function
- [ ] variable
- [ ] arithmetic
- [ ] logical
- [ ] control flow


### How to build

```bash
mkdir build
cd build
cmake -G Ninja ..
ninja
```

### How to run

```bash
./Bahasa main.bh
```

### Language Syntax and design

#### Primitive Types

- int // os bit
- uint // os bit
- bool // true, false equal to int
- float // os bit
#### function


```
fungsi main() -> int1 { // function main return int
    -> 0 // return 0
}
```

#### variable immutable

```
konstan a = 10 // constant variable a with value 10
```

#### variable mutable

```
variabel a = 10 // mutable variable a with value 10
```

#### arithmetic

```
a + b // add a and b
a - b // subtract a and b
a * b // multiply a and b
a / b // divide a and b
a % b // modulus a and b
a ** b // power a and b
```

#### logical

```
a dan b 
a atau b
bukan a dan bukan b
```

#### control flow

```
jika a > b {

}
```

#### while loop

```
selama a < b {

}
```