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
 * Implementation of java.lang.reflect.Proxy.
 *
 * Traditionally this is implemented entirely in interpreted code,
 * generating bytecode that defines the proxy class.  Dalvik doesn't
 * currently support this approach, so we generate the class directly.  If
 * we add support for DefineClass with standard classfiles we can
 * eliminate this.
 */
#include "Dalvik.h"

#include <stdlib.h>

// fwd
static bool returnTypesAreCompatible(Method* baseMethod, Method* subMethod);
static bool gatherMethods(ArrayObject* interfaces, Method*** pMethods,\
    ArrayObject** pThrows, int* pMethodCount);
static int copyWithoutDuplicates(Method** allMethods, int allCount,
    Method** outMethods, ArrayObject* throws);
static bool createExceptionClassList(const Method* method,
    PointerSet** pThrows);
static void updateExceptionClassList(const Method* method, PointerSet* throws);
static void createConstructor(ClassObject* clazz, Method* meth);
static void createHandlerMethod(ClassObject* clazz, Method* dstMeth,
    const Method* srcMeth);
static void proxyConstructor(const u8* args, JValue* pResult,
    const Method* method, Thread* self);
static void proxyInvoker(const u8* args, JValue* pResult,
    const Method* method, Thread* self);
static bool mustWrapException(const Method* method, const Object* throwable);

/* private static fields in the Proxy class */
#define kThrowsField    0


/*
 * Perform Proxy setup.
 */
bool dvmReflectProxyStartup()
{
    /*
     * Standard methods we must provide in our proxy.
     */
    Method* methE;
    Method* methH;
    Method* methT;
    Method* methF;
    methE = dvmFindVirtualMethodByDescriptor(gDvm.classJavaLangObject,
                "equals", "(Ljava/lang/Object;)Z");
    methH = dvmFindVirtualMethodByDescriptor(gDvm.classJavaLangObject,
                "hashCode", "()I");
    methT = dvmFindVirtualMethodByDescriptor(gDvm.classJavaLangObject,
                "toString", "()Ljava/lang/String;");
    methF = dvmFindVirtualMethodByDescriptor(gDvm.classJavaLangObject,
                "finalize", "()V");
    if (methE == NULL || methH == NULL || methT == NULL || methF == NULL) {
        LOGE("Could not find equals/hashCode/toString/finalize in Object\n");
        return false;
    }
    gDvm.voffJavaLangObject_equals = methE->methodIndex;
    gDvm.voffJavaLangObject_hashCode = methH->methodIndex;
    gDvm.voffJavaLangObject_toString = methT->methodIndex;
    gDvm.voffJavaLangObject_finalize = methF->methodIndex;

    /*
     * The prototype signature needs to be cloned from a method in a
     * "real" DEX file.  We declared this otherwise unused method just
     * for this purpose.
     */
    ClassObject* proxyClass;
    Method* meth;
    proxyClass = dvmFindSystemClassNoInit("Ljava/lang/reflect/Proxy;");
    if (proxyClass == NULL) {
        LOGE("No java.lang.reflect.Proxy\n");
        return false;
    }
    meth = dvmFindDirectMethodByDescriptor(proxyClass, "constructorPrototype",
                "(Ljava/lang/reflect/InvocationHandler;)V");
    if (meth == NULL) {
        LOGE("Could not find java.lang.Proxy.constructorPrototype()\n");
        return false;
    }
    gDvm.methJavaLangReflectProxy_constructorPrototype = meth;

    /*
     * Get the offset of the "h" field in Proxy.
     */
    gDvm.offJavaLangReflectProxy_h = dvmFindFieldOffset(proxyClass, "h",
        "Ljava/lang/reflect/InvocationHandler;");
    if (gDvm.offJavaLangReflectProxy_h < 0) {
        LOGE("Unable to find 'h' field in java.lang.Proxy\n");
        return false;
    }

    return true;
}


/*
 * Generate a proxy class with the specified name, interfaces, and loader.
 * "interfaces" is an array of class objects.
 *
 * The Proxy.getProxyClass() code has done the following:
 *  - Verified that "interfaces" contains only interfaces
 *  - Verified that no interface appears twice
 *  - Prepended the package name to the class name if one or more
 *    interfaces are non-public
 *  - Searched for an existing instance of an appropriate Proxy class
 *
 * On failure we leave a partially-created class object sitting around,
 * but the garbage collector will take care of it.
 */
