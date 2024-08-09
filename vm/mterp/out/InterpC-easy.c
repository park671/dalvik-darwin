//
// Created by Park Yu on 2024/8/5.
//
#include "Dalvik.h"
#include "interp/InterpDefs.h"
#include "mterp/Mterp.h"
#include <math.h>                   // needed for fmod, fmodf

static inline s8 getLongFromArray(const u_int64_t *ptr, int idx) {
    return *((s8 *) &ptr[idx]);
}

static inline void putLongToArray(u_int64_t *ptr, int idx, s8 val) {
    *((s8 *) &ptr[idx]) = val;
}

static inline double getDoubleFromArray(const u_int64_t *ptr, int idx) {
    return *((double *) &ptr[idx]);
}

static inline void putDoubleToArray(u_int64_t *ptr, int idx, double dval) {
    *((double *) &ptr[idx]) = dval;
}

INLINE Method *dvmFindInterfaceMethodInCache(ClassObject *thisClass,
                                             u_int32_t methodIdx, const Method *method, DvmDex *methodClassDex) {
    return (Method *) ({
        AtomicCacheEntry *pEntry;
        u_int64_t hash;
        u_int64_t firstVersion;
        u_int64_t value;
        hash = (((u_int64_t) (thisClass) >> 2) ^ (u_int64_t) (methodIdx)) & ((128) - 1);
        pEntry = (methodClassDex->pInterfaceCache)->entries + hash;
        firstVersion = pEntry->version;
        if (pEntry->key1 == (u_int64_t) (thisClass) && pEntry->key2 == (u_int64_t) (methodIdx)) {
            value = pEntry->value;
            if ((firstVersion & 0x01) != 0 || firstVersion != pEntry->version) {
                if (0)(methodClassDex->pInterfaceCache)->fail++;
                value = (u_int64_t) dvmInterpFindInterfaceMethod(thisClass, methodIdx, method, methodClassDex);
            } else { if (0)(methodClassDex->pInterfaceCache)->hits++; }
        } else {
            value = (u_int64_t) dvmInterpFindInterfaceMethod(thisClass, methodIdx, method, methodClassDex);
            dvmUpdateAtomicCache((u_int64_t) (thisClass), (u_int64_t) (methodIdx), value, pEntry, firstVersion);
        }
        value;
    });
}

static inline bool checkForNull(Object *obj) {
    if (obj == NULL) {
        dvmThrowException("Ljava/lang/NullPointerException;", NULL);
        return false;
    }
    if (obj->clazz == NULL || ((u_int64_t) obj->clazz) <= 65536) {
        LOGE("Invalid object class %p (in %p)\n", obj->clazz, obj);
        dvmAbort();
    }
    return true;
}

static inline bool checkForNullExportPC(Object *obj, u_int64_t *fp, const u_int16_t *pc) {
    if (obj == NULL) {
        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
        dvmThrowException("Ljava/lang/NullPointerException;", NULL);
        return false;
    }
    if (obj->clazz == NULL || ((u_int64_t) obj->clazz) <= 65536) {
        LOGE("Invalid object class %p (in %p)\n", obj->clazz, obj);
        dvmAbort();
    }
    return true;
}

