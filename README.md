# cheri-examples
cheri-riscv sample c programs

## Build instructions with CMake
1. Create a separate build directory (\<build-dir\>) outside the source directory (\<source-dir\>).
2. Generate the build files using either **Unix Makefiles** or **Ninja**. Currently only the *RISC-V* cheri-purecap toolchain file has been created. From outside the source directory execute: 

   `cmake -B <build-dir> -S <source-dir> -DCMAKE_TOOLCHAIN_FILE=<source-dir>/riscv64-purecap.cmake -DSDK=<cheri-sdk-dir> -G Ninja`
3. Build all the examples 

   `cmake --build <build-dir>`
