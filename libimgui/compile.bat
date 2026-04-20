clang++^
 -I "./" -I "../include/" -I "../SDKs/wgpu-dawn/include/"^
 backends/imgui_impl_glfw.cpp^
 backends/imgui_impl_wgpu.cpp^
 imgui_draw.cpp^
 imgui_tables.cpp^
 imgui_widgets.cpp^
 imgui.cpp^
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