ClassObject* dvmGenerateProxyClass(StringObject* str, ArrayObject* interfaces,
    Object* loader)
{
    int result = -1;
    char* nameStr = NULL;
    Method** methods = NULL;
    ArrayObject* throws = NULL;
    ClassObject* newClass = NULL;
    int i;
    
    nameStr = dvmCreateCstrFromString(str);
    if (nameStr == NULL) {
        dvmThrowException("Ljava/lang/IllegalArgumentException;",
            "missing name");
        goto bail;
    }

    LOGV("+++ Generate proxy class '%s' %p from %d interface classes\n",
        nameStr, loader, interfaces->length);


    /*
     * Characteristics of a Proxy class:
     * - concrete class, public and final
     * - superclass is java.lang.reflect.Proxy
     * - implements all listed interfaces (req'd for instanceof)
     * - has one method for each method in the interfaces (for duplicates,
     *   the method in the earliest interface wins)
     * - has one constructor (takes an InvocationHandler arg)
     * - has overrides for hashCode, equals, and toString (these come first)
     * - has one field, a reference to the InvocationHandler object, inherited
     *   from Proxy
     *
     * TODO: set protection domain so it matches bootstrap classes.
     *
     * The idea here is to create a class object and fill in the details
     * as we would in loadClassFromDex(), and then call dvmLinkClass() to do
     * all the heavy lifting (notably populating the virtual and interface
     * method tables).
     */

    /*
     * Generate a temporary list of virtual methods.
     */
    int methodCount = -1;
    if (!gatherMethods(interfaces, &methods, &throws, &methodCount))
        goto bail;

    /*
     * Allocate storage for the class object and set some basic fields.
     */
    newClass = (ClassObject*) dvmMalloc(sizeof(*newClass), ALLOC_DEFAULT);
    if (newClass == NULL)
        goto bail;
    DVM_OBJECT_INIT(&newClass->obj, gDvm.unlinkedJavaLangClass);
    dvmSetClassSerialNumber(newClass);
    newClass->descriptorAlloc = dvmNameToDescriptor(nameStr);
    newClass->descriptor = newClass->descriptorAlloc;
    newClass->accessFlags = ACC_PUBLIC | ACC_FINAL;
    newClass->super = gDvm.classJavaLangReflectProxy;
    newClass->primitiveType = PRIM_NOT;
    newClass->classLoader = loader;
#if WITH_HPROF && WITH_HPROF_STACK
    hprofFillInStackTrace(newClass);
#endif

    /*
     * Add direct method definitions.  We have one (the constructor).
     */
    newClass->directMethodCount = 1;
    newClass->directMethods = (Method*) dvmLinearAlloc(newClass->classLoader,
            1 * sizeof(Method));
    createConstructor(newClass, &newClass->directMethods[0]);
    dvmLinearReadOnly(newClass->classLoader, newClass->directMethods);

    /*
     * Add virtual method definitions.
     */
    newClass->virtualMethodCount = methodCount;
    newClass->virtualMethods = (Method*) dvmLinearAlloc(newClass->classLoader,
            newClass->virtualMethodCount * sizeof(Method));
    for (i = 0; i < newClass->virtualMethodCount; i++) {
        createHandlerMethod(newClass, &newClass->virtualMethods[i],methods[i]);
    }
    dvmLinearReadOnly(newClass->classLoader, newClass->virtualMethods);

    /*
     * Add interface list.
     */
    int interfaceCount = interfaces->length;
    ClassObject** ifArray = (ClassObject**) interfaces->contents;
    newClass->interfaceCount = interfaceCount;
    newClass->interfaces = (ClassObject**)dvmLinearAlloc(newClass->classLoader,
                                sizeof(ClassObject*) * interfaceCount);
    for (i = 0; i < interfaceCount; i++)
        newClass->interfaces[i] = ifArray[i];
    dvmLinearReadOnly(newClass->classLoader, newClass->interfaces);

    /*
     * Static field list.  We have one private field, for our list of
     * exceptions declared for each method.
     */
    newClass->sfieldCount = 1;
    newClass->sfields = (StaticField*) calloc(1, sizeof(StaticField));
    StaticField* sfield = &newClass->sfields[kThrowsField];
    sfield->field.clazz = newClass;
    sfield->field.name = "throws";
    sfield->field.signature = "[[Ljava/lang/Throwable;";
    sfield->field.accessFlags = ACC_STATIC | ACC_PRIVATE;
    dvmSetStaticFieldObject(sfield, (Object*)throws);

    /*
     * Everything is ready.  See if the linker will lap it up.
     */
    newClass->status = CLASS_LOADED;
    if (!dvmLinkClass(newClass, true)) {
        LOGD("Proxy class link failed\n");
        goto bail;
    }

    /*
     * All good.  Add it to the hash table.  We should NOT see a collision
     * here; if we do, it means the caller has screwed up and provided us
     * with a duplicate name.
     */
    if (!dvmAddClassToHash(newClass)) {
        LOGE("ERROR: attempted to generate %s more than once\n",
            newClass->descriptor);
        goto bail;
    }

    result = 0;

bail:
    free(nameStr);
    free(methods);
    if (result != 0) {
        /* must free innards explicitly if we didn't finish linking */
        dvmFreeClassInnards(newClass);
        newClass = NULL;
        if (!dvmCheckException(dvmThreadSelf())) {
            /* throw something */
            dvmThrowException("Ljava/lang/RuntimeException;", NULL);
        }
    }

    /* allow the GC to free these when nothing else has a reference */
    dvmReleaseTrackedAlloc((Object*) throws, NULL);
    dvmReleaseTrackedAlloc((Object*) newClass, NULL);

    return newClass;
}


