clang++^
 -I "./" -I "./include/" -I "../" -I "../include/" -I "../libglfw3/include/" -I "../libimgui/include/" -I "../SDKs/wgpu-dawn/include/"^
 -L "../lib/" -L "../SDKs/wgpu-dawn/lib/x64/"^
 -luser32 -lgdi32 -lshell32 -ldxguid -lonecore -lmsvcrt -llibcmt^
 -llibglfw3 -llibimgui -llibassimp -lwgpu -llibfreeimage -llibzlib^
 src/Mouse.cpp^
 src/Material.cpp^
 src/Transform.cpp^
 src/Model.cpp^
 src/ObjModel.cpp^
 src/AssimpModel.cpp^
 src/Camera.cpp^
 src/CharacterSet.cpp^
 src/Application.cpp^
 src/main.cpp^
 states/StateMachine.cpp^
 states/Wireframe.cpp^
 states/ImageBasedLighting.cpp^
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
 -D_MD -D_DLL -O3 -flto -fuse-ld=lld -std=c++17 -D_CRT_SECURE_NO_WARNINGS^
 -DNWASM -DNDEBUG -DFREEIMAGE_LIB -DWEBGPU_DAWN^
 -o Examples.exe