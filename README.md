# Compilers

## Roadmap

- [ ] Grammar parser


# HLVM

The name comes from "High-Level Virtual Machine". OK, I was copying the name from LLVM :) .

## Prerequisites

- CMake
- A C++ compiler with C++26 support
- Conan

## Building

```bash
conan install . -of build -b missing
cmake --preset=conan-default
cmake --build build
```