/*
 * Generate a list of methods.  The Method pointers returned point to the
 * abstract method definition from the appropriate interface, or to the
 * virtual method definition in java.lang.Object.
 *
 * We also allocate an array of arrays of throwable classes, one for each
 * method,so we can do some special handling of checked exceptions.  The
 * caller must call ReleaseTrackedAlloc() on *pThrows.
 */
static bool gatherMethods(ArrayObject* interfaces, Method*** pMethods,
    ArrayObject** pThrows, int* pMethodCount)
{
    ClassObject** classes;
    ArrayObject* throws = NULL;
    Method** methods = NULL;
    Method** allMethods = NULL;
    int numInterfaces, maxCount, actualCount, allCount;
    bool result = false;
    int i;

    /*
     * Get a maximum count so we can allocate storage.  We need the
     * methods declared by each interface and all of its superinterfaces.
     */
    maxCount = 3;       // 3 methods in java.lang.Object
    numInterfaces = interfaces->length;
    classes = (ClassObject**) interfaces->contents;

    for (i = 0; i < numInterfaces; i++, classes++) {
        ClassObject* clazz = *classes;

        LOGVV("---  %s virtualMethodCount=%d\n",
            clazz->descriptor, clazz->virtualMethodCount);
        maxCount += clazz->virtualMethodCount;

        int j;
        for (j = 0; j < clazz->iftableCount; j++) {
            ClassObject* iclass = clazz->iftable[j].clazz;

            LOGVV("---  +%s %d\n",
                iclass->descriptor, iclass->virtualMethodCount);
            maxCount += iclass->virtualMethodCount;
        }
    }

    methods = (Method**) malloc(maxCount * sizeof(*methods));
    allMethods = (Method**) malloc(maxCount * sizeof(*methods));
    if (methods == NULL || allMethods == NULL)
        goto bail;

    /*
     * First three entries are the java.lang.Object methods.
     */
    ClassObject* obj = gDvm.classJavaLangObject;
    allMethods[0] = obj->vtable[gDvm.voffJavaLangObject_equals];
    allMethods[1] = obj->vtable[gDvm.voffJavaLangObject_hashCode];
    allMethods[2] = obj->vtable[gDvm.voffJavaLangObject_toString];
    allCount = 3;

    /*
     * Add the methods from each interface, in order.
     */
    classes = (ClassObject**) interfaces->contents;
    for (i = 0; i < numInterfaces; i++, classes++) {
        ClassObject* clazz = *classes;
        int j;

        for (j = 0; j < clazz->virtualMethodCount; j++) {
            allMethods[allCount++] = &clazz->virtualMethods[j];
        }

        for (j = 0; j < clazz->iftableCount; j++) {
            ClassObject* iclass = clazz->iftable[j].clazz;
            int k;

            for (k = 0; k < iclass->virtualMethodCount; k++) {
                allMethods[allCount++] = &iclass->virtualMethods[k];
            }
        }
    }
    assert(allCount == maxCount);

    /*
     * Allocate some storage to hold the lists of throwables.  We need
     * one entry per unique method, but it's convenient to allocate it
     * ahead of the duplicate processing.
     */
    ClassObject* arrArrClass;
    arrArrClass = dvmFindArrayClass("[[Ljava/lang/Throwable;", NULL);
    if (arrArrClass == NULL)
        goto bail;
    throws = dvmAllocArrayByClass(arrArrClass, allCount, ALLOC_DEFAULT);

    /*
     * Identify and remove duplicates.
     */
    actualCount = copyWithoutDuplicates(allMethods, allCount, methods, throws);
    if (actualCount < 0)
        goto bail;

    //LOGI("gathered methods:\n");
    //for (i = 0; i < actualCount; i++) {
    //    LOGI(" %d: %s.%s\n",
    //        i, methods[i]->clazz->descriptor, methods[i]->name);
    //}

    *pMethods = methods;
    *pMethodCount = actualCount;
    *pThrows = throws;
    result = true;

bail:
    free(allMethods);
    if (!result) {
        free(methods);
        dvmReleaseTrackedAlloc((Object*)throws, NULL);
    }
    return result;
}

