# Snake Game

This is a simple snake game implemented in C. It can be compiled using either `clang` or `gcc` compilers. The game requires linking to the `User32` library on Windows.

## Compilation Instructions

### Using `clang`:
```bash
clang snake.c -o snake.exe -lUser32
```
### Using `gcc`:
```bash
gcc -std=c99 snake.c -o snake.exe -lUser32
```
