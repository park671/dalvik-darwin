cmake_minimum_required(VERSION 3.28)
project(libdvm)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstrict-aliasing -Wstrict-aliasing=2 -fno-align-jumps -DWITH_PROFILER -DWITH_DEBUGGER -DDVM_RESOLVER_CACHE=0")

# Enable deadlock prediction
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWITH_DEADLOCK_PREDICTION -DWITH_MONITOR_TRACKING")

# Enable monitor tracking
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWITH_MONITOR_TRACKING")

# Debug dalvik
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWITH_INSTR_CHECKS -DWITH_EXTRA_OBJECT_VALIDATION -DWITH_TRACKREF_CHECKS -DWITH_ALLOC_LIMITS -DDVM_SHOW_EXCEPTION=3 -DEASY_GDB")

# Source files
set(SOURCE_FILES
        AllocTracker.c
        AtomicCache.c
        CheckJni.c
        Ddm.c
        Debugger.c
        DvmDex.c
        Exception.c
        Hash.c
        Init.c
        InlineNative.c
        Inlines.c
        Intern.c
        Jni.c
        JarFile.c
        LinearAlloc.c
        Misc.c
        Native.c
        PointerSet.c
        Profile.c
        Properties.c
        RawDexFile.c
        ReferenceTable.c
        SignalCatcher.c
        StdioConverter.c
        Sync.c
        Thread.c
        UtfString.c
        alloc/clz.c
        alloc/Alloc.c
        alloc/HeapBitmap.c
        alloc/HeapDebug.c
        alloc/HeapSource.c
        alloc/HeapTable.c
        alloc/HeapWorker.c
        alloc/Heap.c
        alloc/MarkSweep.c
        alloc/DdmHeap.c
        analysis/CodeVerify.c
        analysis/DexOptimize.c
        analysis/DexVerify.c
        analysis/ReduceConstants.c
        analysis/RegisterMap.c
        analysis/VerifySubs.c
        interp/Interp.c
        interp/Stack.c
        jdwp/ExpandBuf.c
        jdwp/JdwpAdb.c
        jdwp/JdwpConstants.c
        jdwp/JdwpEvent.c
        jdwp/JdwpHandler.c
        jdwp/JdwpMain.c
        jdwp/JdwpSocket.c
        native/InternalNative.c
        native/dalvik_system_DexFile.c
        native/dalvik_system_VMDebug.c
        native/dalvik_system_VMRuntime.c
        native/dalvik_system_VMStack.c
        native/dalvik_system_Zygote.c
        native/java_lang_Class.c
        native/java_lang_Object.c
        native/java_lang_Runtime.c
        native/java_lang_String.c
        native/java_lang_System.c
        native/java_lang_SystemProperties.c
        native/java_lang_Throwable.c
        native/java_lang_VMClassLoader.c
        native/java_lang_VMThread.c
        native/java_lang_reflect_AccessibleObject.c
        native/java_lang_reflect_Array.c
        native/java_lang_reflect_Constructor.c
        native/java_lang_reflect_Field.c
        native/java_lang_reflect_Method.c
        native/java_lang_reflect_Proxy.c
        native/java_security_AccessController.c
        native/java_util_concurrent_atomic_AtomicLong.c
        native/org_apache_harmony_dalvik_NativeTestTarget.c
        native/org_apache_harmony_dalvik_ddmc_DdmServer.c
        native/org_apache_harmony_dalvik_ddmc_DdmVmInternal.c
        native/sun_misc_Unsafe.c
        oo/AccessCheck.c
        oo/Array.c
        oo/Class.c
        oo/Object.c
        oo/Resolve.c
        oo/TypeCheck.c
        reflect/Annotation.c
        reflect/Proxy.c
        reflect/Reflect.c
        test/TestHash.c
)

# Optional HPROF sources
list(APPEND SOURCE_FILES
        hprof/Hprof.c
        hprof/HprofClass.c
        hprof/HprofHeap.c
        hprof/HprofOutput.c
        hprof/HprofString.c
)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWITH_HPROF=1")
# Include unreachable HPROF features
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWITH_HPROF_UNREACHABLE=1")
# Include HPROF stack features
list(APPEND SOURCE_FILES
        hprof/HprofStack.c
        hprof/HprofStackFrame.c
)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWITH_HPROF_STACK=1")

# Include directories
include_directories(
        ${JNI_H_INCLUDE}
        dalvik
        dalvik/vm
        external/zlib
        ../
        ./
        ../include
        $(KERNEL_HEADERS)
)

# Define the library
add_library(libdvm SHARED ${SOURCE_FILES})

# Link libraries
target_link_libraries(libdvm
        log
        cutils
        nativehelper
        z
        dl
        pthread
        rt
)
