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
