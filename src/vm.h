#ifndef vm_h
#define vm_h

#include "da.h"
#include "memory.h"
#include "stringz.h"

typedef enum {
    VM_SUCCESS,
    VM_ERROR,
} VmResultType;

typedef struct {
} VmResultSuccess;

typedef struct {
    String message;
} VmResultError;

typedef struct {
    VmResultType type;
    union {
        VmResultSuccess success;
        VmResultError error;
    } as;
} VmResult;

VmResult ExecuteByteCode(ByteDa byteCode, Allocator* allocator);

const char* MapVmResultTypeToStr(VmResultType type);
void PrintVmResult(VmResult vmResult);

#endif
