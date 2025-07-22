#include <stdio.h>
#include "vm.h"
#include "asserts.h"
#include "bytecode.h"

// -- VM --

typedef struct {
    size_t programCounter;
    ByteDa byteCode;
} VmState;

VmState vmState = {0};

static bool IsDone() {
    return vmState.programCounter >= vmState.byteCode.count;
}

static OpCode ConsumeOpCode() {
    return vmState.byteCode.items[vmState.programCounter++];
}

static VmResult CreateError(const char* message) {
    VmResult result = {
        .type = VM_ERROR,
        .as.error = {
            .message = MakeString(message),
        }
    };
    return result;
}

static VmResult CreateSuccess() {
    VmResult result = {
        .type = VM_SUCCESS,
        .as.success = {},
    };
    return result;
}

VmResult ExecuteByteCode(ByteDa byteCode, Allocator* allocator) {
    vmState = (VmState) { .programCounter = 0, .byteCode = byteCode };

    size_t i = 0;
    size_t guard = 1337;

    while (!IsDone() && i++ < guard) {
        OpCode op = ConsumeOpCode();

        switch (op) {
        default: return CreateError("Unsupported op code.");
        }
    }

    Assert(IsDone(), "Program ended before the program counter reached the end.");

    return CreateSuccess();
}

// -- Printing --

const char* MapVmResultTypeToStr(VmResultType type) {
    switch(type) {
    case VM_SUCCESS: return "VM_SUCCESS";
    case VM_ERROR: return "VM_ERROR";
    default: return NULL;
    }
}

void PrintVmResult(VmResult vmResult) {
    if (vmResult.type == VM_ERROR) {
        VmResultError error = vmResult.as.error;
        fprintf(stderr, "Bytecode execution error: ");
        PrintStringErr(error.message);
        fprintf(stderr, "\n");
        return;
    }
    printf("Executed bytecode successfully\n");
}
