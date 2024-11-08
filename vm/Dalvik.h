/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * All-inclusive internal header file.  Include this to get everything useful.
 */
#ifndef _DALVIK_DALVIK
#define _DALVIK_DALVIK

#include <pthread.h>

#include "Common.h"
#include "Inlines.h"
#include "Misc.h"
#include "Bits.h"
#include "libdex/SysUtil.h"
#include "libdex/DexFile.h"
#include "libdex/DexProto.h"
#include "libdex/ZipArchive.h"
#include "analysis/ReduceConstants.h"
#include "DvmDex.h"
#include "RawDexFile.h"
#include "RamOdexFile.h"
#include "Sync.h"
#include "oo/Object.h"
#include "Native.h"
#include "native/InternalNative.h"

#include "DalvikVersion.h"
#include "Debugger.h"
#include "Profile.h"
#include "UtfString.h"
#include "Intern.h"
#include "ReferenceTable.h"
#include "AtomicCache.h"
#include "Thread.h"
#include "Ddm.h"
#include "Hash.h"
#include "interp/Stack.h"
#include "oo/Class.h"
#include "oo/Resolve.h"
#include "oo/Array.h"
#include "Exception.h"
#include "alloc/Alloc.h"
#include "alloc/HeapDebug.h"
#include "alloc/HeapWorker.h"
#include "alloc/GC.h"
#include "oo/AccessCheck.h"
#include "JarFile.h"
#include "Properties.h"
#include "jdwp/Jdwp.h"
#include "SignalCatcher.h"
#include "StdioConverter.h"
#include "JniInternal.h"
#include "LinearAlloc.h"
#include "analysis/DexVerify.h"
#include "analysis/DexOptimize.h"
#include "Init.h"
#include "libdex/OpCode.h"
#include "libdex/InstrUtils.h"
#include "AllocTracker.h"
#include "PointerSet.h"
#include "Globals.h"
#include "reflect/Reflect.h"
#include "oo/TypeCheck.h"
#include "Atomic.h"
#include "interp/Interp.h"
#include "InlineNative.h"

#endif /*_DALVIK_DALVIK*/