/*
 * Identify and remove duplicates, where "duplicate" means it has the
 * same name and arguments, but not necessarily the same return type.
 *
 * If duplicate methods have different return types, we want to use the
 * first method whose return type is assignable from all other duplicate
 * methods.  That is, if we have:
 *   class base {...}
 *   class sub extends base {...}
 *   class subsub extends sub {...}
 * Then we want to return the method that returns subsub, since callers
 * to any form of the method will get a usable object back.
 *
 * All other duplicate methods are stripped out.
 *
 * This also populates the "throwLists" array with arrays of Class objects,
 * one entry per method in "outMethods".  Methods that don't declare any
 * throwables (or have no common throwables with duplicate methods) will
 * have NULL entries.
 *
 * Returns the number of methods copied into "methods", or -1 on failure.
 */
static int copyWithoutDuplicates(Method** allMethods, int allCount,
    Method** outMethods, ArrayObject* throwLists)
{
    Method* best;
    int outCount = 0;
    int i, j;

    /*
     * The plan is to run through all methods, checking all other methods
     * for a duplicate.  If we find a match, we see if the other methods'
     * return type is compatible/assignable with ours.  If the current
     * method is assignable from all others, we copy it to the new list,
     * and NULL out all other entries.  If not, we keep looking for a
     * better version.
     *
     * If there are no duplicates, we copy the method and NULL the entry.
     *
     * At the end of processing, if we have any non-NULL entries, then we
     * have bad duplicates and must exit with an exception.
     */
    for (i = 0; i < allCount; i++) {
        bool best, dupe;

        if (allMethods[i] == NULL)
            continue;

        /*
         * Find all duplicates.  If any of the return types is not
         * assignable to our return type, then we're not the best.
         *
         * We start from 0, not i, because we need to compare assignability
         * the other direction even if we've compared these before.
         */
        dupe = false;
        best = true;
        for (j = 0; j < allCount; j++) {
            if (i == j)
                continue;
            if (allMethods[j] == NULL)
                continue;

            if (dvmCompareMethodNamesAndParameterProtos(allMethods[i],
                    allMethods[j]) == 0)
            {
                /*
                 * Duplicate method, check return type.  If it's a primitive
                 * type or void, the types must match exactly, or we throw
                 * an exception now.
                 */
                LOGV("MATCH on %s.%s and %s.%s\n",
                    allMethods[i]->clazz->descriptor, allMethods[i]->name,
                    allMethods[j]->clazz->descriptor, allMethods[j]->name);
                dupe = true;
                if (!returnTypesAreCompatible(allMethods[i], allMethods[j]))
                    best = false;
            }
        }

        /*
         * If this is the best of a set of duplicates, copy it over and
         * nuke all duplicates.
         *
         * While we do this, we create the set of exceptions declared to
         * be thrown by all occurrences of the method.
         */
        if (dupe) {
            if (best) {
                LOGV("BEST %d %s.%s -> %d\n", i,
                    allMethods[i]->clazz->descriptor, allMethods[i]->name,
                    outCount);

                /* if we have exceptions, make a local copy */
                PointerSet* commonThrows = NULL;
                if (!createExceptionClassList(allMethods[i], &commonThrows))
                    return -1;

                /*
                 * Run through one more time, erasing the duplicates.  (This
                 * would go faster if we had marked them somehow.)
                 */
                for (j = 0; j < allCount; j++) {
                    if (i == j)
                        continue;
                    if (allMethods[j] == NULL)
                        continue;
                    if (dvmCompareMethodNamesAndParameterProtos(allMethods[i],
                            allMethods[j]) == 0)
                    {
                        LOGV("DEL %d %s.%s\n", j,
                            allMethods[j]->clazz->descriptor,
                            allMethods[j]->name);

                        /*
                         * Update set to hold the intersection of method[i]'s
                         * and method[j]'s throws.
                         */
                        if (commonThrows != NULL) {
                            updateExceptionClassList(allMethods[j],
                                commonThrows);
                        }

                        allMethods[j] = NULL;
                    }
                }

                /*
                 * If the set of Throwable classes isn't empty, create an
                 * array of Class, copy them into it, and put the result
                 * into the "throwLists" array.
                 */
                if (commonThrows != NULL &&
                    dvmPointerSetGetCount(commonThrows) > 0)
                {
                    int commonCount = dvmPointerSetGetCount(commonThrows);
                    ArrayObject* throwArray;
                    Object** contents;
                    int ent;

                    throwArray = dvmAllocArrayByClass(
                            gDvm.classJavaLangClassArray, commonCount,
                            ALLOC_DEFAULT);
                    if (throwArray == NULL) {
                        LOGE("common-throw array alloc failed\n");
                        return -1;
                    }

                    contents = (Object**) throwArray->contents;
                    for (ent = 0; ent < commonCount; ent++) {
                        contents[ent] = (Object*)
                            dvmPointerSetGetEntry(commonThrows, ent);
                    }

                    /* add it to the array of arrays */
                    contents = (Object**) throwLists->contents;
                    contents[outCount] = (Object*) throwArray;
                    dvmReleaseTrackedAlloc((Object*) throwArray, NULL);
                }

                /* copy the winner and NULL it out */
                outMethods[outCount++] = allMethods[i];
                allMethods[i] = NULL;

                dvmPointerSetFree(commonThrows);
            } else {
                LOGV("BEST not %d\n", i);
            }
        } else {
            /*
             * Singleton.  Copy the entry and NULL it out.
             */
            LOGV("COPY singleton %d %s.%s -> %d\n", i,
                allMethods[i]->clazz->descriptor, allMethods[i]->name,
                outCount);

            /* keep track of our throwables */
            ArrayObject* exceptionArray = dvmGetMethodThrows(allMethods[i]);
            if (exceptionArray != NULL) {
                Object** contents;

                contents = (Object**) throwLists->contents;
                contents[outCount] = (Object*) exceptionArray;
                dvmReleaseTrackedAlloc((Object*) exceptionArray, NULL);
            }

            outMethods[outCount++] = allMethods[i];
            allMethods[i] = NULL;
        }
    }

    /*
     * Check for stragglers.  If we find any, throw an exception.
     */
    for (i = 0; i < allCount; i++) {
        if (allMethods[i] != NULL) {
            LOGV("BAD DUPE: %d %s.%s\n", i,
                allMethods[i]->clazz->descriptor, allMethods[i]->name);
            dvmThrowException("Ljava/lang/IllegalArgumentException;",
                "incompatible return types in proxied interfaces");
            return -1;
        }
    }

    return outCount;
}


