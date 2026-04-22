clang^
 -I "./include"^
 src/adler32.c^
 src/compress.c^
 src/crc32.c^
 src/deflate.c^
 src/gzclose.c^
 src/gzlib.c^
 src/gzread.c^
 src/gzwrite.c^
 src/inflate.c^
 src/infback.c^
 src/inftrees.c^
 src/inffast.c^
 src/trees.c^
 src/uncompr.c^
 src/zutil.c^
 src/gvmat64.S^
 src/infback9.c^
 src/inftree9.c^
 -DHAVE_HIDDEN -DZLIB_DLL^
 -O3 -D_MD -Xclang --dependent-lib=msvcrt -std=c17 -c

clang++^
 -Xlinker /MANIFEST:EMBED -Xlinker /implib:zlib.lib^
 -nostartfiles -nostdlib -O3 -D_MD -Xclang --dependent-lib=msvcrt -fuse-ld=lld-link -shared^
 adler32.o^
 compress.o^
 crc32.o^
 deflate.o^
 gzclose.o^
 gzlib.o^
 gzread.o^
 gzwrite.o^
 inflate.o^
 infback.o^
 inftrees.o^
 inffast.o^
 trees.o^
 uncompr.o^
 zutil.o^
 gvmat64.o^
 infback9.o^
 inftree9.o^
 -o zlib.dll

copy "zlib.lib" "..\lib\" 
copy "zlib.dll" "..\lib\" 

DEL /S/Q *.o
DEL /S/Q zlib.dll
DEL /S/Q zlib.lib