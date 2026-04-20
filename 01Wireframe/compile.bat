clang++^
 -I "./" -I "./include/" -I "../" -I "../include/" -I "../libimgui" -I "../libimgui/backends" -I "../SDKs/wgpu-dawn/include/"^
 -L "../lib/" -L "../SDKs/wgpu-dawn/lib/x64/"^
 -luser32 -lgdi32 -lshell32 -ldxguid -lonecore -lmsvcrt -llibcmt^
 -llibglfw3 -llibimgui -lwgpu -llibfreeimage -llibjpeg -llibjxr -llibopenexr -llibopenjpeg -llibpng -llibrawlite -llibtiff4 -llibwebp -llibzlib^
 src/Mouse.cpp^
 src/Material.cpp^
 src/Transform.cpp^
 src/Model.cpp^
 src/ObjModel.cpp^
 src/Camera.cpp^
 src/CharacterSet.cpp^
 src/Application.cpp^
 src/main.cpp^
 states/StateMachine.cpp^
 states/Wireframe.cpp^
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
 -DNWASM -DNDEBUG -DFREEIMAGE_LIB -DWEBGPU_DAWN^
 -D_MD -D_DLL -O3 -flto -fuse-ld=lld -std=c++17^
 -o wireframe.exe