/*
 * Classes can declare to throw multiple exceptions in a hierarchy, e.g.
 * IOException and FileNotFoundException.  Since we're only interested in
 * knowing the set that can be thrown without requiring an extra wrapper,
 * we can remove anything that is a subclass of something else in the list.
 *
 * The "mix" step we do next reduces things toward the most-derived class,
 * so it's important that we start with the least-derived classes.
 */
static void reduceExceptionClassList(ArrayObject* exceptionArray)
{
    const ClassObject** classes = (const ClassObject**)exceptionArray->contents;
    int len = exceptionArray->length;
    int i, j;

    /*
     * Consider all pairs of classes.  If one is the subclass of the other,
     * null out the subclass.
     */
    for (i = 0; i < len-1; i++) {
        if (classes[i] == NULL)
            continue;
        for (j = i + 1; j < len; j++) {
            if (classes[j] == NULL)
                continue;

            if (dvmInstanceof(classes[i], classes[j])) {
                classes[i] = NULL;
                break;      /* no more comparisons against classes[i] */
            } else if (dvmInstanceof(classes[j], classes[i])) {
                classes[j] = NULL;
            }
        }
    }
}

/*
 * Create a local array with a copy of the throwable classes declared by
 * "method".  If no throws are declared, "*pSet" will be NULL.
 *
 * Returns "false" on allocation failure.
 */
static bool createExceptionClassList(const Method* method, PointerSet** pThrows)
{
    ArrayObject* exceptionArray = NULL;
    bool result = false;

    exceptionArray = dvmGetMethodThrows(method);
    if (exceptionArray != NULL && exceptionArray->length > 0) {
        /* reduce list, nulling out redundant entries */
        reduceExceptionClassList(exceptionArray);

        *pThrows = dvmPointerSetAlloc(exceptionArray->length);
        if (*pThrows == NULL)
            goto bail;

        const ClassObject** contents;
        int i;

        contents = (const ClassObject**) exceptionArray->contents;
        for (i = 0; i < (int) exceptionArray->length; i++) {
            if (contents[i] != NULL)
                dvmPointerSetAddEntry(*pThrows, contents[i]);
        }
    } else {
        *pThrows = NULL;
    }

    result = true;

bail:
    dvmReleaseTrackedAlloc((Object*) exceptionArray, NULL);
    return result;
}

