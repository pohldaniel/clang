clang++^
 -I "./include"^
 src/RtAudio.cpp^
 src/rtaudio_c.cpp^
 -D_CRT_SECURE_NO_WARNINGS -D__WINDOWS_WASAPI__^
 -D_DLL -D_MD -O3 -std=c++17 -c

llvm-ar^
 qc librtaudio.lib^
 RtAudio.o^
 rtaudio_c.o

llvm-ranlib^
 librtaudio.lib

copy "librtaudio.lib" "..\lib\" 

DEL /S/Q *.o
DEL /S/Q librtaudio.lib