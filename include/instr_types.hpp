#ifndef _INSTR_TYPES_HPP
#define _INSTR_TYPES_HPP

typedef enum INSTR_TYPES {
    ADD = 0,     //0
    SUB,         //1
    MULT,        //2
    DIV,         //3
    EQ,          //4
    IF,          //5
    LIST,        //6
    FLOAT_CONST, //7
    INT_CONST,   //8
    FRAC_CONST,  //9
    VAR_CONST,   //10
    TOTAL_INSTR_TYPES
} INSTR_TYPES;

#endif