/*
 * We need to compute the intersection of the arguments, i.e. remove
 * anything from "throws" that isn't in the method's list of throws.
 *
 * If one class is a subclass of another, we want to keep just the subclass,
 * moving toward the most-restrictive set.
 *
 * We assume these are all classes, and don't try to filter out interfaces.
 */
static void updateExceptionClassList(const Method* method, PointerSet* throws)
{
    int setSize = dvmPointerSetGetCount(throws);
    if (setSize == 0)
        return;

    ArrayObject* exceptionArray = dvmGetMethodThrows(method);
    if (exceptionArray == NULL) {
        /* nothing declared, so intersection is empty */
        dvmPointerSetClear(throws);
        return;
    }

    /* reduce list, nulling out redundant entries */
    reduceExceptionClassList(exceptionArray);

    int mixLen = dvmPointerSetGetCount(throws);
    const ClassObject* mixSet[mixLen];

    int declLen = exceptionArray->length;
    const ClassObject** declSet = (const ClassObject**)exceptionArray->contents;

    int i, j;

    /* grab a local copy to work on */
    for (i = 0; i < mixLen; i++) {
        mixSet[i] = dvmPointerSetGetEntry(throws, i);
    }

    for (i = 0; i < mixLen; i++) {
        for (j = 0; j < declLen; j++) {
            if (declSet[j] == NULL)
                continue;

            if (mixSet[i] == declSet[j]) {
                /* match, keep this one */
                break;
            } else if (dvmInstanceof(mixSet[i], declSet[j])) {
                /* mix is a subclass of a declared throwable, keep it */
                break;
            } else if (dvmInstanceof(declSet[j], mixSet[i])) {
                /* mix is a superclass, replace it */
                mixSet[i] = declSet[j];
                break;
            }
        }

        if (j == declLen) {
            /* no match, remove entry by nulling it out */
            mixSet[i] = NULL;
        }
    }

    /* copy results back out; this eliminates duplicates as we go */
    dvmPointerSetClear(throws);
    for (i = 0; i < mixLen; i++) {
        if (mixSet[i] != NULL)
            dvmPointerSetAddEntry(throws, mixSet[i]);
    }

    dvmReleaseTrackedAlloc((Object*) exceptionArray, NULL);
}


/*
 * Check to see if the return types are compatible.
 *
 * If the return type is primitive or void, it must match exactly.
 *
 * If not, the type in "subMethod" must be assignable to the type in
 * "baseMethod".
 */
static bool returnTypesAreCompatible(Method* subMethod, Method* baseMethod)
{
    const char* baseSig = dexProtoGetReturnType(&baseMethod->prototype);
    const char* subSig = dexProtoGetReturnType(&subMethod->prototype);
    ClassObject* baseClass;
    ClassObject* subClass;

    if (baseSig[1] == '\0' || subSig[1] == '\0') {
        /* at least one is primitive type */
        return (baseSig[0] == subSig[0] && baseSig[1] == subSig[1]);
    }

    baseClass = dvmFindClass(baseSig, baseMethod->clazz->classLoader);
    subClass = dvmFindClass(subSig, subMethod->clazz->classLoader);
    bool result = dvmInstanceof(subClass, baseClass);
    return result;
}

/*
 * Create a constructor for our Proxy class.  The constructor takes one
 * argument, a java.lang.reflect.InvocationHandler.
 */
static void createConstructor(ClassObject* clazz, Method* meth)
{
    meth->clazz = clazz;
    meth->accessFlags = ACC_PUBLIC | ACC_NATIVE;
    meth->name = "<init>";
    meth->prototype =
        gDvm.methJavaLangReflectProxy_constructorPrototype->prototype;
    meth->shorty = 
        gDvm.methJavaLangReflectProxy_constructorPrototype->shorty;
    // no pDexCode or pDexMethod

    int argsSize = dvmComputeMethodArgsSize(meth) + 1;
    meth->registersSize = meth->insSize = argsSize;

    meth->nativeFunc = proxyConstructor;
}

/*
 * Create a method in our Proxy class with the name and signature of
 * the interface method it implements.
 */
