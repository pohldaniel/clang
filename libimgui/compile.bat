clang++^
 -I "./include/" -I "../include/" -I "../SDKs/wgpu-dawn/include/"^
 src/backends/imgui_impl_glfw.cpp^
 src/backends/imgui_impl_wgpu.cpp^
 src/imgui_draw.cpp^
 src/imgui_tables.cpp^
 src/imgui_widgets.cpp^
 src/imgui.cpp^
 -DIMGUI_IMPL_WEBGPU_BACKEND_DAWN^
 -D_MD -O3 -std=c++17 -c

llvm-ar^
 qc libimgui.lib^
 imgui_impl_glfw.o^
 imgui_impl_wgpu.o^
 imgui_draw.o^
 imgui_tables.o^
 imgui_widgets.o^
 imgui.o

llvm-ranlib^
 libimgui.lib

copy "libimgui.lib" "..\lib\" 

DEL /S/Q *.o
DEL /S/Q libimgui.lib