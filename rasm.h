#ifndef RM_H_
#define RM_H_

#include<stdio.h>
#include<assert.h>
#include<stdbool.h>
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

    // INST_PLUSI,
    // INST_SUBI,
    // INST_MULI,
    // INST_DIVI,
} Inst_Type;

typedef struct {
    Inst_Type  inst_type;
    uint64_t   inst_operand;
} Inst;

const char* inst_to_cstr(Inst_Type type);

#define MAKE_INST_NOP		{ .inst_type = INST_NOP }
#define MAKE_INST_HALT		{ .inst_type = INST_HALT }
#define MAKE_INST_PLUSI		{ .inst_type = INST_PLUSI }
#define MAKE_INST_PUSH(value)	{ .inst_type = INST_PUSH, .inst_operand = value }

typedef struct {
    int64_t stack[RM_STACK_CAPACITY];    
    uint64_t rm_stack_size;
    
    Inst program[RM_PROGRAM_CAPACITY];
    uint64_t rm_program_size;
    uint64_t ip;

    bool halt;
} Rm;

void rm_dump_stack(FILE *stream, Rm *rm);
void rm_execute_program(Rm *rm);

#endif // RM_H_

#ifdef RM_IMPLEMENTATION

const char* inst_to_cstr(Inst_Type type) {
    switch(type) {
    case INST_NOP: return "INST_NOP";
    case INST_HALT: return "INST_HALT";
    case INST_PUSH: return "INST_PUSH";
    // case INST_PLUSI: return "INST_PLUSI";
    // case INST_SUBI: return "INST_SUBI";
    // case INST_MULI: return "INST_MULI";
    // case INST_DIVI: return "INST_DIVI";
default:
    return "Unknown type";
    }
}

void rm_dump_stack(FILE *stream, Rm *rm) {
    fprintf(stream, "Stack:\n");
    if(rm->rm_stack_size > 0) {
	for(size_t i = 0; i < rm->rm_stack_size; ++i) {
	   fprintf(stream, "    %lld\n", rm->stack[i]);
	}
    } else {	
	fprintf(stream, "[empty]\n");
    }   
}


void rm_execute_program(Rm *rm) {
    rm->ip = 0;
    for(size_t i = 0; i < rm->rm_program_size; ++i) {
	Inst inst = rm->program[i];
	switch(inst.inst_type) {
	case INST_NOP: {
	    rm->ip += 1;
	} break;
	
	case INST_HALT: {	  
	    rm->halt = true;	    
	    rm->ip += 1;	    
	} break;	

	case INST_PUSH: {	    
	    rm->stack[rm->ip] = (int64_t)inst.inst_operand;
	    rm->ip += 1;
	    rm->rm_stack_size += 1;
	} break;

    default:	
	    printf("Unknown Instruction\n");
	}
    }
}



#endif // RM_IMPLEMENTATION