bool dvmInterpretStd(Thread *self, InterpState *interpState) {
    DvmDex *methodClassDex;
    JValue retval;
    const Method *curMethod;
    const u_int16_t *pc;
    u8 *fp;
    u_int16_t inst;
    u_int16_t ref;
    u_int16_t vsrc1, vsrc2, vdst;
    const Method *methodToCall;
    bool methodCallRange;
    curMethod = interpState->method;
    pc = interpState->pc;
    fp = interpState->fp;
    retval = interpState->retval;
    methodClassDex = curMethod->clazz->pDvmDex;
            LOGVV("dvmInterpretStd: threadid=%d: entry(%s) %s.%s pc=0x%lx fp=%p ep=%d\n",
                  self->threadId, (interpState->nextMode == INTERP_STD) ? "STD" : "DBG",
                  curMethod->clazz->descriptor, curMethod->name, pc - curMethod->insns,
                  fp, interpState->entryPoint);
    methodToCall = (const Method *) -1;
    switch (interpState->entryPoint) {
        case kInterpEntryInstr:
            break;
        case kInterpEntryReturn:
            goto returnFromMethod;
        case kInterpEntryThrow:
            goto exceptionThrown;
        default:
            dvmAbort();
    }
    while (1) {
        ((void) 0);
        ((void) 0);
        inst = (pc[(0)]);
        switch (((inst) & 0xff)) {
            case OP_NOP: {
                (pc += 1);
                break;
            };
            case OP_MOVE:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (fp[(vdst)] = ((fp[(vsrc1)])));
                {
                    (pc += 1);
                    break;
                };
            case OP_MOVE_FROM16:
                vdst = ((inst) >> 8);
                vsrc1 = (pc[(1)]);
                ((void) 0);
                (fp[(vdst)] = ((fp[(vsrc1)])));
                {
                    (pc += 2);
                    break;
                };
            case OP_MOVE_16:
                vdst = (pc[(1)]);
                vsrc1 = (pc[(2)]);
                ((void) 0);
                (fp[(vdst)] = ((fp[(vsrc1)])));
                {
                    (pc += 3);
                    break;
                };
            case OP_MOVE_WIDE:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putLongToArray(fp, (vdst), (getLongFromArray(fp, (vsrc1))));
                {
                    (pc += 1);
                    break;
                };
            case OP_MOVE_WIDE_FROM16:
                vdst = ((inst) >> 8);
                vsrc1 = (pc[(1)]);
                ((void) 0);
                putLongToArray(fp, (vdst), (getLongFromArray(fp, (vsrc1))));
                {
                    (pc += 2);
                    break;
                };
            case OP_MOVE_WIDE_16:
                vdst = (pc[(1)]);
                vsrc1 = (pc[(2)]);
                ((void) 0);
                putLongToArray(fp, (vdst), (getLongFromArray(fp, (vsrc1))));
                {
                    (pc += 3);
                    break;
                };
            case OP_MOVE_OBJECT:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (fp[(vdst)] = ((fp[(vsrc1)])));
                {
                    (pc += 1);
                    break;
                };
            case OP_MOVE_OBJECT_FROM16:
                vdst = ((inst) >> 8);
                vsrc1 = (pc[(1)]);
                ((void) 0);
                (fp[(vdst)] = ((fp[(vsrc1)])));
                {
                    (pc += 2);
                    break;
                };
            case OP_MOVE_OBJECT_16:
                vdst = (pc[(1)]);
                vsrc1 = (pc[(2)]);
                ((void) 0);
                (fp[(vdst)] = ((fp[(vsrc1)])));
                {
                    (pc += 3);
                    break;
                };
            case OP_MOVE_RESULT:
                vdst = ((inst) >> 8);
                ((void) 0);
                (fp[(vdst)] = (retval.j));
                {
                    (pc += 1);
                    break;
                };
            case OP_MOVE_RESULT_WIDE:
                vdst = ((inst) >> 8);
                ((void) 0);
                putLongToArray(fp, (vdst), (retval.j));
                {
                    (pc += 1);
                    break;
                };
            case OP_MOVE_RESULT_OBJECT:
                vdst = ((inst) >> 8);
                ((void) 0);
                (fp[(vdst)] = (retval.j));
                {
                    (pc += 1);
                    break;
                };
            case OP_MOVE_EXCEPTION:
                vdst = ((inst) >> 8);
                ((void) 0);
                assert(self->exception != NULL);
                (fp[(vdst)] = ((u8) self->exception));
                dvmClearException(self);
                {
                    (pc += 1);
                    break;
                };
            case OP_RETURN_VOID:
                ((void) 0);
                retval.j = 0xababababULL;
                goto returnFromMethod;;
            case OP_RETURN:
                vsrc1 = ((inst) >> 8);
                ((void) 0);
                retval.j = (fp[(vsrc1)]);
                goto returnFromMethod;;
            case OP_RETURN_WIDE:
                vsrc1 = ((inst) >> 8);
                ((void) 0);
                retval.j = getLongFromArray(fp, (vsrc1));
                goto returnFromMethod;;
            case OP_RETURN_OBJECT:
                vsrc1 = ((inst) >> 8);
                ((void) 0);
                retval.j = (fp[(vsrc1)]);
                goto returnFromMethod;;
            case OP_CONST_4: {
                int32_t tmp;
                vdst = (((inst) >> 8) & 0x0f);
                tmp = (int32_t) (((inst) >> 12) << 28) >> 28;
                ((void) 0);
                (fp[(vdst)] = (tmp));
            }
                {
                    (pc += 1);
                    break;
                };
            case OP_CONST_16:
                vdst = ((inst) >> 8);
                vsrc1 = (pc[(1)]);
                ((void) 0);
                (fp[(vdst)] = ((int16_t) vsrc1));
                {
                    (pc += 2);
                    break;
                };
            case OP_CONST: {
                u_int32_t tmp;
                vdst = ((inst) >> 8);
                tmp = (pc[(1)]);
                tmp |= (u_int32_t) (pc[(2)]) << 16;
                ((void) 0);
                (fp[(vdst)] = (tmp));
            }
                {
                    (pc += 3);
                    break;
                };
            case OP_CONST_HIGH16:
                vdst = ((inst) >> 8);
                vsrc1 = (pc[(1)]);
                ((void) 0);
                (fp[(vdst)] = (vsrc1 << 16));
                {
                    (pc += 2);
                    break;
                };
            case OP_CONST_WIDE_16:
                vdst = ((inst) >> 8);
                vsrc1 = (pc[(1)]);
                ((void) 0);
                putLongToArray(fp, (vdst), ((int16_t) vsrc1));
                {
                    (pc += 2);
                    break;
                };
            case OP_CONST_WIDE_32: {
                u_int32_t tmp;
                vdst = ((inst) >> 8);
                tmp = (pc[(1)]);
                tmp |= (u_int32_t) (pc[(2)]) << 16;
                ((void) 0);
                putLongToArray(fp, (vdst), ((int32_t) tmp));
            }
                {
                    (pc += 3);
                    break;
                };
            case OP_CONST_WIDE: {
                u_int64_t tmp;
                vdst = ((inst) >> 8);
                tmp = (pc[(1)]);
                tmp |= (u_int64_t) (pc[(2)]) << 16;
                tmp |= (u_int64_t) (pc[(3)]) << 32;
                tmp |= (u_int64_t) (pc[(4)]) << 48;
                ((void) 0);
                putLongToArray(fp, (vdst), (tmp));
            }
                {
                    (pc += 5);
                    break;
                };
            case OP_CONST_WIDE_HIGH16:
                vdst = ((inst) >> 8);
                vsrc1 = (pc[(1)]);
                ((void) 0);
                putLongToArray(fp, (vdst), (((u_int64_t) vsrc1) << 48));
                {
                    (pc += 2);
                    break;
                };
            case OP_CONST_STRING: {
                StringObject *strObj;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                strObj = dvmDexGetResolvedString(methodClassDex, ref);
                if (strObj == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    strObj = dvmResolveString(curMethod->clazz, ref);
                    if (strObj == NULL)
                        goto exceptionThrown;;
                }
                (fp[(vdst)] = ((u_int64_t) strObj));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_CONST_STRING_JUMBO: {
                StringObject *strObj;
                u_int32_t tmp;
                vdst = ((inst) >> 8);
                tmp = (pc[(1)]);
                tmp |= (u_int32_t) (pc[(2)]) << 16;
                ((void) 0);
                strObj = dvmDexGetResolvedString(methodClassDex, tmp);
                if (strObj == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    strObj = dvmResolveString(curMethod->clazz, tmp);
                    if (strObj == NULL)
                        goto exceptionThrown;;
                }
                (fp[(vdst)] = ((u_int64_t) strObj));
            }
                {
                    (pc += 3);
                    break;
                };
            case OP_CONST_CLASS: {
                ClassObject *clazz;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                clazz = dvmDexGetResolvedClass(methodClassDex, ref);
                if (clazz == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    clazz = dvmResolveClass(curMethod->clazz, ref, true);
                    if (clazz == NULL)
                        goto exceptionThrown;;
                }
                (fp[(vdst)] = ((u_int64_t) clazz));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_MONITOR_ENTER: {
                Object *obj;
                vsrc1 = ((inst) >> 8);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNullExportPC(obj, fp, pc))
                    goto exceptionThrown;;
                ((void) 0);
                dvmLockObject(self, obj);
            }
                {
                    (pc += 1);
                    break;
                };
            case OP_MONITOR_EXIT: {
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vsrc1 = ((inst) >> 8);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) {
                    (pc += 1);
                    goto exceptionThrown;;
                }
                ((void) 0);
                if (!dvmUnlockObject(self, obj)) {
                    assert(dvmCheckException(self));
                    (pc += 1);
                    goto exceptionThrown;;
                }
            }
                {
                    (pc += 1);
                    break;
                };
            case OP_CHECK_CAST: {
                ClassObject *clazz;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vsrc1 = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (obj != NULL) {
                    clazz = dvmDexGetResolvedClass(methodClassDex, ref);
                    if (clazz == NULL) {
                        clazz = dvmResolveClass(curMethod->clazz, ref, false);
                        if (clazz == NULL)
                            goto exceptionThrown;;
                    }
                    if (!dvmInstanceof(obj->clazz, clazz)) {
                        dvmThrowExceptionWithClassMessage(
                                "Ljava/lang/ClassCastException;", obj->clazz->descriptor);
                        goto exceptionThrown;;
                    }
                }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_INSTANCE_OF: {
                ClassObject *clazz;
                Object *obj;
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (obj == NULL) {
                    (fp[(vdst)] = (0));
                } else {
                    clazz = dvmDexGetResolvedClass(methodClassDex, ref);
                    if (clazz == NULL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        clazz = dvmResolveClass(curMethod->clazz, ref, true);
                        if (clazz == NULL)
                            goto exceptionThrown;;
                    }
                    (fp[(vdst)] = (dvmInstanceof(obj->clazz, clazz)));
                }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_ARRAY_LENGTH: {
                ArrayObject *arrayObj;
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                ((void) 0);
                if (!checkForNullExportPC((Object *) arrayObj, fp, pc))
                    goto exceptionThrown;;
                (fp[(vdst)] = (arrayObj->length));
            }
                {
                    (pc += 1);
                    break;
                };
            case OP_NEW_INSTANCE: {
                ClassObject *clazz;
                Object *newObj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                clazz = dvmDexGetResolvedClass(methodClassDex, ref);
                if (clazz == NULL) {
                    clazz = dvmResolveClass(curMethod->clazz, ref, false);
                    if (clazz == NULL)
                        goto exceptionThrown;;
                }
                if (!dvmIsClassInitialized(clazz) && !dvmInitClass(clazz))
                    goto exceptionThrown;;
                if (dvmIsInterfaceClass(clazz) || dvmIsAbstractClass(clazz)) {
                    dvmThrowExceptionWithClassMessage("Ljava/lang/InstantiationError;",
                                                      clazz->descriptor);
                    goto exceptionThrown;;
                }
                newObj = dvmAllocObject(clazz, ALLOC_DONT_TRACK);
                if (newObj == NULL)
                    goto exceptionThrown;;
                (fp[(vdst)] = ((u8) newObj));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_NEW_ARRAY: {
                ClassObject *arrayClass;
                ArrayObject *newArray;
                int32_t length;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                length = (int32_t) (fp[(vsrc1)]);
                if (length < 0) {
                    dvmThrowException("Ljava/lang/NegativeArraySizeException;", NULL);
                    goto exceptionThrown;;
                }
                arrayClass = dvmDexGetResolvedClass(methodClassDex, ref);
                if (arrayClass == NULL) {
                    arrayClass = dvmResolveClass(curMethod->clazz, ref, false);
                    if (arrayClass == NULL)
                        goto exceptionThrown;;
                }
                assert(dvmIsArrayClass(arrayClass));
                assert(dvmIsClassInitialized(arrayClass));
                newArray = dvmAllocArrayByClass(arrayClass, length, ALLOC_DONT_TRACK);
                if (newArray == NULL)
                    goto exceptionThrown;;
                (fp[(vdst)] = ((u8) newArray));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_FILLED_NEW_ARRAY:
                do {
                    methodCallRange = false;
                    goto filledNewArray;
                } while (false);
            case OP_FILLED_NEW_ARRAY_RANGE:
                do {
                    methodCallRange = true;
                    goto filledNewArray;
                } while (false);
            case OP_FILL_ARRAY_DATA: {
                const u_int16_t *arrayData;
                int32_t offset;
                ArrayObject *arrayObj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vsrc1 = ((inst) >> 8);
                offset = (pc[(1)]) | (((int32_t) (pc[(2)])) << 16);
                ((void) 0);
                arrayData = pc + offset;
                if (arrayData < curMethod->insns ||
                    arrayData >= curMethod->insns + dvmGetMethodInsnsSize(curMethod)) {
                    dvmThrowException("Ljava/lang/InternalError;",
                                      "bad fill array data");
                    goto exceptionThrown;;
                }
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!dvmInterpHandleFillArrayData(arrayObj, arrayData)) {
                    goto exceptionThrown;;
                }
                {
                    (pc += 3);
                    break;
                };
            }
            case OP_THROW: {
                Object *obj;
                vsrc1 = ((inst) >> 8);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNullExportPC(obj, fp, pc)) {
                            LOGVV("Bad exception\n");
                } else {
                    dvmSetException(self, obj);
                }
                goto exceptionThrown;;
            }
            case OP_GOTO:
                vdst = ((inst) >> 8);
                if ((s1) vdst < 0)
                    ((void) 0);
                else
                    ((void) 0);
                ((void) 0);
                if ((s1) vdst < 0) {
                    dvmCheckSuspendQuick(self);
                    if ((false)) {
                        (pc += (s1) vdst);
                        interpState->entryPoint = kInterpEntryInstr;
                                LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                      (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                      ((s1) vdst));
                        goto bail_switch;;
                    }
                };
                {
                    (pc += (s1) vdst);
                    break;
                };
            case OP_GOTO_16: {
                int32_t offset = (int16_t) (pc[(1)]);
                if (offset < 0)
                    ((void) 0);
                else
                    ((void) 0);
                ((void) 0);
                if (offset < 0) {
                    dvmCheckSuspendQuick(self);
                    if ((false)) {
                        (pc += offset);
                        interpState->entryPoint = kInterpEntryInstr;
                                LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                      (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                      (offset));
                        goto bail_switch;;
                    }
                };
                {
                    (pc += offset);
                    break;
                };
            }
            case OP_GOTO_32: {
                int32_t offset = (pc[(1)]);
                offset |= ((int32_t) (pc[(2)])) << 16;
                if (offset < 0)
                    ((void) 0);
                else
                    ((void) 0);
                ((void) 0);
                if (offset <= 0) {
                    dvmCheckSuspendQuick(self);
                    if ((false)) {
                        (pc += offset);
                        interpState->entryPoint = kInterpEntryInstr;
                                LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                      (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                      (offset));
                        goto bail_switch;;
                    }
                };
                {
                    (pc += offset);
                    break;
                };
            }
            case OP_PACKED_SWITCH: {
                const u_int16_t *switchData;
                u_int32_t testVal;
                int32_t offset;
                vsrc1 = ((inst) >> 8);
                offset = (pc[(1)]) | (((int32_t) (pc[(2)])) << 16);
                ((void) 0);
                switchData = pc + offset;
                if (switchData < curMethod->insns ||
                    switchData >= curMethod->insns + dvmGetMethodInsnsSize(curMethod)) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    dvmThrowException("Ljava/lang/InternalError;", "bad packed switch");
                    goto exceptionThrown;;
                }
                testVal = (fp[(vsrc1)]);
                offset = dvmInterpHandlePackedSwitch(switchData, testVal);
                ((void) 0);
                if (offset <= 0) {
                    dvmCheckSuspendQuick(self);
                    if ((false)) {
                        (pc += offset);
                        interpState->entryPoint = kInterpEntryInstr;
                                LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                      (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                      (offset));
                        goto bail_switch;;
                    }
                };
                {
                    (pc += offset);
                    break;
                };
            }
            case OP_SPARSE_SWITCH: {
                const u_int16_t *switchData;
                u_int32_t testVal;
                int32_t offset;
                vsrc1 = ((inst) >> 8);
                offset = (pc[(1)]) | (((int32_t) (pc[(2)])) << 16);
                ((void) 0);
                switchData = pc + offset;
                if (switchData < curMethod->insns ||
                    switchData >= curMethod->insns + dvmGetMethodInsnsSize(curMethod)) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    dvmThrowException("Ljava/lang/InternalError;", "bad sparse switch");
                    goto exceptionThrown;;
                }
                testVal = (fp[(vsrc1)]);
                //todo: check this
                offset = dvmInterpHandleSparseSwitch(switchData, testVal);
                ((void) 0);
                if (offset <= 0) {
                    dvmCheckSuspendQuick(self);
                    if ((false)) {
                        (pc += offset);
                        interpState->entryPoint = kInterpEntryInstr;
                                LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                      (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                      (offset));
                        goto bail_switch;;
                    }
                };
                {
                    (pc += offset);
                    break;
                };
            }
            case OP_CMPL_FLOAT: {
                int result;
                u_int16_t regs;
                float val1, val2;
                vdst = ((inst) >> 8);
                regs = (pc[(1)]);
                vsrc1 = regs & 0xff;
                vsrc2 = regs >> 8;
                ((void) 0);
                val1 = (*((float *) &fp[(vsrc1)]));
                val2 = (*((float *) &fp[(vsrc2)]));
                if (val1 == val2) result = 0;
                else if (val1 < val2) result = -1;
                else if (val1 > val2)
                    result = 1;
                else result = (-1);
                ((void) 0);
                (fp[(vdst)] = (result));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_CMPG_FLOAT: {
                int result;
                u_int16_t regs;
                float val1, val2;
                vdst = ((inst) >> 8);
                regs = (pc[(1)]);
                vsrc1 = regs & 0xff;
                vsrc2 = regs >> 8;
                ((void) 0);
                val1 = (*((float *) &fp[(vsrc1)]));
                val2 = (*((float *) &fp[(vsrc2)]));
                if (val1 == val2) result = 0;
                else if (val1 < val2) result = -1;
                else if (val1 > val2)
                    result = 1;
                else result = (1);
                ((void) 0);
                (fp[(vdst)] = (result));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_CMPL_DOUBLE: {
                int result;
                u_int16_t regs;
                double val1, val2;
                vdst = ((inst) >> 8);
                regs = (pc[(1)]);
                vsrc1 = regs & 0xff;
                vsrc2 = regs >> 8;
                ((void) 0);
                val1 = getDoubleFromArray(fp, (vsrc1));
                val2 = getDoubleFromArray(fp, (vsrc2));
                if (val1 == val2) result = 0;
                else if (val1 < val2) result = -1;
                else if (val1 > val2)
                    result = 1;
                else result = (-1);
                ((void) 0);
                (fp[(vdst)] = (result));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_CMPG_DOUBLE: {
                int result;
                u_int16_t regs;
                double val1, val2;
                vdst = ((inst) >> 8);
                regs = (pc[(1)]);
                vsrc1 = regs & 0xff;
                vsrc2 = regs >> 8;
                ((void) 0);
                val1 = getDoubleFromArray(fp, (vsrc1));
                val2 = getDoubleFromArray(fp, (vsrc2));
                if (val1 == val2) result = 0;
                else if (val1 < val2) result = -1;
                else if (val1 > val2)
                    result = 1;
                else result = (1);
                ((void) 0);
                (fp[(vdst)] = (result));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_CMP_LONG: {
                int result;
                u_int16_t regs;
                s8 val1, val2;
                vdst = ((inst) >> 8);
                regs = (pc[(1)]);
                vsrc1 = regs & 0xff;
                vsrc2 = regs >> 8;
                ((void) 0);
                val1 = getLongFromArray(fp, (vsrc1));
                val2 = getLongFromArray(fp, (vsrc2));
                if (val1 == val2) result = 0;
                else if (val1 < val2) result = -1;
                else if (val1 > val2)
                    result = 1;
                else result = (0);
                ((void) 0);
                (fp[(vdst)] = (result));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IF_EQ:
                vsrc1 = (((inst) >> 8) & 0x0f);
                vsrc2 = ((inst) >> 12);
                if ((int32_t) (fp[(vsrc1)]) == (int32_t) (fp[(vsrc2)])) {
                    int branchOffset = (int16_t) (pc[(1)]);
                    ((void) 0);
                    ((void) 0);
                    if (branchOffset < 0) {
                        dvmCheckSuspendQuick(self);
                        if ((false)) {
                            (pc += branchOffset);
                            interpState->entryPoint = kInterpEntryInstr;
                                    LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                          (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                          (branchOffset));
                            goto bail_switch;;
                        }
                    };
                    {
                        (pc += branchOffset);
                        break;
                    };
                } else {
                    ((void) 0);
                    {
                        (pc += 2);
                        break;
                    };
                }
            case OP_IF_NE:
                vsrc1 = (((inst) >> 8) & 0x0f);
                vsrc2 = ((inst) >> 12);
                if ((int32_t) (fp[(vsrc1)]) != (int32_t) (fp[(vsrc2)])) {
                    int branchOffset = (int16_t) (pc[(1)]);
                    ((void) 0);
                    ((void) 0);
                    if (branchOffset < 0) {
                        dvmCheckSuspendQuick(self);
                        if ((false)) {
                            (pc += branchOffset);
                            interpState->entryPoint = kInterpEntryInstr;
                                    LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                          (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                          (branchOffset));
                            goto bail_switch;;
                        }
                    };
                    {
                        (pc += branchOffset);
                        break;
                    };
                } else {
                    ((void) 0);
                    {
                        (pc += 2);
                        break;
                    };
                }
            case OP_IF_LT:
                vsrc1 = (((inst) >> 8) & 0x0f);
                vsrc2 = ((inst) >> 12);
                if ((int32_t) (fp[(vsrc1)]) < (int32_t) (fp[(vsrc2)])) {
                    int branchOffset = (int16_t) (pc[(1)]);
                    ((void) 0);
                    ((void) 0);
                    if (branchOffset < 0) {
                        dvmCheckSuspendQuick(self);
                        if ((false)) {
                            (pc += branchOffset);
                            interpState->entryPoint = kInterpEntryInstr;
                                    LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                          (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                          (branchOffset));
                            goto bail_switch;;
                        }
                    };
                    {
                        (pc += branchOffset);
                        break;
                    };
                } else {
                    ((void) 0);
                    {
                        (pc += 2);
                        break;
                    };
                }
            case OP_IF_GE:
                vsrc1 = (((inst) >> 8) & 0x0f);
                vsrc2 = ((inst) >> 12);
                if ((int32_t) (fp[(vsrc1)]) >= (int32_t) (fp[(vsrc2)])) {
                    int branchOffset = (int16_t) (pc[(1)]);
                    ((void) 0);
                    ((void) 0);
                    if (branchOffset < 0) {
                        dvmCheckSuspendQuick(self);
                        if ((false)) {
                            (pc += branchOffset);
                            interpState->entryPoint = kInterpEntryInstr;
                                    LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                          (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                          (branchOffset));
                            goto bail_switch;;
                        }
                    };
                    {
                        (pc += branchOffset);
                        break;
                    };
                } else {
                    ((void) 0);
                    {
                        (pc += 2);
                        break;
                    };
                }
            case OP_IF_GT:
                vsrc1 = (((inst) >> 8) & 0x0f);
                vsrc2 = ((inst) >> 12);
                if ((int32_t) (fp[(vsrc1)]) > (int32_t) (fp[(vsrc2)])) {
                    int branchOffset = (int16_t) (pc[(1)]);
                    ((void) 0);
                    ((void) 0);
                    if (branchOffset < 0) {
                        dvmCheckSuspendQuick(self);
                        if ((false)) {
                            (pc += branchOffset);
                            interpState->entryPoint = kInterpEntryInstr;
                                    LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                          (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                          (branchOffset));
                            goto bail_switch;;
                        }
                    };
                    {
                        (pc += branchOffset);
                        break;
                    };
                } else {
                    ((void) 0);
                    {
                        (pc += 2);
                        break;
                    };
                }
            case OP_IF_LE:
                vsrc1 = (((inst) >> 8) & 0x0f);
                vsrc2 = ((inst) >> 12);
                if ((int32_t) (fp[(vsrc1)]) <= (int32_t) (fp[(vsrc2)])) {
                    int branchOffset = (int16_t) (pc[(1)]);
                    ((void) 0);
                    ((void) 0);
                    if (branchOffset < 0) {
                        dvmCheckSuspendQuick(self);
                        if ((false)) {
                            (pc += branchOffset);
                            interpState->entryPoint = kInterpEntryInstr;
                                    LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                          (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                          (branchOffset));
                            goto bail_switch;;
                        }
                    };
                    {
                        (pc += branchOffset);
                        break;
                    };
                } else {
                    ((void) 0);
                    {
                        (pc += 2);
                        break;
                    };
                }
            case OP_IF_EQZ:
                vsrc1 = ((inst) >> 8);
                if ((int32_t) (fp[(vsrc1)]) == 0) {
                    int branchOffset = (int16_t) (pc[(1)]);
                    ((void) 0);
                    ((void) 0);
                    if (branchOffset < 0) {
                        dvmCheckSuspendQuick(self);
                        if ((false)) {
                            (pc += branchOffset);
                            interpState->entryPoint = kInterpEntryInstr;
                                    LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                          (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                          (branchOffset));
                            goto bail_switch;;
                        }
                    };
                    {
                        (pc += branchOffset);
                        break;
                    };
                } else {
                    ((void) 0);
                    {
                        (pc += 2);
                        break;
                    };
                }
            case OP_IF_NEZ:
                vsrc1 = ((inst) >> 8);
                if ((int32_t) (fp[(vsrc1)]) != 0) {
                    int branchOffset = (int16_t) (pc[(1)]);
                    ((void) 0);
                    ((void) 0);
                    if (branchOffset < 0) {
                        dvmCheckSuspendQuick(self);
                        if ((false)) {
                            (pc += branchOffset);
                            interpState->entryPoint = kInterpEntryInstr;
                                    LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                          (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                          (branchOffset));
                            goto bail_switch;;
                        }
                    };
                    {
                        (pc += branchOffset);
                        break;
                    };
                } else {
                    ((void) 0);
                    {
                        (pc += 2);
                        break;
                    };
                }
            case OP_IF_LTZ:
                vsrc1 = ((inst) >> 8);
                if ((int32_t) (fp[(vsrc1)]) < 0) {
                    int branchOffset = (int16_t) (pc[(1)]);
                    ((void) 0);
                    ((void) 0);
                    if (branchOffset < 0) {
                        dvmCheckSuspendQuick(self);
                        if ((false)) {
                            (pc += branchOffset);
                            interpState->entryPoint = kInterpEntryInstr;
                                    LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                          (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                          (branchOffset));
                            goto bail_switch;;
                        }
                    };
                    {
                        (pc += branchOffset);
                        break;
                    };
                } else {
                    ((void) 0);
                    {
                        (pc += 2);
                        break;
                    };
                }
            case OP_IF_GEZ:
                vsrc1 = ((inst) >> 8);
                if ((int32_t) (fp[(vsrc1)]) >= 0) {
                    int branchOffset = (int16_t) (pc[(1)]);
                    ((void) 0);
                    ((void) 0);
                    if (branchOffset < 0) {
                        dvmCheckSuspendQuick(self);
                        if ((false)) {
                            (pc += branchOffset);
                            interpState->entryPoint = kInterpEntryInstr;
                                    LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                          (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                          (branchOffset));
                            goto bail_switch;;
                        }
                    };
                    {
                        (pc += branchOffset);
                        break;
                    };
                } else {
                    ((void) 0);
                    {
                        (pc += 2);
                        break;
                    };
                }
            case OP_IF_GTZ:
                vsrc1 = ((inst) >> 8);
                if ((int32_t) (fp[(vsrc1)]) > 0) {
                    int branchOffset = (int16_t) (pc[(1)]);
                    ((void) 0);
                    ((void) 0);
                    if (branchOffset < 0) {
                        dvmCheckSuspendQuick(self);
                        if ((false)) {
                            (pc += branchOffset);
                            interpState->entryPoint = kInterpEntryInstr;
                                    LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                          (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                          (branchOffset));
                            goto bail_switch;;
                        }
                    };
                    {
                        (pc += branchOffset);
                        break;
                    };
                } else {
                    ((void) 0);
                    {
                        (pc += 2);
                        break;
                    };
                }
            case OP_IF_LEZ:
                vsrc1 = ((inst) >> 8);
                if ((int32_t) (fp[(vsrc1)]) <= 0) {
                    int branchOffset = (int16_t) (pc[(1)]);
                    ((void) 0);
                    ((void) 0);
                    if (branchOffset < 0) {
                        dvmCheckSuspendQuick(self);
                        if ((false)) {
                            (pc += branchOffset);
                            interpState->entryPoint = kInterpEntryInstr;
                                    LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                          (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryInstr),
                                          (branchOffset));
                            goto bail_switch;;
                        }
                    };
                    {
                        (pc += branchOffset);
                        break;
                    };
                } else {
                    ((void) 0);
                    {
                        (pc += 2);
                        break;
                    };
                }
            case OP_UNUSED_3E:
            case OP_UNUSED_3F:
            case OP_UNUSED_40:
            case OP_UNUSED_41:
            case OP_UNUSED_42:
            case OP_UNUSED_43:
            case OP_AGET: {
                ArrayObject *arrayObj;
                u_int16_t arrayInfo;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                arrayInfo = (pc[(1)]);
                vsrc1 = arrayInfo & 0xff;
                vsrc2 = arrayInfo >> 8;
                ((void) 0);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!checkForNull((Object *) arrayObj)) goto exceptionThrown;;
                if ((fp[(vsrc2)]) >= arrayObj->length) {
                    LOGV("Invalid array access: %p %d (len=%d)\n", arrayObj, vsrc2, arrayObj->length);
                    dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;", NULL);
                    goto exceptionThrown;;
                }
                (fp[(vdst)] = (((u8 *) arrayObj->contents)[(fp[(vsrc2)])]));
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_AGET_WIDE: {
                ArrayObject *arrayObj;
                u_int16_t arrayInfo;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                arrayInfo = (pc[(1)]);
                vsrc1 = arrayInfo & 0xff;
                vsrc2 = arrayInfo >> 8;
                ((void) 0);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!checkForNull((Object *) arrayObj)) goto exceptionThrown;;
                if ((fp[(vsrc2)]) >= arrayObj->length) {
                    LOGV("Invalid array access: %p %d (len=%d)\n", arrayObj, vsrc2, arrayObj->length);
                    dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;", NULL);
                    goto exceptionThrown;;
                }
                putLongToArray(fp, (vdst), (((s8 *) arrayObj->contents)[(fp[(vsrc2)])]));
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_AGET_OBJECT: {
                ArrayObject *arrayObj;
                u_int16_t arrayInfo;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                arrayInfo = (pc[(1)]);
                vsrc1 = arrayInfo & 0xff;
                vsrc2 = arrayInfo >> 8;
                ((void) 0);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!checkForNull((Object *) arrayObj)) goto exceptionThrown;;
                if ((fp[(vsrc2)]) >= arrayObj->length) {
                    LOGV("Invalid array access: %p %d (len=%d)\n", arrayObj, vsrc2, arrayObj->length);
                    dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;", NULL);
                    goto exceptionThrown;;
                }
                (fp[(vdst)] = (((u8 *) arrayObj->contents)[(fp[(vsrc2)])]));
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_AGET_BOOLEAN: {
                ArrayObject *arrayObj;
                u_int16_t arrayInfo;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                arrayInfo = (pc[(1)]);
                vsrc1 = arrayInfo & 0xff;
                vsrc2 = arrayInfo >> 8;
                ((void) 0);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!checkForNull((Object *) arrayObj)) goto exceptionThrown;;
                if ((fp[(vsrc2)]) >= arrayObj->length) {
                    LOGV("Invalid array access: %p %d (len=%d)\n", arrayObj, vsrc2, arrayObj->length);
                    dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;", NULL);
                    goto exceptionThrown;;
                }
                (fp[(vdst)] = (((u_int8_t *) arrayObj->contents)[(fp[(vsrc2)])]));
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_AGET_BYTE: {
                ArrayObject *arrayObj;
                u_int16_t arrayInfo;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                arrayInfo = (pc[(1)]);
                vsrc1 = arrayInfo & 0xff;
                vsrc2 = arrayInfo >> 8;
                ((void) 0);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!checkForNull((Object *) arrayObj)) goto exceptionThrown;;
                if ((fp[(vsrc2)]) >= arrayObj->length) {
                    LOGV("Invalid array access: %p %d (len=%d)\n", arrayObj, vsrc2, arrayObj->length);
                    dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;", NULL);
                    goto exceptionThrown;;
                }
                (fp[(vdst)] = (((s1 *) arrayObj->contents)[(fp[(vsrc2)])]));
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_AGET_CHAR: {
                ArrayObject *arrayObj;
                u_int16_t arrayInfo;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                arrayInfo = (pc[(1)]);
                vsrc1 = arrayInfo & 0xff;
                vsrc2 = arrayInfo >> 8;
                ((void) 0);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!checkForNull((Object *) arrayObj)) goto exceptionThrown;;
                if ((fp[(vsrc2)]) >= arrayObj->length) {
                    LOGV("Invalid array access: %p %d (len=%d)\n", arrayObj, vsrc2, arrayObj->length);
                    dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;", NULL);
                    goto exceptionThrown;;
                }
                (fp[(vdst)] = (((u_int16_t *) arrayObj->contents)[(fp[(vsrc2)])]));
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_AGET_SHORT: {
                ArrayObject *arrayObj;
                u_int16_t arrayInfo;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                arrayInfo = (pc[(1)]);
                vsrc1 = arrayInfo & 0xff;
                vsrc2 = arrayInfo >> 8;
                ((void) 0);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!checkForNull((Object *) arrayObj)) goto exceptionThrown;;
                if ((fp[(vsrc2)]) >= arrayObj->length) {
                    LOGV("Invalid array access: %p %d (len=%d)\n", arrayObj, vsrc2, arrayObj->length);
                    dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;", NULL);
                    goto exceptionThrown;;
                }
                (fp[(vdst)] = (((int16_t *) arrayObj->contents)[(fp[(vsrc2)])]));
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_APUT: {
                ArrayObject *arrayObj;
                u_int16_t arrayInfo;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                arrayInfo = (pc[(1)]);
                vsrc1 = arrayInfo & 0xff;
                vsrc2 = arrayInfo >> 8;
                ((void) 0);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!checkForNull((Object *) arrayObj)) goto exceptionThrown;;
                if ((fp[(vsrc2)]) >= arrayObj->length) {
                    dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;", NULL);
                    goto exceptionThrown;;
                }
                ((void) 0);
                ((u8 *) arrayObj->contents)[(fp[(vsrc2)])] = (fp[(vdst)]);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_APUT_WIDE: {
                ArrayObject *arrayObj;
                u_int16_t arrayInfo;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                arrayInfo = (pc[(1)]);
                vsrc1 = arrayInfo & 0xff;
                vsrc2 = arrayInfo >> 8;
                ((void) 0);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!checkForNull((Object *) arrayObj)) goto exceptionThrown;;
                if ((fp[(vsrc2)]) >= arrayObj->length) {
                    dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;", NULL);
                    goto exceptionThrown;;
                }
                ((void) 0);
                ((s8 *) arrayObj->contents)[(fp[(vsrc2)])] = getLongFromArray(fp, (vdst));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_APUT_OBJECT: {
                ArrayObject *arrayObj;
                Object *obj;
                u_int16_t arrayInfo;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                arrayInfo = (pc[(1)]);
                vsrc1 = arrayInfo & 0xff;
                vsrc2 = arrayInfo >> 8;
                ((void) 0);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!checkForNull((Object *) arrayObj))
                    goto exceptionThrown;;
                if ((fp[(vsrc2)]) >= arrayObj->length) {
                    dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;",
                                      NULL);
                    goto exceptionThrown;;
                }
                obj = (Object *) (fp[(vdst)]);
                if (obj != NULL) {
                    if (!checkForNull(obj))
                        goto exceptionThrown;;
                    if (!dvmCanPutArrayElement(obj->clazz, arrayObj->obj.clazz)) {
                        LOGV("Can't put a '%s'(%p) into array type='%s'(%p)\n",
                             obj->clazz->descriptor, obj,
                             arrayObj->obj.clazz->descriptor, arrayObj);
                        dvmThrowException("Ljava/lang/ArrayStoreException;", NULL);
                        goto exceptionThrown;;
                    }
                }
                ((void) 0);
                ((u8 *) arrayObj->contents)[(fp[(vsrc2)])] =
                        (fp[(vdst)]);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_APUT_BOOLEAN: {
                ArrayObject *arrayObj;
                u_int16_t arrayInfo;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                arrayInfo = (pc[(1)]);
                vsrc1 = arrayInfo & 0xff;
                vsrc2 = arrayInfo >> 8;
                ((void) 0);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!checkForNull((Object *) arrayObj)) goto exceptionThrown;;
                if ((fp[(vsrc2)]) >= arrayObj->length) {
                    dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;", NULL);
                    goto exceptionThrown;;
                }
                ((void) 0);
                ((u_int8_t *) arrayObj->contents)[(fp[(vsrc2)])] = (fp[(vdst)]);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_APUT_BYTE: {
                ArrayObject *arrayObj;
                u_int16_t arrayInfo;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                arrayInfo = (pc[(1)]);
                vsrc1 = arrayInfo & 0xff;
                vsrc2 = arrayInfo >> 8;
                ((void) 0);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!checkForNull((Object *) arrayObj)) goto exceptionThrown;;
                if ((fp[(vsrc2)]) >= arrayObj->length) {
                    dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;", NULL);
                    goto exceptionThrown;;
                }
                ((void) 0);
                ((s1 *) arrayObj->contents)[(fp[(vsrc2)])] = (fp[(vdst)]);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_APUT_CHAR: {
                ArrayObject *arrayObj;
                u_int16_t arrayInfo;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                arrayInfo = (pc[(1)]);
                vsrc1 = arrayInfo & 0xff;
                vsrc2 = arrayInfo >> 8;
                ((void) 0);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!checkForNull((Object *) arrayObj)) goto exceptionThrown;;
                if ((fp[(vsrc2)]) >= arrayObj->length) {
                    dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;", NULL);
                    goto exceptionThrown;;
                }
                ((void) 0);
                ((u_int16_t *) arrayObj->contents)[(fp[(vsrc2)])] = (fp[(vdst)]);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_APUT_SHORT: {
                ArrayObject *arrayObj;
                u_int16_t arrayInfo;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = ((inst) >> 8);
                arrayInfo = (pc[(1)]);
                vsrc1 = arrayInfo & 0xff;
                vsrc2 = arrayInfo >> 8;
                ((void) 0);
                arrayObj = (ArrayObject *) (fp[(vsrc1)]);
                if (!checkForNull((Object *) arrayObj)) goto exceptionThrown;;
                if ((fp[(vsrc2)]) >= arrayObj->length) {
                    dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;", NULL);
                    goto exceptionThrown;;
                }
                ((void) 0);
                ((int16_t *) arrayObj->contents)[(fp[(vsrc2)])] = (fp[(vdst)]);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IGET: {
                InstField *ifield;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) goto exceptionThrown;;
                ifield = (InstField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (ifield == NULL) {
                    ifield = dvmResolveInstField(curMethod->clazz, ref);
                    if (ifield == NULL) goto exceptionThrown;;
                }
                (fp[(vdst)] = (dvmGetFieldInt(obj, ifield->byteOffset)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IGET_WIDE: {
                InstField *ifield;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) goto exceptionThrown;;
                ifield = (InstField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (ifield == NULL) {
                    ifield = dvmResolveInstField(curMethod->clazz, ref);
                    if (ifield == NULL) goto exceptionThrown;;
                }
                putLongToArray(fp, (vdst), (dvmGetFieldLong(obj, ifield->byteOffset)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IGET_OBJECT: {
                InstField *ifield;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) goto exceptionThrown;;
                ifield = (InstField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (ifield == NULL) {
                    ifield = dvmResolveInstField(curMethod->clazz, ref);
                    if (ifield == NULL) goto exceptionThrown;;
                }
                (fp[(vdst)] = (u8) (dvmGetFieldObject(obj, ifield->byteOffset)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IGET_BOOLEAN: {
                InstField *ifield;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) goto exceptionThrown;;
                ifield = (InstField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (ifield == NULL) {
                    ifield = dvmResolveInstField(curMethod->clazz, ref);
                    if (ifield == NULL) goto exceptionThrown;;
                }
                (fp[(vdst)] = (dvmGetFieldInt(obj, ifield->byteOffset)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IGET_BYTE: {
                InstField *ifield;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) goto exceptionThrown;;
                ifield = (InstField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (ifield == NULL) {
                    ifield = dvmResolveInstField(curMethod->clazz, ref);
                    if (ifield == NULL) goto exceptionThrown;;
                }
                (fp[(vdst)] = (dvmGetFieldInt(obj, ifield->byteOffset)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IGET_CHAR: {
                InstField *ifield;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) goto exceptionThrown;;
                ifield = (InstField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (ifield == NULL) {
                    ifield = dvmResolveInstField(curMethod->clazz, ref);
                    if (ifield == NULL) goto exceptionThrown;;
                }
                (fp[(vdst)] = (dvmGetFieldInt(obj, ifield->byteOffset)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IGET_SHORT: {
                InstField *ifield;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) goto exceptionThrown;;
                ifield = (InstField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (ifield == NULL) {
                    ifield = dvmResolveInstField(curMethod->clazz, ref);
                    if (ifield == NULL) goto exceptionThrown;;
                }
                (fp[(vdst)] = (dvmGetFieldInt(obj, ifield->byteOffset)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IPUT: {
                InstField *ifield;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) goto exceptionThrown;;
                ifield = (InstField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (ifield == NULL) {
                    ifield = dvmResolveInstField(curMethod->clazz, ref);
                    if (ifield == NULL) goto exceptionThrown;;
                }
                dvmSetFieldInt(obj, ifield->byteOffset, (fp[(vdst)]));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IPUT_WIDE: {
                InstField *ifield;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) goto exceptionThrown;;
                ifield = (InstField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (ifield == NULL) {
                    ifield = dvmResolveInstField(curMethod->clazz, ref);
                    if (ifield == NULL) goto exceptionThrown;;
                }
                dvmSetFieldLong(obj, ifield->byteOffset, getLongFromArray(fp, (vdst)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IPUT_OBJECT: {
                InstField *ifield;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) goto exceptionThrown;;
                ifield = (InstField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (ifield == NULL) {
                    ifield = dvmResolveInstField(curMethod->clazz, ref);
                    if (ifield == NULL) goto exceptionThrown;;
                }
                dvmSetFieldObject(obj, ifield->byteOffset, ((Object *) fp[(vdst)]));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IPUT_BOOLEAN: {
                InstField *ifield;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) goto exceptionThrown;;
                ifield = (InstField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (ifield == NULL) {
                    ifield = dvmResolveInstField(curMethod->clazz, ref);
                    if (ifield == NULL) goto exceptionThrown;;
                }
                dvmSetFieldInt(obj, ifield->byteOffset, (fp[(vdst)]));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IPUT_BYTE: {
                InstField *ifield;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) goto exceptionThrown;;
                ifield = (InstField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (ifield == NULL) {
                    ifield = dvmResolveInstField(curMethod->clazz, ref);
                    if (ifield == NULL) goto exceptionThrown;;
                }
                dvmSetFieldInt(obj, ifield->byteOffset, (fp[(vdst)]));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IPUT_CHAR: {
                InstField *ifield;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) goto exceptionThrown;;
                ifield = (InstField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (ifield == NULL) {
                    ifield = dvmResolveInstField(curMethod->clazz, ref);
                    if (ifield == NULL) goto exceptionThrown;;
                }
                dvmSetFieldInt(obj, ifield->byteOffset, (fp[(vdst)]));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IPUT_SHORT: {
                InstField *ifield;
                Object *obj;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNull(obj)) goto exceptionThrown;;
                ifield = (InstField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (ifield == NULL) {
                    ifield = dvmResolveInstField(curMethod->clazz, ref);
                    if (ifield == NULL) goto exceptionThrown;;
                }
                dvmSetFieldInt(obj, ifield->byteOffset, (fp[(vdst)]));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SGET: {
                StaticField *sfield;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                sfield = (StaticField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (sfield == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    sfield = dvmResolveStaticField(curMethod->clazz, ref);
                    if (sfield == NULL) goto exceptionThrown;;
                }
                (fp[(vdst)] = (dvmGetStaticFieldInt(sfield)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SGET_WIDE: {
                StaticField *sfield;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                sfield = (StaticField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (sfield == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    sfield = dvmResolveStaticField(curMethod->clazz, ref);
                    if (sfield == NULL) goto exceptionThrown;;
                }
                putLongToArray(fp, (vdst), (dvmGetStaticFieldLong(sfield)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SGET_OBJECT: {
                StaticField *sfield;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                sfield = (StaticField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (sfield == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    sfield = dvmResolveStaticField(curMethod->clazz, ref);
                    if (sfield == NULL) goto exceptionThrown;;
                }
                (fp[(vdst)] = (u8) (dvmGetStaticFieldObject(sfield)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SGET_BOOLEAN: {
                StaticField *sfield;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                sfield = (StaticField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (sfield == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    sfield = dvmResolveStaticField(curMethod->clazz, ref);
                    if (sfield == NULL) goto exceptionThrown;;
                }
                (fp[(vdst)] = (dvmGetStaticFieldInt(sfield)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SGET_BYTE: {
                StaticField *sfield;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                sfield = (StaticField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (sfield == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    sfield = dvmResolveStaticField(curMethod->clazz, ref);
                    if (sfield == NULL) goto exceptionThrown;;
                }
                (fp[(vdst)] = (dvmGetStaticFieldInt(sfield)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SGET_CHAR: {
                StaticField *sfield;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                sfield = (StaticField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (sfield == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    sfield = dvmResolveStaticField(curMethod->clazz, ref);
                    if (sfield == NULL) goto exceptionThrown;;
                }
                (fp[(vdst)] = (dvmGetStaticFieldInt(sfield)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SGET_SHORT: {
                StaticField *sfield;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                sfield = (StaticField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (sfield == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    sfield = dvmResolveStaticField(curMethod->clazz, ref);
                    if (sfield == NULL) goto exceptionThrown;;
                }
                (fp[(vdst)] = (dvmGetStaticFieldInt(sfield)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SPUT: {
                StaticField *sfield;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                sfield = (StaticField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (sfield == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    sfield = dvmResolveStaticField(curMethod->clazz, ref);
                    if (sfield == NULL) goto exceptionThrown;;
                }
                dvmSetStaticFieldInt(sfield, (fp[(vdst)]));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SPUT_WIDE: {
                StaticField *sfield;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                sfield = (StaticField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (sfield == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    sfield = dvmResolveStaticField(curMethod->clazz, ref);
                    if (sfield == NULL) goto exceptionThrown;;
                }
                dvmSetStaticFieldLong(sfield, getLongFromArray(fp, (vdst)));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SPUT_OBJECT: {
                StaticField *sfield;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                sfield = (StaticField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (sfield == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    sfield = dvmResolveStaticField(curMethod->clazz, ref);
                    if (sfield == NULL) goto exceptionThrown;;
                }
                dvmSetStaticFieldObject(sfield, ((Object *) fp[(vdst)]));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SPUT_BOOLEAN: {
                StaticField *sfield;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                sfield = (StaticField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (sfield == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    sfield = dvmResolveStaticField(curMethod->clazz, ref);
                    if (sfield == NULL) goto exceptionThrown;;
                }
                dvmSetStaticFieldInt(sfield, (fp[(vdst)]));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SPUT_BYTE: {
                StaticField *sfield;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                sfield = (StaticField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (sfield == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    sfield = dvmResolveStaticField(curMethod->clazz, ref);
                    if (sfield == NULL) goto exceptionThrown;;
                }
                dvmSetStaticFieldInt(sfield, (fp[(vdst)]));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SPUT_CHAR: {
                StaticField *sfield;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                sfield = (StaticField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (sfield == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    sfield = dvmResolveStaticField(curMethod->clazz, ref);
                    if (sfield == NULL) goto exceptionThrown;;
                }
                dvmSetStaticFieldInt(sfield, (fp[(vdst)]));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SPUT_SHORT: {
                StaticField *sfield;
                vdst = ((inst) >> 8);
                ref = (pc[(1)]);
                ((void) 0);
                sfield = (StaticField *) dvmDexGetResolvedField(methodClassDex, ref);
                if (sfield == NULL) {
                    (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                    sfield = dvmResolveStaticField(curMethod->clazz, ref);
                    if (sfield == NULL) goto exceptionThrown;;
                }
                dvmSetStaticFieldInt(sfield, (fp[(vdst)]));
                ((void) 0);
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_INVOKE_VIRTUAL:
                do {
                    methodCallRange = false;
                    goto invokeVirtual;
                } while (false);
            case OP_INVOKE_SUPER:
                do {
                    methodCallRange = false;
                    goto invokeSuper;
                } while (false);
            case OP_INVOKE_DIRECT:
                do {
                    methodCallRange = false;
                    goto invokeDirect;
                } while (false);
            case OP_INVOKE_STATIC:
                do {
                    methodCallRange = false;
                    goto invokeStatic;
                } while (false);
            case OP_INVOKE_INTERFACE:
                do {
                    methodCallRange = false;
                    goto invokeInterface;
                } while (false);
            case OP_UNUSED_73:
            case OP_INVOKE_VIRTUAL_RANGE:
                do {
                    methodCallRange = true;
                    goto invokeVirtual;
                } while (false);
            case OP_INVOKE_SUPER_RANGE:
                do {
                    methodCallRange = true;
                    goto invokeSuper;
                } while (false);
            case OP_INVOKE_DIRECT_RANGE:
                do {
                    methodCallRange = true;
                    goto invokeDirect;
                } while (false);
            case OP_INVOKE_STATIC_RANGE:
                do {
                    methodCallRange = true;
                    goto invokeStatic;
                } while (false);
            case OP_INVOKE_INTERFACE_RANGE:
                do {
                    methodCallRange = true;
                    goto invokeInterface;
                } while (false);
            case OP_UNUSED_79:
            case OP_UNUSED_7A:
            case OP_NEG_INT:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (fp[(vdst)] = (-(fp[(vsrc1)])));
                {
                    (pc += 1);
                    break;
                };
            case OP_NOT_INT:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (fp[(vdst)] = ((fp[(vsrc1)]) ^ 0xffffffff));
                {
                    (pc += 1);
                    break;
                };
            case OP_NEG_LONG:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putLongToArray(fp, (vdst), (-getLongFromArray(fp, (vsrc1))));
                {
                    (pc += 1);
                    break;
                };
            case OP_NOT_LONG:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putLongToArray(fp, (vdst), (getLongFromArray(fp, (vsrc1)) ^ 0xffffffffffffffffULL));
                {
                    (pc += 1);
                    break;
                };
            case OP_NEG_FLOAT:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (*((float *) &fp[(vdst)]) = (-(*((float *) &fp[(vsrc1)]))));
                {
                    (pc += 1);
                    break;
                };
            case OP_NEG_DOUBLE:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putDoubleToArray(fp, (vdst), (-getDoubleFromArray(fp, (vsrc1))));
                {
                    (pc += 1);
                    break;
                };
            case OP_INT_TO_LONG:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putLongToArray(fp, (vdst), (((int32_t) (fp[(vsrc1)]))));
                {
                    (pc += 1);
                    break;
                };
            case OP_INT_TO_FLOAT:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (*((float *) &fp[(vdst)]) = (((int32_t) (fp[(vsrc1)]))));
                {
                    (pc += 1);
                    break;
                };
            case OP_INT_TO_DOUBLE:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putDoubleToArray(fp, (vdst), (((int32_t) (fp[(vsrc1)]))));
                {
                    (pc += 1);
                    break;
                };
            case OP_LONG_TO_INT:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (fp[(vdst)] = ((int32_t) getLongFromArray(fp, (vsrc1))));
                {
                    (pc += 1);
                    break;
                };
            case OP_LONG_TO_FLOAT:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (*((float *) &fp[(vdst)]) = (getLongFromArray(fp, (vsrc1))));
                {
                    (pc += 1);
                    break;
                };
            case OP_LONG_TO_DOUBLE:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putDoubleToArray(fp, (vdst), (getLongFromArray(fp, (vsrc1))));
                {
                    (pc += 1);
                    break;
                };
            case OP_FLOAT_TO_INT: {
                float val;
                int32_t intMin, intMax, result;
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                val = (*((float *) &fp[(vsrc1)]));
                intMin = (int32_t) 1 << (sizeof(int32_t) * 8 - 1);
                intMax = ~intMin;
                result = (int32_t) val;
                if (val >= intMax) result = intMax;
                else if (val <= intMin) result = intMin;
                else if (val != val)
                    result = 0;
                else result = (int32_t) val;
                (fp[(vdst)] = ((int32_t) result));
            }
                {
                    (pc += 1);
                    break;
                };
            case OP_FLOAT_TO_LONG: {
                float val;
                s8 intMin, intMax, result;
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                val = (*((float *) &fp[(vsrc1)]));
                intMin = (s8) 1 << (sizeof(s8) * 8 - 1);
                intMax = ~intMin;
                result = (s8) val;
                if (val >= intMax) result = intMax;
                else if (val <= intMin) result = intMin;
                else if (val != val)
                    result = 0;
                else result = (s8) val;
                putLongToArray(fp, (vdst), (result));
            }
                {
                    (pc += 1);
                    break;
                };
            case OP_FLOAT_TO_DOUBLE:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putDoubleToArray(fp, (vdst), ((*((float *) &fp[(vsrc1)]))));
                {
                    (pc += 1);
                    break;
                };
            case OP_DOUBLE_TO_INT: {
                double val;
                int32_t intMin, intMax, result;
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                val = getDoubleFromArray(fp, (vsrc1));
                intMin = (int32_t) 1 << (sizeof(int32_t) * 8 - 1);
                intMax = ~intMin;
                result = (int32_t) val;
                if (val >= intMax) result = intMax;
                else if (val <= intMin) result = intMin;
                else if (val != val)
                    result = 0;
                else result = (int32_t) val;
                (fp[(vdst)] = ((int32_t) result));
            }
                {
                    (pc += 1);
                    break;
                };
            case OP_DOUBLE_TO_LONG: {
                double val;
                s8 intMin, intMax, result;
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                val = getDoubleFromArray(fp, (vsrc1));
                intMin = (s8) 1 << (sizeof(s8) * 8 - 1);
                intMax = ~intMin;
                result = (s8) val;
                if (val >= intMax) result = intMax;
                else if (val <= intMin) result = intMin;
                else if (val != val)
                    result = 0;
                else result = (s8) val;
                putLongToArray(fp, (vdst), (result));
            }
                {
                    (pc += 1);
                    break;
                };
            case OP_DOUBLE_TO_FLOAT:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (*((float *) &fp[(vdst)]) = (getDoubleFromArray(fp, (vsrc1))));
                {
                    (pc += 1);
                    break;
                };
            case OP_INT_TO_BYTE:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (fp[(vdst)] = ((s1) (fp[(vsrc1)])));
                {
                    (pc += 1);
                    break;
                };
            case OP_INT_TO_CHAR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (fp[(vdst)] = ((u_int16_t) (fp[(vsrc1)])));
                {
                    (pc += 1);
                    break;
                };
            case OP_INT_TO_SHORT:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (fp[(vdst)] = ((int16_t) (fp[(vsrc1)])));
                {
                    (pc += 1);
                    break;
                };
            case OP_ADD_INT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vsrc1)]);
                    secondVal = (fp[(vsrc2)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal + secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) + (int32_t) (fp[(vsrc2)]))); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SUB_INT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vsrc1)]);
                    secondVal = (fp[(vsrc2)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal - secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) - (int32_t) (fp[(vsrc2)]))); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_MUL_INT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vsrc1)]);
                    secondVal = (fp[(vsrc2)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal * secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) * (int32_t) (fp[(vsrc2)]))); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_DIV_INT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (1 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vsrc1)]);
                    secondVal = (fp[(vsrc2)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (1 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal / secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) / (int32_t) (fp[(vsrc2)]))); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_REM_INT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (2 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vsrc1)]);
                    secondVal = (fp[(vsrc2)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (2 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal % secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) % (int32_t) (fp[(vsrc2)]))); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_AND_INT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vsrc1)]);
                    secondVal = (fp[(vsrc2)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal & secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) & (int32_t) (fp[(vsrc2)]))); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_OR_INT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vsrc1)]);
                    secondVal = (fp[(vsrc2)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal | secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) | (int32_t) (fp[(vsrc2)]))); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_XOR_INT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vsrc1)]);
                    secondVal = (fp[(vsrc2)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal ^ secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) ^ (int32_t) (fp[(vsrc2)]))); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SHL_INT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) << ((fp[(vsrc2)]) & 0x1f)));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SHR_INT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) >> ((fp[(vsrc2)]) & 0x1f)));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_USHR_INT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                (fp[(vdst)] = ((u_int32_t) (fp[(vsrc1)]) >> ((fp[(vsrc2)]) & 0x1f)));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_ADD_LONG: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (0 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vsrc1));
                    secondVal = getLongFromArray(fp, (vsrc2));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal + secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vsrc1)) + (s8) getLongFromArray(fp, (vsrc2))));
                }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SUB_LONG: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (0 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vsrc1));
                    secondVal = getLongFromArray(fp, (vsrc2));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal - secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vsrc1)) - (s8) getLongFromArray(fp, (vsrc2))));
                }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_MUL_LONG: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (0 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vsrc1));
                    secondVal = getLongFromArray(fp, (vsrc2));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal * secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vsrc1)) * (s8) getLongFromArray(fp, (vsrc2))));
                }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_DIV_LONG: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (1 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vsrc1));
                    secondVal = getLongFromArray(fp, (vsrc2));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (1 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal / secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vsrc1)) / (s8) getLongFromArray(fp, (vsrc2))));
                }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_REM_LONG: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (2 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vsrc1));
                    secondVal = getLongFromArray(fp, (vsrc2));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (2 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal % secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vsrc1)) % (s8) getLongFromArray(fp, (vsrc2))));
                }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_AND_LONG: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (0 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vsrc1));
                    secondVal = getLongFromArray(fp, (vsrc2));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal & secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vsrc1)) & (s8) getLongFromArray(fp, (vsrc2))));
                }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_OR_LONG: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (0 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vsrc1));
                    secondVal = getLongFromArray(fp, (vsrc2));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal | secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vsrc1)) | (s8) getLongFromArray(fp, (vsrc2))));
                }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_XOR_LONG: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                if (0 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vsrc1));
                    secondVal = getLongFromArray(fp, (vsrc2));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal ^ secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vsrc1)) ^ (s8) getLongFromArray(fp, (vsrc2))));
                }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SHL_LONG: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                putLongToArray(fp, (vdst), ((s8) getLongFromArray(fp, (vsrc1)) << ((fp[(vsrc2)]) & 0x3f)));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SHR_LONG: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                putLongToArray(fp, (vdst), ((s8) getLongFromArray(fp, (vsrc1)) >> ((fp[(vsrc2)]) & 0x3f)));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_USHR_LONG: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                putLongToArray(fp, (vdst), ((u_int64_t) getLongFromArray(fp, (vsrc1)) >> ((fp[(vsrc2)]) & 0x3f)));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_ADD_FLOAT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                (*((float *) &fp[(vdst)]) = ((*((float *) &fp[(vsrc1)])) + (*((float *) &fp[(vsrc2)]))));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SUB_FLOAT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                (*((float *) &fp[(vdst)]) = ((*((float *) &fp[(vsrc1)])) - (*((float *) &fp[(vsrc2)]))));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_MUL_FLOAT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                (*((float *) &fp[(vdst)]) = ((*((float *) &fp[(vsrc1)])) * (*((float *) &fp[(vsrc2)]))));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_DIV_FLOAT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                (*((float *) &fp[(vdst)]) = ((*((float *) &fp[(vsrc1)])) / (*((float *) &fp[(vsrc2)]))));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_REM_FLOAT: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                (*((float *) &fp[(vdst)]) = (fmodf((*((float *) &fp[(vsrc1)])), (*((float *) &fp[(vsrc2)])))));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_ADD_DOUBLE: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                putDoubleToArray(fp, (vdst), (getDoubleFromArray(fp, (vsrc1)) + getDoubleFromArray(fp, (vsrc2))));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SUB_DOUBLE: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                putDoubleToArray(fp, (vdst), (getDoubleFromArray(fp, (vsrc1)) - getDoubleFromArray(fp, (vsrc2))));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_MUL_DOUBLE: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                putDoubleToArray(fp, (vdst), (getDoubleFromArray(fp, (vsrc1)) * getDoubleFromArray(fp, (vsrc2))));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_DIV_DOUBLE: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                putDoubleToArray(fp, (vdst), (getDoubleFromArray(fp, (vsrc1)) / getDoubleFromArray(fp, (vsrc2))));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_REM_DOUBLE: {
                u_int16_t srcRegs;
                vdst = ((inst) >> 8);
                srcRegs = (pc[(1)]);
                vsrc1 = srcRegs & 0xff;
                vsrc2 = srcRegs >> 8;
                ((void) 0);
                putDoubleToArray(fp, (vdst), (fmod(getDoubleFromArray(fp, (vsrc1)), getDoubleFromArray(fp, (vsrc2)))));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_ADD_INT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vdst)]);
                    secondVal = (fp[(vsrc1)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal + secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vdst)]) + (int32_t) (fp[(vsrc1)]))); }
                {
                    (pc += 1);
                    break;
                };
            case OP_SUB_INT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vdst)]);
                    secondVal = (fp[(vsrc1)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal - secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vdst)]) - (int32_t) (fp[(vsrc1)]))); }
                {
                    (pc += 1);
                    break;
                };
            case OP_MUL_INT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vdst)]);
                    secondVal = (fp[(vsrc1)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal * secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vdst)]) * (int32_t) (fp[(vsrc1)]))); }
                {
                    (pc += 1);
                    break;
                };
            case OP_DIV_INT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (1 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vdst)]);
                    secondVal = (fp[(vsrc1)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (1 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal / secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vdst)]) / (int32_t) (fp[(vsrc1)]))); }
                {
                    (pc += 1);
                    break;
                };
            case OP_REM_INT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (2 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vdst)]);
                    secondVal = (fp[(vsrc1)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (2 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal % secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vdst)]) % (int32_t) (fp[(vsrc1)]))); }
                {
                    (pc += 1);
                    break;
                };
            case OP_AND_INT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vdst)]);
                    secondVal = (fp[(vsrc1)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal & secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vdst)]) & (int32_t) (fp[(vsrc1)]))); }
                {
                    (pc += 1);
                    break;
                };
            case OP_OR_INT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vdst)]);
                    secondVal = (fp[(vsrc1)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal | secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vdst)]) | (int32_t) (fp[(vsrc1)]))); }
                {
                    (pc += 1);
                    break;
                };
            case OP_XOR_INT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, secondVal, result;
                    firstVal = (fp[(vdst)]);
                    secondVal = (fp[(vsrc1)]);
                    if (secondVal == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && secondVal == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal ^ secondVal; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vdst)]) ^ (int32_t) (fp[(vsrc1)]))); }
                {
                    (pc += 1);
                    break;
                };
            case OP_SHL_INT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (fp[(vdst)] = ((int32_t) (fp[(vdst)]) << ((fp[(vsrc1)]) & 0x1f)));
                {
                    (pc += 1);
                    break;
                };
            case OP_SHR_INT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (fp[(vdst)] = ((int32_t) (fp[(vdst)]) >> ((fp[(vsrc1)]) & 0x1f)));
                {
                    (pc += 1);
                    break;
                };
            case OP_USHR_INT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (fp[(vdst)] = ((u_int32_t) (fp[(vdst)]) >> ((fp[(vsrc1)]) & 0x1f)));
                {
                    (pc += 1);
                    break;
                };
            case OP_ADD_LONG_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (0 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vdst));
                    secondVal = getLongFromArray(fp, (vsrc1));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal + secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vdst)) + (s8) getLongFromArray(fp, (vsrc1))));
                }
                {
                    (pc += 1);
                    break;
                };
            case OP_SUB_LONG_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (0 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vdst));
                    secondVal = getLongFromArray(fp, (vsrc1));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal - secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vdst)) - (s8) getLongFromArray(fp, (vsrc1))));
                }
                {
                    (pc += 1);
                    break;
                };
            case OP_MUL_LONG_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (0 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vdst));
                    secondVal = getLongFromArray(fp, (vsrc1));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal * secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vdst)) * (s8) getLongFromArray(fp, (vsrc1))));
                }
                {
                    (pc += 1);
                    break;
                };
            case OP_DIV_LONG_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (1 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vdst));
                    secondVal = getLongFromArray(fp, (vsrc1));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (1 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal / secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vdst)) / (s8) getLongFromArray(fp, (vsrc1))));
                }
                {
                    (pc += 1);
                    break;
                };
            case OP_REM_LONG_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (2 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vdst));
                    secondVal = getLongFromArray(fp, (vsrc1));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (2 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal % secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vdst)) % (s8) getLongFromArray(fp, (vsrc1))));
                }
                {
                    (pc += 1);
                    break;
                };
            case OP_AND_LONG_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (0 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vdst));
                    secondVal = getLongFromArray(fp, (vsrc1));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal & secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vdst)) & (s8) getLongFromArray(fp, (vsrc1))));
                }
                {
                    (pc += 1);
                    break;
                };
            case OP_OR_LONG_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (0 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vdst));
                    secondVal = getLongFromArray(fp, (vsrc1));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal | secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vdst)) | (s8) getLongFromArray(fp, (vsrc1))));
                }
                {
                    (pc += 1);
                    break;
                };
            case OP_XOR_LONG_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                if (0 != 0) {
                    s8 firstVal, secondVal, result;
                    firstVal = getLongFromArray(fp, (vdst));
                    secondVal = getLongFromArray(fp, (vsrc1));
                    if (secondVal == 0LL) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int64_t) firstVal == 0x8000000000000000ULL && secondVal == -1LL) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal ^ secondVal; }
                    putLongToArray(fp, (vdst), (result));
                } else {
                    putLongToArray(fp, (vdst),
                                   ((s8) getLongFromArray(fp, (vdst)) ^ (s8) getLongFromArray(fp, (vsrc1))));
                }
                {
                    (pc += 1);
                    break;
                };
            case OP_SHL_LONG_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putLongToArray(fp, (vdst), ((s8) getLongFromArray(fp, (vdst)) << ((fp[(vsrc1)]) & 0x3f)));
                {
                    (pc += 1);
                    break;
                };
            case OP_SHR_LONG_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putLongToArray(fp, (vdst), ((s8) getLongFromArray(fp, (vdst)) >> ((fp[(vsrc1)]) & 0x3f)));
                {
                    (pc += 1);
                    break;
                };
            case OP_USHR_LONG_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putLongToArray(fp, (vdst), ((u_int64_t) getLongFromArray(fp, (vdst)) >> ((fp[(vsrc1)]) & 0x3f)));
                {
                    (pc += 1);
                    break;
                };
            case OP_ADD_FLOAT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (*((float *) &fp[(vdst)]) = ((*((float *) &fp[(vdst)])) + (*((float *) &fp[(vsrc1)]))));
                {
                    (pc += 1);
                    break;
                };
            case OP_SUB_FLOAT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (*((float *) &fp[(vdst)]) = ((*((float *) &fp[(vdst)])) - (*((float *) &fp[(vsrc1)]))));
                {
                    (pc += 1);
                    break;
                };
            case OP_MUL_FLOAT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (*((float *) &fp[(vdst)]) = ((*((float *) &fp[(vdst)])) * (*((float *) &fp[(vsrc1)]))));
                {
                    (pc += 1);
                    break;
                };
            case OP_DIV_FLOAT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (*((float *) &fp[(vdst)]) = ((*((float *) &fp[(vdst)])) / (*((float *) &fp[(vsrc1)]))));
                {
                    (pc += 1);
                    break;
                };
            case OP_REM_FLOAT_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                (*((float *) &fp[(vdst)]) = (fmodf((*((float *) &fp[(vdst)])), (*((float *) &fp[(vsrc1)])))));
                {
                    (pc += 1);
                    break;
                };
            case OP_ADD_DOUBLE_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putDoubleToArray(fp, (vdst), (getDoubleFromArray(fp, (vdst)) + getDoubleFromArray(fp, (vsrc1))));
                {
                    (pc += 1);
                    break;
                };
            case OP_SUB_DOUBLE_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putDoubleToArray(fp, (vdst), (getDoubleFromArray(fp, (vdst)) - getDoubleFromArray(fp, (vsrc1))));
                {
                    (pc += 1);
                    break;
                };
            case OP_MUL_DOUBLE_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putDoubleToArray(fp, (vdst), (getDoubleFromArray(fp, (vdst)) * getDoubleFromArray(fp, (vsrc1))));
                {
                    (pc += 1);
                    break;
                };
            case OP_DIV_DOUBLE_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putDoubleToArray(fp, (vdst), (getDoubleFromArray(fp, (vdst)) / getDoubleFromArray(fp, (vsrc1))));
                {
                    (pc += 1);
                    break;
                };
            case OP_REM_DOUBLE_2ADDR:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ((void) 0);
                putDoubleToArray(fp, (vdst), (fmod(getDoubleFromArray(fp, (vdst)), getDoubleFromArray(fp, (vsrc1)))));
                {
                    (pc += 1);
                    break;
                };
            case OP_ADD_INT_LIT16:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                vsrc2 = (pc[(1)]);
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, result;
                    firstVal = (fp[(vsrc1)]);
                    if ((int16_t) vsrc2 == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && ((int16_t) vsrc2) == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal + (int16_t) vsrc2; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((fp[(vsrc1)]) + (int16_t) vsrc2)); }
                {
                    (pc += 2);
                    break;
                };
            case OP_RSUB_INT: {
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                vsrc2 = (pc[(1)]);
                ((void) 0);
                (fp[(vdst)] = ((int16_t) vsrc2 - (int32_t) (fp[(vsrc1)])));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_MUL_INT_LIT16:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                vsrc2 = (pc[(1)]);
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, result;
                    firstVal = (fp[(vsrc1)]);
                    if ((int16_t) vsrc2 == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && ((int16_t) vsrc2) == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal * (int16_t) vsrc2; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((fp[(vsrc1)]) * (int16_t) vsrc2)); }
                {
                    (pc += 2);
                    break;
                };
            case OP_DIV_INT_LIT16:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                vsrc2 = (pc[(1)]);
                ((void) 0);
                if (1 != 0) {
                    int32_t firstVal, result;
                    firstVal = (fp[(vsrc1)]);
                    if ((int16_t) vsrc2 == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && ((int16_t) vsrc2) == -1) {
                        if (1 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal / (int16_t) vsrc2; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((fp[(vsrc1)]) / (int16_t) vsrc2)); }
                {
                    (pc += 2);
                    break;
                };
            case OP_REM_INT_LIT16:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                vsrc2 = (pc[(1)]);
                ((void) 0);
                if (2 != 0) {
                    int32_t firstVal, result;
                    firstVal = (fp[(vsrc1)]);
                    if ((int16_t) vsrc2 == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && ((int16_t) vsrc2) == -1) {
                        if (2 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal % (int16_t) vsrc2; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((fp[(vsrc1)]) % (int16_t) vsrc2)); }
                {
                    (pc += 2);
                    break;
                };
            case OP_AND_INT_LIT16:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                vsrc2 = (pc[(1)]);
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, result;
                    firstVal = (fp[(vsrc1)]);
                    if ((int16_t) vsrc2 == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && ((int16_t) vsrc2) == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal & (int16_t) vsrc2; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((fp[(vsrc1)]) & (int16_t) vsrc2)); }
                {
                    (pc += 2);
                    break;
                };
            case OP_OR_INT_LIT16:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                vsrc2 = (pc[(1)]);
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, result;
                    firstVal = (fp[(vsrc1)]);
                    if ((int16_t) vsrc2 == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && ((int16_t) vsrc2) == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal | (int16_t) vsrc2; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((fp[(vsrc1)]) | (int16_t) vsrc2)); }
                {
                    (pc += 2);
                    break;
                };
            case OP_XOR_INT_LIT16:
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                vsrc2 = (pc[(1)]);
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, result;
                    firstVal = (fp[(vsrc1)]);
                    if ((int16_t) vsrc2 == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && ((int16_t) vsrc2) == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal ^ (int16_t) vsrc2; }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((fp[(vsrc1)]) ^ (int16_t) vsrc2)); }
                {
                    (pc += 2);
                    break;
                };
            case OP_ADD_INT_LIT8: {
                u_int16_t litInfo;
                vdst = ((inst) >> 8);
                litInfo = (pc[(1)]);
                vsrc1 = litInfo & 0xff;
                vsrc2 = litInfo >> 8;
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, result;
                    firstVal = (fp[(vsrc1)]);
                    if ((s1) vsrc2 == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && ((s1) vsrc2) == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal + ((s1) vsrc2); }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) + (s1) vsrc2)); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_RSUB_INT_LIT8: {
                u_int16_t litInfo;
                vdst = ((inst) >> 8);
                litInfo = (pc[(1)]);
                vsrc1 = litInfo & 0xff;
                vsrc2 = litInfo >> 8;
                ((void) 0);
                (fp[(vdst)] = ((s1) vsrc2 - (int32_t) (fp[(vsrc1)])));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_MUL_INT_LIT8: {
                u_int16_t litInfo;
                vdst = ((inst) >> 8);
                litInfo = (pc[(1)]);
                vsrc1 = litInfo & 0xff;
                vsrc2 = litInfo >> 8;
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, result;
                    firstVal = (fp[(vsrc1)]);
                    if ((s1) vsrc2 == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && ((s1) vsrc2) == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal * ((s1) vsrc2); }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) * (s1) vsrc2)); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_DIV_INT_LIT8: {
                u_int16_t litInfo;
                vdst = ((inst) >> 8);
                litInfo = (pc[(1)]);
                vsrc1 = litInfo & 0xff;
                vsrc2 = litInfo >> 8;
                ((void) 0);
                if (1 != 0) {
                    int32_t firstVal, result;
                    firstVal = (fp[(vsrc1)]);
                    if ((s1) vsrc2 == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && ((s1) vsrc2) == -1) {
                        if (1 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal / ((s1) vsrc2); }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) / (s1) vsrc2)); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_REM_INT_LIT8: {
                u_int16_t litInfo;
                vdst = ((inst) >> 8);
                litInfo = (pc[(1)]);
                vsrc1 = litInfo & 0xff;
                vsrc2 = litInfo >> 8;
                ((void) 0);
                if (2 != 0) {
                    int32_t firstVal, result;
                    firstVal = (fp[(vsrc1)]);
                    if ((s1) vsrc2 == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && ((s1) vsrc2) == -1) {
                        if (2 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal % ((s1) vsrc2); }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) % (s1) vsrc2)); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_AND_INT_LIT8: {
                u_int16_t litInfo;
                vdst = ((inst) >> 8);
                litInfo = (pc[(1)]);
                vsrc1 = litInfo & 0xff;
                vsrc2 = litInfo >> 8;
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, result;
                    firstVal = (fp[(vsrc1)]);
                    if ((s1) vsrc2 == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && ((s1) vsrc2) == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal & ((s1) vsrc2); }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) & (s1) vsrc2)); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_OR_INT_LIT8: {
                u_int16_t litInfo;
                vdst = ((inst) >> 8);
                litInfo = (pc[(1)]);
                vsrc1 = litInfo & 0xff;
                vsrc2 = litInfo >> 8;
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, result;
                    firstVal = (fp[(vsrc1)]);
                    if ((s1) vsrc2 == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && ((s1) vsrc2) == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal | ((s1) vsrc2); }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) | (s1) vsrc2)); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_XOR_INT_LIT8: {
                u_int16_t litInfo;
                vdst = ((inst) >> 8);
                litInfo = (pc[(1)]);
                vsrc1 = litInfo & 0xff;
                vsrc2 = litInfo >> 8;
                ((void) 0);
                if (0 != 0) {
                    int32_t firstVal, result;
                    firstVal = (fp[(vsrc1)]);
                    if ((s1) vsrc2 == 0) {
                        (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                        dvmThrowException("Ljava/lang/ArithmeticException;", "divide by zero");
                        goto exceptionThrown;;
                    }
                    if ((u_int32_t) firstVal == 0x80000000 && ((s1) vsrc2) == -1) {
                        if (0 == 1)
                            result = firstVal;
                        else result = 0;
                    } else { result = firstVal ^ ((s1) vsrc2); }
                    (fp[(vdst)] = (result));
                } else { (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) ^ (s1) vsrc2)); }
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SHL_INT_LIT8: {
                u_int16_t litInfo;
                vdst = ((inst) >> 8);
                litInfo = (pc[(1)]);
                vsrc1 = litInfo & 0xff;
                vsrc2 = litInfo >> 8;
                ((void) 0);
                (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) << (vsrc2 & 0x1f)));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_SHR_INT_LIT8: {
                u_int16_t litInfo;
                vdst = ((inst) >> 8);
                litInfo = (pc[(1)]);
                vsrc1 = litInfo & 0xff;
                vsrc2 = litInfo >> 8;
                ((void) 0);
                (fp[(vdst)] = ((int32_t) (fp[(vsrc1)]) >> (vsrc2 & 0x1f)));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_USHR_INT_LIT8: {
                u_int16_t litInfo;
                vdst = ((inst) >> 8);
                litInfo = (pc[(1)]);
                vsrc1 = litInfo & 0xff;
                vsrc2 = litInfo >> 8;
                ((void) 0);
                (fp[(vdst)] = ((u_int32_t) (fp[(vsrc1)]) >> (vsrc2 & 0x1f)));
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_UNUSED_E3:
            case OP_UNUSED_E4:
            case OP_UNUSED_E5:
            case OP_UNUSED_E6:
            case OP_UNUSED_E7:
            case OP_UNUSED_E8:
            case OP_UNUSED_E9:
            case OP_UNUSED_EA:
            case OP_UNUSED_EB:
            case OP_UNUSED_EC:
            case OP_UNUSED_ED:
            case OP_EXECUTE_INLINE: {
                u8 arg0, arg1, arg2, arg3;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                vdst = (pc[(2)]);
                ((void) 0);
                assert((vdst >> 16) == 0);
                assert(vsrc1 <= 4);
                switch (vsrc1) {
                    case 4:
                        arg3 = (fp[(vdst >> 12)]);
                    case 3:
                        arg2 = (fp[((vdst & 0x0f00) >> 8)]);
                    case 2:
                        arg1 = (fp[((vdst & 0x00f0) >> 4)]);
                    case 1:
                        arg0 = (fp[(vdst & 0x0f)]);
                    default:;
                }
                if (!dvmPerformInlineOp4Std(arg0, arg1, arg2, arg3, &retval, ref))
                    goto exceptionThrown;;
            }
                {
                    (pc += 3);
                    break;
                };
            case OP_UNUSED_EF:
            case OP_INVOKE_DIRECT_EMPTY:
                if (!gDvm.debuggerActive) {
                    {
                        (pc += 3);
                        break;
                    };
                } else {
                    do {
                        methodCallRange = false;
                        goto invokeDirect;
                    } while (false);
                }
            case OP_UNUSED_F1:
            case OP_IGET_QUICK: {
                Object *obj;
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNullExportPC(obj, fp, pc)) goto exceptionThrown;;
                (fp[(vdst)] = (dvmGetFieldInt(obj, ref)));
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IGET_WIDE_QUICK: {
                Object *obj;
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNullExportPC(obj, fp, pc)) goto exceptionThrown;;
                putLongToArray(fp, (vdst), (dvmGetFieldLong(obj, ref)));
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IGET_OBJECT_QUICK: {
                Object *obj;
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNullExportPC(obj, fp, pc)) goto exceptionThrown;;
                (fp[(vdst)] = (u8) (dvmGetFieldObject(obj, ref)));
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IPUT_QUICK: {
                Object *obj;
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNullExportPC(obj, fp, pc)) goto exceptionThrown;;
                dvmSetFieldInt(obj, ref, (fp[(vdst)]));
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IPUT_WIDE_QUICK: {
                Object *obj;
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNullExportPC(obj, fp, pc)) goto exceptionThrown;;
                dvmSetFieldLong(obj, ref, getLongFromArray(fp, (vdst)));
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_IPUT_OBJECT_QUICK: {
                Object *obj;
                vdst = (((inst) >> 8) & 0x0f);
                vsrc1 = ((inst) >> 12);
                ref = (pc[(1)]);
                ((void) 0);
                obj = (Object *) (fp[(vsrc1)]);
                if (!checkForNullExportPC(obj, fp, pc)) goto exceptionThrown;;
                dvmSetFieldObject(obj, ref, ((Object *) fp[(vdst)]));
                ((void) 0);
            }
                {
                    (pc += 2);
                    break;
                };
            case OP_INVOKE_VIRTUAL_QUICK:
                do {
                    methodCallRange = false;
                    goto invokeVirtualQuick;
                } while (false);
            case OP_INVOKE_VIRTUAL_QUICK_RANGE:
                do {
                    methodCallRange = true;
                    goto invokeVirtualQuick;
                } while (false);
            case OP_INVOKE_SUPER_QUICK:
                do {
                    methodCallRange = false;
                    goto invokeSuperQuick;
                } while (false);
            case OP_INVOKE_SUPER_QUICK_RANGE:
                do {
                    methodCallRange = true;
                    goto invokeSuperQuick;
                } while (false);
            case OP_UNUSED_FC:
            case OP_UNUSED_FD:
            case OP_UNUSED_FE:
            case OP_UNUSED_FF:
                LOGE("unknown opcode 0x%02x\n", ((inst) & 0xff));
                dvmAbort();
                {
                    (pc += 1);
                    break;
                };
            filledNewArray:
            {
                ClassObject *arrayClass;
                ArrayObject *newArray;
                u8 *contents;
                char typeCh;
                int i;
                u_int32_t arg5;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                ref = (pc[(1)]);
                vdst = (pc[(2)]);
                if (methodCallRange) {
                    vsrc1 = ((inst) >> 8);
                    arg5 = -1;
                    ((void) 0);
                } else {
                    arg5 = (((inst) >> 8) & 0x0f);
                    vsrc1 = ((inst) >> 12);
                    ((void) 0);
                }
                arrayClass = dvmDexGetResolvedClass(methodClassDex, ref);
                if (arrayClass == NULL) {
                    arrayClass = dvmResolveClass(curMethod->clazz, ref, false);
                    if (arrayClass == NULL)
                        goto exceptionThrown;;
                }
                assert(dvmIsArrayClass(arrayClass));
                assert(dvmIsClassInitialized(arrayClass));
                        LOGVV("+++ filled-new-array type is '%s'\n", arrayClass->descriptor);
                typeCh = arrayClass->descriptor[1];
                if (typeCh == 'D' || typeCh == 'J') {
                    dvmThrowException("Ljava/lang/RuntimeError;",
                                      "bad filled array req");
                    goto exceptionThrown;;
                } else if (typeCh != 'L' && typeCh != '[' && typeCh != 'I') {
                    LOGE("non-int primitives not implemented\n");
                    dvmThrowException("Ljava/lang/InternalError;",
                                      "filled-new-array not implemented for anything but 'int'");
                    goto exceptionThrown;;
                }
                newArray = dvmAllocArrayByClass(arrayClass, vsrc1, ALLOC_DONT_TRACK);
                if (newArray == NULL)
                    goto exceptionThrown;;
                contents = (u8 *) newArray->contents;
                if (methodCallRange) {
                    for (i = 0; i < vsrc1; i++)
                        contents[i] = (fp[(vdst + i)]);
                } else {
                    assert(vsrc1 <= 5);
                    if (vsrc1 == 5) {
                        contents[4] = (fp[(arg5)]);
                        vsrc1--;
                    }
                    for (i = 0; i < vsrc1; i++) {
                        contents[i] = (fp[(vdst & 0x0f)]);
                        vdst >>= 4;
                    }
                }
                retval.l = newArray;
            }
                {
                    (pc += 3);
                    break;
                };
            invokeVirtual:
            {
                Method *baseMethod;
                Object *thisPtr;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vsrc1 = ((inst) >> 8);
                ref = (pc[(1)]);
                vdst = (pc[(2)]);
                if (methodCallRange) {
                    assert(vsrc1 > 0);
                    ((void) 0);
                    thisPtr = (Object *) (fp[(vdst)]);
                } else {
                    assert((vsrc1 >> 4) > 0);
                    ((void) 0);
                    LOGD("[-] fp=%p, fp addr offset=%d\n", fp, (vdst & 0x0f));
                    thisPtr = (Object *) (fp[(vdst & 0x0f)]);
                }
                if (!checkForNull(thisPtr))
                    goto exceptionThrown;;
                baseMethod = dvmDexGetResolvedMethod(methodClassDex, ref);
                if (baseMethod == NULL) {
                    baseMethod = dvmResolveMethod(curMethod->clazz, ref, METHOD_VIRTUAL);
                    if (baseMethod == NULL) {
                        ((void) 0);
                        goto exceptionThrown;;
                    }
                }
                assert(baseMethod->methodIndex < thisPtr->clazz->vtableCount);
                methodToCall = thisPtr->clazz->vtable[baseMethod->methodIndex];
                assert(!dvmIsAbstractMethod(methodToCall) ||
                       methodToCall->nativeFunc != NULL);
                        LOGVV("+++ base=%s.%s virtual[%d]=%s.%s\n",
                              baseMethod->clazz->descriptor, baseMethod->name,
                              (u_int32_t) baseMethod->methodIndex,
                              methodToCall->clazz->descriptor, methodToCall->name);
                assert(methodToCall != NULL);
                goto invokeMethod;;
            }
            invokeSuper:
            {
                Method *baseMethod;
                u_int16_t thisReg;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vsrc1 = ((inst) >> 8);
                ref = (pc[(1)]);
                vdst = (pc[(2)]);
                if (methodCallRange) {
                    ((void) 0);
                    thisReg = vdst;
                } else {
                    ((void) 0);
                    thisReg = vdst & 0x0f;
                }
                if (!checkForNull((Object *) (fp[(thisReg)])))
                    goto exceptionThrown;;
                baseMethod = dvmDexGetResolvedMethod(methodClassDex, ref);
                if (baseMethod == NULL) {
                    baseMethod = dvmResolveMethod(curMethod->clazz, ref, METHOD_VIRTUAL);
                    if (baseMethod == NULL) {
                        ((void) 0);
                        goto exceptionThrown;;
                    }
                }
                if (baseMethod->methodIndex >= curMethod->clazz->super->vtableCount) {
                    dvmThrowException("Ljava/lang/NoSuchMethodError;",
                                      baseMethod->name);
                    goto exceptionThrown;;
                }
                methodToCall = curMethod->clazz->super->vtable[baseMethod->methodIndex];
                assert(!dvmIsAbstractMethod(methodToCall) ||
                       methodToCall->nativeFunc != NULL);
                        LOGVV("+++ base=%s.%s super-virtual=%s.%s\n",
                              baseMethod->clazz->descriptor, baseMethod->name,
                              methodToCall->clazz->descriptor, methodToCall->name);
                assert(methodToCall != NULL);
                goto invokeMethod;;
            }
            invokeInterface:
            {
                Object *thisPtr;
                ClassObject *thisClass;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vsrc1 = ((inst) >> 8);
                ref = (pc[(1)]);
                vdst = (pc[(2)]);
                if (methodCallRange) {
                    assert(vsrc1 > 0);
                    ((void) 0);
                    thisPtr = (Object *) (fp[(vdst)]);
                } else {
                    assert((vsrc1 >> 4) > 0);
                    ((void) 0);
                    thisPtr = (Object *) (fp[(vdst & 0x0f)]);
                }
                if (!checkForNull(thisPtr))
                    goto exceptionThrown;;
                thisClass = thisPtr->clazz;
                methodToCall = dvmFindInterfaceMethodInCache(thisClass, ref, curMethod,
                                                             methodClassDex);
                if (methodToCall == NULL) {
                    assert(dvmCheckException(self));
                    goto exceptionThrown;;
                }
                goto invokeMethod;;
            }
            invokeDirect:
            {
                u_int16_t thisReg;
                vsrc1 = ((inst) >> 8);
                ref = (pc[(1)]);
                vdst = (pc[(2)]);
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                if (methodCallRange) {
                    ((void) 0);
                    thisReg = vdst;
                } else {
                    ((void) 0);
                    thisReg = vdst & 0x0f;
                }
                if (!checkForNull((Object *) (fp[(thisReg)])))
                    goto exceptionThrown;;
                methodToCall = dvmDexGetResolvedMethod(methodClassDex, ref);
                if (methodToCall == NULL) {
                    methodToCall = dvmResolveMethod(curMethod->clazz, ref,
                                                    METHOD_DIRECT);
                    if (methodToCall == NULL) {
                        ((void) 0);
                        goto exceptionThrown;;
                    }
                }
                goto invokeMethod;;
            }
            invokeStatic:
                vsrc1 = ((inst) >> 8);
                ref = (pc[(1)]);
                vdst = (pc[(2)]);
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                if (methodCallRange)
                    ((void) 0);
                else
                    ((void) 0);
                methodToCall = dvmDexGetResolvedMethod(methodClassDex, ref);
                if (methodToCall == NULL) {
                    methodToCall = dvmResolveMethod(curMethod->clazz, ref, METHOD_STATIC);
                    if (methodToCall == NULL) {
                        ((void) 0);
                        goto exceptionThrown;;
                    }
                }
                goto invokeMethod;;
            invokeVirtualQuick:
            {
                Object *thisPtr;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vsrc1 = ((inst) >> 8);
                ref = (pc[(1)]);
                vdst = (pc[(2)]);
                if (methodCallRange) {
                    assert(vsrc1 > 0);
                    ((void) 0);
                    thisPtr = (Object *) (fp[(vdst)]);
                } else {
                    assert((vsrc1 >> 4) > 0);
                    ((void) 0);
                    thisPtr = (Object *) (fp[(vdst & 0x0f)]);
                }
                if (!checkForNull(thisPtr))
                    goto exceptionThrown;;
                assert(ref < thisPtr->clazz->vtableCount);
                methodToCall = thisPtr->clazz->vtable[ref];
                assert(!dvmIsAbstractMethod(methodToCall) ||
                       methodToCall->nativeFunc != NULL);
                        LOGVV("+++ virtual[%d]=%s.%s\n",
                              ref, methodToCall->clazz->descriptor, methodToCall->name);
                assert(methodToCall != NULL);
                goto invokeMethod;;
            }
            invokeSuperQuick:
            {
                u_int16_t thisReg;
                (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc);
                vsrc1 = ((inst) >> 8);
                ref = (pc[(1)]);
                vdst = (pc[(2)]);
                if (methodCallRange) {
                    ((void) 0);
                    thisReg = vdst;
                } else {
                    ((void) 0);
                    thisReg = vdst & 0x0f;
                }
                if (!checkForNull((Object *) (fp[(thisReg)])))
                    goto exceptionThrown;;
                assert(ref < curMethod->clazz->super->vtableCount);
                methodToCall = curMethod->clazz->super->vtable[ref];
                assert(!dvmIsAbstractMethod(methodToCall) ||
                       methodToCall->nativeFunc != NULL);
                        LOGVV("+++ super-virtual[%d]=%s.%s\n",
                              ref, methodToCall->clazz->descriptor, methodToCall->name);
                assert(methodToCall != NULL);
                goto invokeMethod;;
            }
            returnFromMethod:
            {
                StackSaveArea *saveArea;
                {
                    dvmCheckSuspendQuick(self);
                    if ((false)) {
                        (pc += 0);
                        interpState->entryPoint = kInterpEntryReturn;
                                LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                      (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryReturn), (0));
                        goto bail_switch;;
                    }
                };
                ((void) 0);
                LOGD("[-] return from %p", fp);
                saveArea = SAVEAREA_FROM_FP(fp);
                fp = saveArea->prevFrame;
                LOGD(" to %p, current method addr=%p\n", fp, SAVEAREA_FROM_FP(fp)->method);
                assert(fp != NULL);
                if (dvmIsBreakFrame(fp)) {
                            LOGVV("+++ returned into break frame(std\n");
                            LOGVV("+++ next frame addr=%p\n", SAVEAREA_FROM_FP(fp)->prevFrame);
                    goto bail;;
                }
                self->curFrame = fp;
                curMethod = SAVEAREA_FROM_FP(fp)->method;
                LOGD("current fp method=%s\n", curMethod->name);
                if(strcmp("findMethodByName", curMethod->name) == 0) {
                    LOGD("[-] findMethodByName\n");
                }
                methodClassDex = curMethod->clazz->pDvmDex;
                pc = saveArea->savedPc;
                ((void) 0);
                if (true) {
                    {
                        (pc += 3);
                        break;
                    };
                } else {
                    assert(false);
                }
            }
            exceptionThrown:
            {
                Object *exception;
                int catchRelPc;
                {
                    dvmCheckSuspendQuick(self);
                    if ((false)) {
                        (pc += 0);
                        interpState->entryPoint = kInterpEntryThrow;
                                LOGVV("threadid=%d: switch to %s ep=%d adj=%d\n", self->threadId,
                                      (interpState->nextMode == INTERP_STD) ? "STD" : "DBG", (kInterpEntryThrow), (0));
                        goto bail_switch;;
                    }
                };
                assert(dvmCheckException(self));
                exception = dvmGetException(self);
                dvmAddTrackedAlloc(exception, self);
                dvmClearException(self);
                LOGV("Handling exception %s at %s:%d\n",
                     exception->clazz->descriptor, curMethod->name,
                     dvmLineNumFromPC(curMethod, pc - curMethod->insns));
                catchRelPc = dvmFindCatchBlock(self, pc - curMethod->insns,
                                               exception, false, (void *) &fp);
                if (self->stackOverflowed)
                    dvmCleanupStackOverflow(self);
                if (catchRelPc < 0) {
                    dvmSetException(self, exception);
                    dvmReleaseTrackedAlloc(exception, self);
                    goto bail;;
                }
                curMethod = SAVEAREA_FROM_FP(fp)->method;
                methodClassDex = curMethod->clazz->pDvmDex;
                pc = curMethod->insns + catchRelPc;
                ((void) 0);
                ((void) 0);
                if ((((pc[(0)])) & 0xff) == OP_MOVE_EXCEPTION)
                    dvmSetException(self, exception);
                dvmReleaseTrackedAlloc(exception, self);
                {
                    (pc += 0);
                    break;
                };
            }
            invokeMethod:
            { ;
                u8 *outs;
                int i;
                if (methodCallRange) {
                    assert(vsrc1 <= curMethod->outsSize);
                    assert(vsrc1 == methodToCall->insSize);
                    outs = OUTS_FROM_FP(fp, vsrc1);
                    for (i = 0; i < vsrc1; i++)
                        outs[i] = (fp[(vdst + i)]);
                } else {
                    u_int32_t count = vsrc1 >> 4;
                    assert(count <= curMethod->outsSize);
                    assert(count == methodToCall->insSize);
                    assert(count <= 5);
                    outs = OUTS_FROM_FP(fp, count);
                    assert((vdst >> 16) == 0);
                    switch (count) {
                        case 5:
                            outs[4] = (fp[(vsrc1 & 0x0f)]);
                        case 4:
                            outs[3] = (fp[(vdst >> 12)]);
                        case 3:
                            outs[2] = (fp[((vdst & 0x0f00) >> 8)]);
                        case 2:
                            outs[1] = (fp[((vdst & 0x00f0) >> 4)]);
                        case 1:
                            outs[0] = (fp[(vdst & 0x0f)]);
                        default:;
                    }
                }
            }
                {
                    StackSaveArea *newSaveArea;
                    u8 *newFp;
                    ((void) 0);
                    newFp = ((u8 *) SAVEAREA_FROM_FP(fp)) - methodToCall->registersSize;
                    LOGD("[+] new fp from %p to %p, old fp method addr=%p, new fp method addr=%p\n", fp, newFp,
                         SAVEAREA_FROM_FP(fp)->method, methodToCall);
                    LOGD("[+] reg size=%d\n", methodToCall->registersSize);
                    newSaveArea = SAVEAREA_FROM_FP(newFp);
                    if (true) {
                        u_int8_t *bottom;
                        bottom = (u_int8_t *) newSaveArea - methodToCall->outsSize * sizeof(u8);
                        if (bottom < self->interpStackEnd) {
                            LOGV("Stack overflow on method call (start=%p end=%p newBot=%p size=%d '%s')\n",
                                 self->interpStackStart, self->interpStackEnd, bottom,
                                 self->interpStackSize, methodToCall->name);
                            dvmHandleStackOverflow(self);
                            assert(dvmCheckException(self));
                            goto exceptionThrown;;
                        }
                    }
                    newSaveArea->prevFrame = fp;
                    newSaveArea->savedPc = pc;
                    newSaveArea->method = methodToCall;
                    if (!dvmIsNativeMethod(methodToCall)) {
                        curMethod = methodToCall;
                        methodClassDex = curMethod->clazz->pDvmDex;
                        pc = methodToCall->insns;
                        fp = self->curFrame = newFp;
                        ((void) 0);
                        ((void) 0);
                        {
                            (pc += 0);
                            break;
                        };
                    } else {
                        newSaveArea->xtra.localRefTop = self->jniLocalRefTable.nextEntry;
                        self->curFrame = newFp;
                        ((void) 0);
                        ((void) 0);
                        (*methodToCall->nativeFunc)(newFp, &retval, methodToCall, self);
                        dvmPopJniLocals(self, newSaveArea);
                        self->curFrame = fp;
                        if (dvmCheckException(self)) {
                            LOGV("Exception thrown by/below native code\n");
                            goto exceptionThrown;;
                        }
                        ((void) 0);
                        ((void) 0);
                        if (true) {
                            {
                                (pc += 3);
                                break;
                            };
                        } else {
                            assert(false);
                        }
                    }
                }
                assert(false);
        }
    }
    bail:
    ((void) 0);
    interpState->retval = retval;
    return false;
    bail_switch:
    interpState->method = curMethod;
    interpState->pc = pc;
    interpState->fp = fp;
    interpState->retval = retval;
    interpState->nextMode =
            (INTERP_STD == INTERP_STD) ? INTERP_DBG : INTERP_STD;
            LOGVV(" meth='%s.%s' pc=0x%lx fp=%p\n",
                  curMethod->clazz->descriptor, curMethod->name,
                  pc - curMethod->insns, fp);
    return true;
}
