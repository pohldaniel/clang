clang++^
 -I "./include"^
 src/RtAudio.cpp^
 src/rtaudio_c.cpp^
 -DUNICODE -D_CRT_SECURE_NO_WARNINGS -D__WINDOWS_WASAPI__ -DRTAUDIO_EXPORT^
 -O3 -D_MD -Xclang --dependent-lib=msvcrt -std=c++17 -c

clang++^
 -Xlinker /MANIFEST:EMBED -Xlinker /implib:rtaudio.lib^
 -nostartfiles -nostdlib -O3 -D_MD -Xclang --dependent-lib=msvcrt -fuse-ld=lld-link -shared^
 -luser32 -lgdi32 -lwinspool -lshell32^
 -lole32 -lksuser -luuid -ladvapi32^
 RtAudio.o^
 rtaudio_c.o^
 -o rtaudio.dll

copy "rtaudio.lib" "..\lib\" 
copy "rtaudio.dll" "..\lib\" 

DEL /S/Q *.o
DEL /S/Q rtaudio.dll
DEL /S/Q rtaudio.lib