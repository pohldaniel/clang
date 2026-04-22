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
 -DHAVE_HIDDEN^
 -D_MD -O3 -std=c17 -c

llvm-ar^
 qc libzlib.lib^
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
 inftree9.o

llvm-ranlib^
 libzlib.lib

copy "libzlib.lib" "..\lib\" 

DEL /S/Q *.o
DEL /S/Q libzlib.lib