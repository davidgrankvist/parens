#ifndef vm_h
#define vm_h

#include "memory.h"
#include "stringz.h"
#include "values.h"

typedef enum {
    VM_SUCCESS,
    VM_ERROR,
} VmResultType;

typedef struct {
    ValueDa values;
} VmSuccess;

typedef struct {
    String message;
} VmError;

typedef struct {
    VmResultType type;
    union {
        VmSuccess success;
        VmError error;
    } as;
} VmResult;

VmResult ExecuteByteCode(ByteDa byteCode, Allocator* allocator);

const char* MapVmResultTypeToStr(VmResultType type);
void PrintVmResult(VmResult vmResult);

#endif
