cmake_minimum_required(VERSION 3.28)
project(dalvik_darwin_c99 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_C_FLAGS -O2)
set(CMAKE_CXX_FLAGS -O2)

add_compile_options()

find_package(ZLIB REQUIRED)

find_package(PkgConfig)
pkg_check_modules(LIBFFI REQUIRED IMPORTED_TARGET libffi)

# Include directories
include_directories(
        ./
        ./safe-iop
        ./fdlibm
        ./bionic
        ./ashmem
        ./libnativehelper/include/nativehelper
        ./vm
        ./include
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstrict-aliasing -Wstrict-aliasing=2 -fno-align-jumps -DWITH_PROFILER -DWITH_DEBUGGER -DDVM_RESOLVER_CACHE=0")

# Enable deadlock prediction
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWITH_DEADLOCK_PREDICTION -DWITH_MONITOR_TRACKING")

# Enable monitor tracking
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWITH_MONITOR_TRACKING")

# Debug dalvik
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWITH_INSTR_CHECKS -DWITH_EXTRA_OBJECT_VALIDATION -DWITH_TRACKREF_CHECKS -DWITH_ALLOC_LIMITS -DDVM_SHOW_EXCEPTION=3 -DEASY_GDB")

file(GLOB_RECURSE FDLIBM "fdlibm/*.c")
add_library(dalvik_darwin_fdlibm STATIC
        ${FDLIBM}
)

file(GLOB_RECURSE LIB_CORE "libcore/*.c" "libcore/*.cpp")
add_library(dalvik_darwin_lib_core STATIC
        ${LIB_CORE}
)
set_target_properties(dalvik_darwin_lib_core PROPERTIES
        CXX_STANDARD 98
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS ON
)
target_link_libraries(dalvik_darwin_lib_core dalvik_darwin_fdlibm)

file(GLOB_RECURSE JNI_HELPER "libnativehelper/*.c")
add_library(dalvik_darwin_jni_helper STATIC
        ${JNI_HELPER}
)
target_link_libraries(dalvik_darwin_jni_helper dalvik_darwin_lib_core)

file(GLOB_RECURSE ASHMEM "ashmem/*.c")
add_library(dalvik_darwin_ashmem STATIC
        ${ASHMEM}
)

file(GLOB_RECURSE LIBCUTILS "libcutils/*.c" "libcutils/*.cpp")
file(GLOB_RECURSE SAFE_IOP "safe-iop/*.c")
add_library(dalvik_darwin_libcutils STATIC
        ${LIBCUTILS}
        ${SAFE_IOP}
)

file(GLOB_RECURSE VM "vm/*.c")
add_library(dalvik_darwin_vm STATIC
        ${VM}
)
target_link_libraries(dalvik_darwin_vm
        dalvik_darwin_ashmem
        dalvik_darwin_libcutils
        dalvik_darwin_jni_helper
        PkgConfig::LIBFFI
        ZLIB::ZLIB)

file(GLOB_RECURSE MAIN_DIR "DexList.c" "Main.c")
file(GLOB_RECURSE LIB_DEX "libdex/*.c")
file(GLOB_RECURSE SAFE_IOP "safe-iop/*.c")
add_executable(dalvik_darwin_c99
        ${MAIN_DIR}
        ${LIB_DEX}
)
target_link_libraries(dalvik_darwin_c99 dalvik_darwin_vm ZLIB::ZLIB)

file(GLOB_RECURSE DEX_OPT "dexopt/*.c" "DexList.c")
add_executable(dalvik_darwin_c99_dexopt
        ${DEX_OPT}
        ${LIB_DEX}
)
target_link_libraries(dalvik_darwin_c99_dexopt dalvik_darwin_vm ZLIB::ZLIB)

add_dependencies(dalvik_darwin_c99 dalvik_darwin_c99_dexopt)
