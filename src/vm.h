#ifndef vm_h
#define vm_h

#include "memory.h"
#include "common.h"
#include "values.h"

typedef struct {
    ValueDa values;
} VmSuccess;

typedef struct {
    String message;
} VmError;

typedef struct {
    ResultType type;
    union {
        VmSuccess success;
        VmError error;
    } as;
} VmResult;

VmResult ExecuteByteCode(ByteDa byteCode, Allocator* allocator);

void PrintVmResult(VmResult vmResult);

#endif
