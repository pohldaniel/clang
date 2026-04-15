clang++ -MD -O3 -flto -fuse-ld=lld -std=c++17^
 -I "../" -I "../include"^
 -L "../lib"^
 -luser32 -lgdi32 -lshell32 -lmsvcrt -llibcmt^
 -llibglfw3^
 src/main.cpp^
 src/Application.cpp^
 -D_WASM -DNDEBUG^
 -o wireframe.exe