static void createHandlerMethod(ClassObject* clazz, Method* dstMeth,
    const Method* srcMeth)
{
    dstMeth->clazz = clazz;
    dstMeth->insns = (u2*) srcMeth;
    dstMeth->accessFlags = ACC_PUBLIC | ACC_NATIVE;
    dstMeth->name = srcMeth->name;
    dstMeth->prototype = srcMeth->prototype;
    dstMeth->shorty = srcMeth->shorty;
    // no pDexCode or pDexMethod

    int argsSize = dvmComputeMethodArgsSize(dstMeth) + 1;
    dstMeth->registersSize = dstMeth->insSize = argsSize;

    dstMeth->nativeFunc = proxyInvoker;
}

/*
 * Return a new Object[] array with the contents of "args".  We determine
 * the number and types of values in "args" based on the method signature.
 * Primitive types are boxed.
 *
 * Returns NULL if the method takes no arguments.
 *
 * The caller must call dvmReleaseTrackedAlloc() on the return value.
 *
 * On failure, returns with an appropriate exception raised.
 */
static ArrayObject* boxMethodArgs(const Method* method, const u4* args)
{
    const char* desc = &method->shorty[1]; // [0] is the return type.
    ArrayObject* argArray = NULL;
    int argCount;
    Object** argObjects;
    bool failed = true;

    /* count args */
    argCount = dexProtoGetParameterCount(&method->prototype);

    /* allocate storage */
    argArray = dvmAllocArray(gDvm.classJavaLangObjectArray, argCount,
        kObjectArrayRefWidth, ALLOC_DEFAULT);
    if (argArray == NULL)
        goto bail;
    argObjects = (Object**) argArray->contents;

    /*
     * Fill in the array.
     */

    int srcIndex = 0;
    
    argCount = 0;
    while (*desc != '\0') {
        char descChar = *(desc++);
        JValue value;

        switch (descChar) {
        case 'Z':
        case 'C':
        case 'F':
        case 'B':
        case 'S':
        case 'I':
            value.i = args[srcIndex++];
            argObjects[argCount] = (Object*) dvmWrapPrimitive(value,
                dvmFindPrimitiveClass(descChar));
            /* argObjects is tracked, don't need to hold this too */
            dvmReleaseTrackedAlloc(argObjects[argCount], NULL);
            argCount++;
            break;
        case 'D':
        case 'J':
            value.j = dvmGetArgLong(args, srcIndex);
            srcIndex += 2;
            argObjects[argCount] = (Object*) dvmWrapPrimitive(value,
                dvmFindPrimitiveClass(descChar));
            dvmReleaseTrackedAlloc(argObjects[argCount], NULL);
            argCount++;
            break;
        case '[':
        case 'L':
            argObjects[argCount++] = (Object*) args[srcIndex++];
            break;
        }
    }

    failed = false;

bail:
    if (failed) {
        dvmReleaseTrackedAlloc((Object*)argArray, NULL);
        argArray = NULL;
    }
    return argArray;
}

/*
 * This is the constructor for a generated proxy object.  All we need to
 * do is stuff "handler" into "h".
 */
static void proxyConstructor(const u8* args, JValue* pResult,
    const Method* method, Thread* self)
{
    Object* obj = (Object*) args[0];
    Object* handler = (Object*) args[1];

    dvmSetFieldObject(obj, gDvm.offJavaLangReflectProxy_h, handler);
}

/*
 * This is the common message body for proxy methods.
 *
 * The method we're calling looks like:
 *   public Object invoke(Object proxy, Method method, Object[] args)
 *
 * This means we have to create a Method object, box our arguments into
 * a new Object[] array, make the call, and unbox the return value if
 * necessary.
 */
