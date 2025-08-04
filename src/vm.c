#include <stdio.h>
#include "vm.h"
#include "asserts.h"
#include "da.h"
#include "bytecode.h"

typedef struct {
    size_t programCounter;
    ByteDa byteCode;
    ValueDa values;
} VmState;

VmState vmState = {0};

static void PushValue(Value val) {
    DA_APPEND(&vmState.values, val);
}

static Value PopValue() {
    return DA_POP(&vmState.values);
}

static bool IsDone() {
    return vmState.programCounter >= vmState.byteCode.count;
}

static Byte ConsumeByte() {
    return vmState.byteCode.items[vmState.programCounter++];
}

static Byte* ConsumeBytes(size_t count) {
    Byte* start = &vmState.byteCode.items[vmState.programCounter];
    vmState.programCounter += count;
    return start;
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
        .as.success = {
            .values = vmState.values,
        },
    };
    return result;
}

#define BINARY_OP(o) \
    do { \
        Value v1 = PopValue(); \
        Value v2 = PopValue(); \
        if (v1.type != VALUE_F64 || v2.type != VALUE_F64) { \
            return CreateError("Arithmetic operator failed. Expected F64 values."); \
        } \
        PushValue(MAKE_VALUE_F64(v1.as.f64 o v2.as.f64)); \
    } while(0)

VmResult ExecuteByteCode(ByteDa byteCode, Allocator* allocator) {
    vmState = (VmState) {
        .programCounter = 0,
        .byteCode = byteCode,
        .values = DA_MAKE_DEFAULT(Value),
    };

    size_t i = 0;
    size_t guard = 1337;

    VmResult result = {0};

    while (!IsDone() && i++ < guard) {
        OpCode op = ConsumeByte();

        switch (op) {
            case OP_NIL:
                PushValue(MAKE_VALUE_NIL());
                break;
            case OP_TRUE:
                PushValue(MAKE_VALUE_BOOL(true));
                break;
            case OP_FALSE:
                PushValue(MAKE_VALUE_BOOL(false));
                break;
            case OP_F64: {
                Byte* bytes = ConsumeBytes(8);
                double d = ReadDoubleFromLittleEndian8(bytes);
                PushValue(MAKE_VALUE_F64(d));
                break;
            }
            case OP_BUILTIN_FN: {
                OpCode o = ConsumeByte();
                if (o < OPERATOR_ADD || o > OPERATOR_PRINT) {
                    result = CreateError("Unexpected builtin operator");
                    break;
                }
                PushValue(MAKE_VALUE_OPERATOR(o));
                break;
            }
            case OP_ADD: {
                BINARY_OP(+);
                break;
            }
            case OP_SUBTRACT: {
                BINARY_OP(-);
                break;
            }
            case OP_MULTIPLY: {
                BINARY_OP(*);
                break;
            }
            case OP_DIVIDE: {
                BINARY_OP(/);
                break;
            }
            case OP_NEGATE: {
                Value v = PopValue();
                if (v.type != VALUE_F64) {
                    result = CreateError("Unable to negate. Expected F64 value.");
                    break;
                }
                PushValue(MAKE_VALUE_F64(-v.as.f64));
                break;
            }
            case OP_PRINT: {
                Value v = PopValue();
                PrintValue(v);
                PushValue(MAKE_VALUE_NIL());
                break;
            }
            case OP_CONS_CELL: {
                // TODO(memory): consider freeing, refcounting, etc.
                Value head = PopValue();
                Value tail = PopValue();
                Object* consObj = CreateConsCellObject(head, tail, allocator);
                Value objVal = MAKE_VALUE_OBJECT(consObj);
                PushValue(objVal);
                break;
            }
            default:
                result = CreateError("Unsupported op code.");
                break;
        }
    }

    if (result.type == VM_ERROR) {
        return result;
    } else {
        return CreateSuccess();
    }
}

const char* MapVmResultTypeToStr(VmResultType type) {
    switch(type) {
        case VM_SUCCESS: return "VM_SUCCESS";
        case VM_ERROR: return "VM_ERROR";
        default: return NULL;
    }
}

void PrintVmResult(VmResult vmResult) {
    if (vmResult.type == VM_ERROR) {
        VmError error = vmResult.as.error;
        fprintf(stderr, "Bytecode execution error: ");
        PrintStringErr(error.message);
        fprintf(stderr, "\n");
        return;
    }
    printf("Executed bytecode successfully\n");
}
