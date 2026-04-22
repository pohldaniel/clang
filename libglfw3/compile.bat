clang^
 -I "./include"^
 src/context.c^
 src/init.c^
 src/input.c^
 src/monitor.c^
 src/platform.c^
 src/vulkan.c^
 src/window.c^
 src/egl_context.c^
 src/osmesa_context.c^
 src/null_init.c^
 src/null_monitor.c^
 src/null_window.c^
 src/null_joystick.c^
 src/win32_module.c^
 src/win32_time.c^
 src/win32_thread.c^
 src/win32_init.c^
 src/win32_joystick.c^
 src/win32_monitor.c^
 src/win32_window.c^
 src/wgl_context.c^
 -D_CRT_SECURE_NO_WARNINGS -D_GLFW_WIN32^
 -D_MD -O3 -std=c17 -c

llvm-ar^
 qc libglfw3.lib^
 context.o^
 init.o^
 input.o^
 monitor.o^
 platform.o^
 vulkan.o^
 window.o^
 egl_context.o^
 osmesa_context.o^
 null_init.o^
 null_monitor.o^
 null_window.o^
 null_joystick.o^
 win32_module.o^
 win32_time.o^
 win32_thread.o^
 win32_init.o^
 win32_joystick.o^
 win32_monitor.o^
 win32_window.o^
 wgl_context.o


llvm-ranlib^
 libglfw3.lib

copy "libglfw3.lib" "..\lib\" 

DEL /S/Q *.o
DEL /S/Q libglfw3.lib