static void proxyInvoker(const u8* args, JValue* pResult,
    const Method* method, Thread* self)
{
    Object* thisObj = (Object*) args[0];
    Object* methodObj = NULL;
    ArrayObject* argArray = NULL;
    Object* handler;
    Method* invoke;
    ClassObject* returnType;
    int hOffset;
    JValue invokeResult;

    /*
     * Retrieve handler object for this proxy instance.  The field is
     * defined in the superclass (Proxy).
     */
    handler = dvmGetFieldObject(thisObj, gDvm.offJavaLangReflectProxy_h);

    /*
     * Find the invoke() method, looking in "this"s class.  (Because we
     * start here we don't have to convert it to a vtable index and then
     * index into this' vtable.)
     */
    invoke = dvmFindVirtualMethodHierByDescriptor(handler->clazz, "invoke",
            "(Ljava/lang/Object;Ljava/lang/reflect/Method;[Ljava/lang/Object;)Ljava/lang/Object;");
    if (invoke == NULL) {
        LOGE("Unable to find invoke()\n");
        dvmAbort();
    }

    LOGV("invoke: %s.%s, this=%p, handler=%s\n",
        method->clazz->descriptor, method->name,
        thisObj, handler->clazz->descriptor);

    /*
     * Create a java.lang.reflect.Method object for this method.
     *
     * We don't want to use "method", because that's the concrete
     * implementation in the proxy class.  We want the abstract Method
     * from the declaring interface.  We have a pointer to it tucked
     * away in the "insns" field.
     *
     * TODO: this could be cached for performance.
     */
    methodObj = dvmCreateReflectMethodObject((Method*) method->insns);
    if (methodObj == NULL) {
        assert(dvmCheckException(self));
        goto bail;
    }

    /*
     * Determine the return type from the signature.
     *
     * TODO: this could be cached for performance.
     */
    returnType = dvmGetBoxedReturnType(method);
    if (returnType == NULL) {
        char* desc = dexProtoCopyMethodDescriptor(&method->prototype);
        LOGE("Could not determine return type for '%s'\n", desc);
        free(desc);
        assert(dvmCheckException(self));
        goto bail;
    }
    LOGV("  return type will be %s\n", returnType->descriptor);

    /*
     * Convert "args" array into Object[] array, using the method
     * signature to determine types.  If the method takes no arguments,
     * we must pass null.
     */
    argArray = boxMethodArgs(method, args+1);
    if (dvmCheckException(self))
        goto bail;

    /*
     * Call h.invoke(proxy, method, args).
     *
     * We don't need to repackage exceptions, so if one has been thrown
     * just jump to the end.
     */
    dvmCallMethod(self, invoke, handler, &invokeResult,
        thisObj, methodObj, argArray);
    if (dvmCheckException(self)) {
        Object* excep = dvmGetException(self);
        if (mustWrapException(method, excep)) {
            /* wrap with UndeclaredThrowableException */
            dvmWrapException("Ljava/lang/reflect/UndeclaredThrowableException;");
        }
        goto bail;
    }

    /*
     * Unbox the return value.  If it's the wrong type, throw a
     * ClassCastException.  If it's a null pointer and we need a
     * primitive type, throw a NullPointerException.
     */
    if (returnType->primitiveType == PRIM_VOID) {
        LOGVV("+++ ignoring return to void\n");
    } else if (invokeResult.l == NULL) {
        if (dvmIsPrimitiveClass(returnType)) {
            dvmThrowException("Ljava/lang/NullPointerException;",
                "null result when primitive expected");
            goto bail;
        }
        pResult->l = NULL;
    } else {
        if (!dvmUnwrapPrimitive(invokeResult.l, returnType, pResult)) {
            dvmThrowExceptionWithClassMessage("Ljava/lang/ClassCastException;",
                ((Object*)invokeResult.l)->clazz->descriptor);
            goto bail;
        }
    }

bail:
    dvmReleaseTrackedAlloc(methodObj, self);
    dvmReleaseTrackedAlloc((Object*)argArray, self);
}

/*
 * Determine if it's okay for this method to throw this exception.  If
 * an unchecked exception was thrown we immediately return false.  If
 * checked, we have to ensure that this method and all of its duplicates
 * have declared that they throw it.
 */
static bool mustWrapException(const Method* method, const Object* throwable)
{
    const ArrayObject* throws;
    const ArrayObject* methodThrows;
    const Object** contents;
    const ClassObject** classes;

    if (!dvmIsCheckedException(throwable))
        return false;

    const StaticField* sfield = &method->clazz->sfields[kThrowsField];
    throws = (ArrayObject*) dvmGetStaticFieldObject(sfield);

    int methodIndex = method - method->clazz->virtualMethods;
    assert(methodIndex >= 0 && methodIndex < method->clazz->virtualMethodCount);

    contents = (const Object**) throws->contents;
    methodThrows = (ArrayObject*) contents[methodIndex];

    if (methodThrows == NULL) {
        /* no throws declared, must wrap all checked exceptions */
        //printf("+++ methodThrows[%d] is null, wrapping all\n", methodIndex);
        return true;
    }

    int throwCount = methodThrows->length;
    classes = (const ClassObject**) methodThrows->contents;
    int i;

    //printf("%s.%s list:\n", method->clazz->descriptor, method->name);
    //for (i = 0; i < throwCount; i++)
    //    printf(" %d: %s\n", i, classes[i]->descriptor);

    for (i = 0; i < throwCount; i++) {
        if (dvmInstanceof(throwable->clazz, classes[i])) {
            /* this was declared, okay to throw */
            return false;
        }
    }

    /* no match in declared throws */
    return true;
}

