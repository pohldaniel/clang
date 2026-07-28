clang++^
 -I "./" -I "./include/" -I "./include/animation/" -I "../" -I "../include/" -I "../libglfw3/include/" -I "../libimgui/include/" -I "../SDKs/wgpu-dawn/include/" -I "../SDKs/ffmpeg/include/"^
 -L "../lib/" -L "../SDKs/wgpu-dawn/lib/x64/" -L "../SDKs/ffmpeg/lib/"^
 -luser32 -lgdi32 -lshell32 -ldxguid -lonecore -lmsvcrt -llibcmt -lstrmiids -lmfuuid^
 -llibglfw3 -llibimgui -llibassimp -lwgpu -llibfreeimage -llibzlib -llibavutil -llibavcodec -llibswresample -llibavformat -llibswscale^
 src/animation/BoneDescription.cpp^
 src/animation/Bone.cpp^
 src/animation/Animation.cpp^
 src/animation/AnimationState.cpp^
 src/animation/AnimatedModel.cpp^
 src/animation/AnimationController.cpp^
 src/VideoReader.cpp^
 src/BinaryIO.cpp^
 src/Fade.cpp^
 src/Mouse.cpp^
 src/Keyboard.cpp^
 src/Material.cpp^
 src/Transform.cpp^
 src/Mesh.cpp^
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
 states/ShadowMapping.cpp^
 states/SkinnedMesh.cpp^
 states/ComputeParticleLogo.cpp^
 states/PrimitivePicking.cpp^
 states/StencilMask.cpp^
 states/DeferredRendering.cpp^
 states/VolumeRendering.cpp^
 states/OcclusionQuery.cpp^
 states/VideoDecode.cpp^
 states/RenderBundles.cpp^
 states/NuklearGui.cpp^
 states/Isometric.cpp^
 ../Shape/Capsule.cpp^
 ../Shape/Cube.cpp^
 ../Shape/Cylinder.cpp^
 ../Shape/Quad.cpp^
 ../Shape/Segment.cpp^
 ../Shape/Sphere.cpp^
 ../Shape/Spiral.cpp^
 ../Shape/Torus.cpp^
 ../Shape/TorusKnot.cpp^
 ../Shape/Shape.cpp^
 ../WebGPU/WgpContext.cpp^
 ../WebGPU/WgpTexture.cpp^
 ../WebGPU/WgpBuffer.cpp^
 ../WebGPU/WgpMesh.cpp^
 ../WebGPU/WgpModel.cpp^
 ../WebGPU/WgpBatchRenderer.cpp^
 ../WebGPU/WgpFontRenderer.cpp^
 ../WebGPU/WgpRenderer.cpp^
 ../Nuklear/NkContext.cpp^
 -D_MD -D_DLL -O3 -flto -fuse-ld=lld -std=c++17 -D_CRT_SECURE_NO_WARNINGS^
 -Wno-return-type-c-linkage^
 -DNWASM -DNDEBUG -DFREEIMAGE_LIB -DWEBGPU_DAWN^
 -o Examples.exe