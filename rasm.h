#ifndef RM_H_
#define RM_H_

#include<stdio.h>
#include<assert.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<inttypes.h>

#define RM_STACK_CAPACITY 1024
#define RM_PROGRAM_CAPACITY 1024

#define ARRAY_SIZE(arr) sizeof(arr)/sizeof(arr[0])

typedef enum {
    INST_NOP,
    INST_HALT,

    INST_PUSH,
    INST_DUP,
    INST_JMP,

    INST_PLUSI,
    INST_MINUSI,
    INST_MULI,
    INST_DIVI,
    INST_MODI,
} Inst_Type;

typedef struct {
    Inst_Type  inst_type;
    uint64_t   inst_operand;
} Inst;

const char* inst_to_cstr(Inst_Type type);

// * vm error's
typedef enum {
    ERR_STACK_UNDERFLOW = 0,
    ERR_STACK_OVERFLOW,
    ERR_ILLEGAL_INST,
    ERR_OK,
} Err;
const char* err_as_cstr(Err err);

#define MAKE_INST_NOP		{ .inst_type = INST_NOP }
#define MAKE_INST_HALT		{ .inst_type = INST_HALT }
#define MAKE_INST_PUSH(value)	{ .inst_type = INST_PUSH, .inst_operand = value }
#define MAKE_INST_DUP(value)	{ .inst_type = INST_DUP, .inst_operand = value }
#define MAKE_INST_JMP(value)	{ .inst_type = INST_JMP, .inst_operand = value }

#define MAKE_INST_PLUSI		{ .inst_type = INST_PLUSI }
#define MAKE_INST_MINUSI	{ .inst_type = INST_MINUSI }
#define MAKE_INST_DIVI		{ .inst_type = INST_DIVI }
#define MAKE_INST_MODI		{ .inst_type = INST_MODI }

typedef struct {
    int64_t stack[RM_STACK_CAPACITY];    
    uint64_t rm_stack_size;
    
    Inst program[RM_PROGRAM_CAPACITY];
    uint64_t rm_program_size;
    uint64_t ip;

    bool halt;
} Rm;

void rm_dump_stack(FILE *stream, Rm *rm);
Err rm_execute_program(Rm *rm, int limit);
Err rm_execute_inst(Rm *rm);

#endif // RM_H_

#ifdef RM_IMPLEMENTATION

const char* inst_to_cstr(Inst_Type type) {
    switch(type) {
    case INST_NOP: return "INST_NOP";
    case INST_HALT: return "INST_HALT";
    case INST_PUSH: return "INST_PUSH";
    case INST_DUP: return "INST_DUP";
    case INST_JMP: return "INST_JMP";
    
    case INST_PLUSI: return "INST_PLUSI";
    case INST_MINUSI: return "INST_MINUSI";
    case INST_MULI: return "INST_MULI";
    case INST_DIVI: return "INST_DIVI";
    case INST_MODI: return "INST_MODI";
default:
    return "Unknown type";
    }
}

const char* err_as_cstr(Err err) {
    switch(err) {
    case ERR_OK: return "ERR_OK";
    case ERR_ILLEGAL_INST: return "ERR_ILLEGAL_INST";
    case ERR_STACK_OVERFLOW: return "ERR_STACK_OVERFLOW";
    case ERR_STACK_UNDERFLOW: return "ERR_STACK_UNDERFLOW";
    default:
	return "Unknown Err";
    }
}

void rm_dump_stack(FILE *stream, Rm *rm) {
    fprintf(stream, "Stack:\n");
    if(rm->rm_stack_size > 0) {
	for(size_t i = 0; i < rm->rm_stack_size; ++i) {
	   fprintf(stream, "    %"PRIu64"\n", rm->stack[i]);
	}
    } else {	
	fprintf(stream, "[empty]\n");
    }   
}

Err rm_execute_program(Rm *rm, int limit) {
    while(limit != 0 && !rm->halt) {
	Err err = rm_execute_inst(rm);
	if(err != ERR_OK) {
	    printf("ERROR: %s\n", err_as_cstr(err));
	    return err;
	}
	if(limit > 0) {
	    --limit;
	}
    }
    return ERR_OK;
}

Err rm_execute_inst(Rm *rm) {
    if(rm->ip > rm->rm_program_size) {
	return ERR_ILLEGAL_INST;
    }
    
    Inst inst = rm->program[rm->ip];
    switch(inst.inst_type) {
    case INST_NOP: {
	rm->ip += 1;
    } break;

    case INST_HALT: {	  
	rm->halt = true;	    
	rm->ip += 1;	    
    } break;	

    case INST_PUSH: {
	if(rm->rm_stack_size >= RM_STACK_CAPACITY) {
	    return ERR_STACK_OVERFLOW;
	}
	rm->stack[rm->rm_stack_size++] = (int64_t)inst.inst_operand;
	rm->ip += 1;
    } break;

    case INST_DUP: {
	if(rm->rm_stack_size >= RM_STACK_CAPACITY) {
	    return ERR_STACK_OVERFLOW;
	}

	uint64_t pos = inst.inst_operand;
	if(pos >= rm->rm_stack_size) {
	    return ERR_STACK_UNDERFLOW;
	}
	const uint64_t idx = rm->rm_stack_size - 1 - inst.inst_operand;
	rm->stack[rm->rm_stack_size++] = rm->stack[idx];
	rm->ip += 1;
    } break;

    case INST_JMP: {
	rm->ip = inst.inst_operand;	    
    } break;	

    case INST_PLUSI: {
	if(rm->rm_stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	int64_t first_op = (int64_t)rm->stack[rm->rm_stack_size - 2];
	int64_t second_op = (int64_t)rm->stack[rm->rm_stack_size - 1];
	rm->stack[rm->rm_stack_size-2] = first_op + second_op;
	rm->ip += 1;
	rm->rm_stack_size -= 1;
    } break;

    case INST_MINUSI: {
	if(rm->rm_stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	int64_t first_op = (int64_t)rm->stack[rm->rm_stack_size - 2];
	int64_t second_op = (int64_t)rm->stack[rm->rm_stack_size - 1];
	rm->stack[rm->rm_stack_size-2] = first_op - second_op;
	rm->ip += 1;
	rm->rm_stack_size -= 1;
    } break;


    case INST_MULI: {
	if(rm->rm_stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	int64_t first_op = (int64_t)rm->stack[rm->rm_stack_size - 2];
	int64_t second_op = (int64_t)rm->stack[rm->rm_stack_size - 1];
	rm->stack[rm->rm_stack_size-2] = first_op * second_op;
	rm->ip += 1;
	rm->rm_stack_size -= 1;
    } break;


    case INST_DIVI: {
	if(rm->rm_stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	int64_t first_op = (int64_t)rm->stack[rm->rm_stack_size - 2];
	int64_t second_op = (int64_t)rm->stack[rm->rm_stack_size - 1];
	rm->stack[rm->rm_stack_size-2] = first_op / second_op;
	rm->ip += 1;
	rm->rm_stack_size -= 1;
    } break;

    case INST_MODI: {
	if(rm->rm_stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	int64_t first_op = (int64_t)rm->stack[rm->rm_stack_size - 2];
	int64_t second_op = (int64_t)rm->stack[rm->rm_stack_size - 1];
	rm->stack[rm->rm_stack_size-2] = first_op % second_op;
	rm->ip += 1;
	rm->rm_stack_size -= 1;
    } break;	

    default:
    	printf("Unknown Instruction\n");

    }
    return ERR_OK;
}



#endif // RM_IMPLEMENTATION
