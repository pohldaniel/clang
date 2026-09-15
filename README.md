# clang

For compiling FreeImage go to https://github.com/pohldaniel/FreeImage_emscripten add a subfolder build and use the following commad prompts ou oft this folder

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cmake .. -G "Unix Makefiles" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release -DBUILD_JXR=ON -DBUILD_LIBRAWLITE=ON -DBUILD_OPENEXR=ON -DBUILD_ZLIB=OFF

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cmake --build .

For compiling Assimp I have used the following configuration commands

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cmake ..  -G "Unix Makefiles" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_ZLIB=OFF -DASSIMP_BUILD_TESTS=OFF -DASSIMP_WARNINGS_AS_ERRORS=OFF -DASSIMP_BUILD_USE_CCACHE=OFF -DASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT=OFF -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF -DASSIMP_BUILD_GLTF_IMPORTER=ON -DASSIMP_BUILD_OBJ_IMPORTER=ON -DASSIMP_BUILD_COLLADA_IMPORTER=ON -DASSIMP_BUILD_FBX_IMPORTER=ON -DASSIMP_BUILD_PLY_IMPORTER=ON -DZLIB_INCLUDE_DIR=<PATH_TO_ZLIB_INCLUDE>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cmake --build .

I also had to tweak the CMakeLists.txt file for building Assimp static without zlib.

&nbsp;&nbsp;IF( NOT BUILD_SHARED_LIBS )  
&nbsp;&nbsp;&nbsp;&nbsp;ADD_DEFINITIONS(-DASSIMP_BUILD_NO_OWN_ZLIB)  
&nbsp;&nbsp;&nbsp;&nbsp;INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})  
&nbsp;&nbsp;&nbsp;&nbsp;message(STATUS "Zlib include path " ${ZLIB_INCLUDE_DIR})  
&nbsp;&nbsp;ELSE()  
&nbsp;&nbsp;&nbsp;&nbsp;message( FATAL_ERROR  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"Build configured with -DASSIMP_BUILD_ZLIB=OFF but unable to find zlib"  
&nbsp;&nbsp;&nbsp;&nbsp;)  
&nbsp;&nbsp;ENDIF()  

./configure --prefix=installed --cc=clang-cl --toolchain=msvc --target-os=win64 --arch=x86_64 --ld=lld-link --disable-doc --disable-shared --disable-debug --enable-gpl --enable-stripping --enable-optimizations --enable-static --extra-ldflags="/OPT:REF /OPT:ICF" --extra-cxxflags="/clang:-Oz -MD" --extra-cflags="/clang:-Oz -MD"

./configure --prefix=installed --cc=clang-cl --toolchain=msvc --target-os=win64 --arch=x86_64 --ld=lld-link --disable-doc --disable-static --disable-debug --enable-gpl --enable-stripping --enable-optimizations --enable-shared --extra-ldflags="/OPT:REF /OPT:ICF" --extra-cxxflags="/clang:-Oz -MD" --extra-cflags="/clang:-Oz -MD"


OpenAL

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cmake --build . cmake .. -G "Unix Makefiles" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release -DLIBTYPE=STATIC -DALSOFT_BACKEND_OPENSL=OFF -DALSOFT_BACKEND_WAVE=OFF -DALSOFT_EMBED_HRTF_DATA=OFF -DALSOFT_EXAMPLES=OFF -DALSOFT_UTILS=OFF -DALSOFT_TESTS=OFF -DALSOFT_INSTALL=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_CXX_FLAGS="-fvisibility=hidden -fvisibility-inlines-hidden -fdata-sections -ffunction-sections" -DCMAKE_C_FLAGS="-fvisibility=hidden -fdata-sections -ffunction-sections"

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cmake --build .

Bullet Physics

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cmake .. -G "Unix Makefiles" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release -DBUILD_EXTRAS=OFF -DBUILD_BULLET3=OFF -DBUILD_BULLET2_DEMOS=OFF -DBUILD_CPU_DEMOS=OFF -DBUILD_OPENGL3_DEMOS=OFF -DBUILD_UNIT_TESTS=OFF -DBUILD_PYBULLET=OFF -DUSE_GLUT=OFF -DUSE_GRAPHICAL_BENCHMARK=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cmake --build .

RtAudio

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cmake .. -G "Unix Makefiles" -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DRTAUDIO_API_WASAPI=ON

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cmake --build .