# Brainfuck
A Brainfuck compiler targeting x86-64 Assembly and ANSI C.

```sh
$ ./compile.sh
Compilation succeeded!
$ ./fetch_examples.sh
Fetched some examples!
```

```sh
$ ./brainfuck c89 examples/game_of_life.b /tmp/game_of_life.c
$ gcc -std=c89 /tmp/game_of_life.c -o game_of_life
$ ./game_of_life
```

```sh
$ ./brainfuck x64 examples/mandelbrot.b /tmp/mandelbrot.asm
$ nasm /tmp/mandelbrot.asm -f elf64 -o /tmp/mandelbrot.o
$ ld /tmp/mandelbrot.o -o mandelbrot
$ ./mandelbrot
```
