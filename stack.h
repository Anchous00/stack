#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include "colors.h"

#ifdef STKDEBUG
#define ONDEBUG(...) __VA_ARGS__
#else
#define ONDEBUG(...)
#endif


typedef double stackelem_t;
const double EPSILON = 1e-6;
const stackelem_t POIZON = NAN;

const int TEST_AMOUNT = 1000000;

enum return_code
{
    ERROR = 1,
    SUCCESS = 0
};

ONDEBUG(
        const stackelem_t LEFT_DATA_CANARY    = 0xE1DADEDA;
        const stackelem_t RIGHT_DATA_CANARY   = 0xEBA1DEDA;
        const stackelem_t LEFT_STRUCT_CANARY  = 0xB1BA;
        const stackelem_t RIGHT_STRUCT_CANARY = 0xB0BA;

        enum stack_err_code_t
        {
            STACK_OK                 =  1,
            NULL_STACK               =  0,
            STACK_DESTROYED          = -1,
            NULL_STACK_DATA          = -2,
            NEGATIVE_STACK_SIZE      = -3,
            NEGATIVE_STACK_CAPACITY  = -4,
            LEFT_DATA_CANARY_DEAD    = -5,
            RIGHT_DATA_CANARY_DEAD   = -6,
            LEFT_STRUCT_CANARY_DEAD  = -7,
            RIGHT_STRUCT_CANARY_DEAD = -8,
            WRONG_DATA_HASH = -9
        };
        )

struct stack_t
{
    ONDEBUG(
            stackelem_t left_struct_canary;
           )
    stackelem_t *memory;
    stackelem_t *data;
    size_t size;
    size_t capacity;
    ONDEBUG(
            stack_err_code_t err;
            const char *filename;
            const char *name;
            int line;
            uint64_t data_hash;

            stackelem_t right_struct_canary;
           )
};

return_code stack_ctor       ( stack_t *stk, size_t capacity ONDEBUG(, const char *filename, const char *name, int line));
return_code stack_detor      ( stack_t *stk                 );
return_code stack_push       ( stack_t *stk, stackelem_t  x );
return_code stack_pop        ( stack_t *stk, stackelem_t *x );
return_code stack_resize_up  ( stack_t *stk                 );
return_code stack_resize_down( stack_t *stk                 );
void stack_ugabuga_test      ( stack_t *stk );

ONDEBUG(
        void stack_dump              ( stack_t *stk );

        return_code stack_print_error( stack_t *stk );
        stack_err_code_t stack_ok    ( stack_t *stk );

        uint64_t stack_count_data_hash( stack_t *stk );
        uint64_t djb2( void *ptr, size_t size );
        bool is_zero( stackelem_t x );
)

#endif
