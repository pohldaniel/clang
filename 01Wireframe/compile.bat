clang++^
 -I "./include/" -I "../" -I "../include/" -I "../SDKs/wgpu-dawn/include/"^
 -L "../lib/" -L "../SDKs/wgpu-dawn/lib/x64/"^
 -luser32 -lgdi32 -lshell32 -ldxguid -lonecore -lmsvcrt -llibcmt^
 -llibglfw3 -lwgpu -llibfreeimage -llibjpeg -llibjxr -llibopenexr -llibopenjpeg -llibpng -llibrawlite -llibtiff4 -llibwebp -llibzlib^
 src/Mouse.cpp^
 src/Material.cpp^
 src/Transform.cpp^
 src/Model.cpp^
 src/ObjModel.cpp^
 src/Camera.cpp^
 src/CharacterSet.cpp^
 src/Application.cpp^
 src/main.cpp^
 ../Shape/Cube.cpp^
 ../Shape/Sphere.cpp^
 ../Shape/Torus.cpp^
 ../Shape/TorusKnot.cpp^
 ../Shape/Spiral.cpp^
 ../Shape/Shape.cpp^
 ../WebGPU/WgpContext.cpp^
 ../WebGPU/WgpTexture.cpp^
 ../WebGPU/WgpBuffer.cpp^
 ../WebGPU/WgpMesh.cpp^
 ../WebGPU/WgpModel.cpp^
 ../WebGPU/WgpBatchRenderer.cpp^
 ../WebGPU/WgpFontRenderer.cpp^
 -D_WASM -DNDEBUG -DFREEIMAGE_LIB^
 -D_MD -D_DLL -O3 -flto -fuse-ld=lld -std=c++17^
 -o wireframe.exe