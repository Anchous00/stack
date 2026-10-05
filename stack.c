#include "stack.h"


return_status stack_ctor( stack_t *stk, size_t capacity ONDEBUG(, const char *filename, const char *name, int line ))
{
    if (!is_valid(stk))
        return ERROR;
    if (capacity > SIZE_MAX / 2)
    {
        ONDEBUG(stk->errs = stk->errs | NEGATIVE_STACK_CAPACITY;)
        return ERROR;
    }

    stk->capacity = capacity;
    stk->data = (stackelem_t*)calloc(stk->capacity + 2, sizeof(stackelem_t));
    if (!is_valid(stk->data))
    {
        ONDEBUG(stk->errs = stk->errs | WRONG_STACK_DATA;)
        return ERROR;
    }


    for (size_t i = 1; i < 1 + capacity; i++)
        stk->data[i] = POIZON;
    stk->size = 0;

ONDEBUG(
            stk->data[0]            = LEFT_DATA_CANARY;
            stk->data[1 + stk->capacity] = RIGHT_DATA_CANARY;
            stk->left_struct_canary  = LEFT_STRUCT_CANARY;
            stk->right_struct_canary = RIGHT_STRUCT_CANARY;

            stack_count_data_hash(stk);

            stk->filename = filename;
            stk->name     = name;
            stk->line     = line;

            if ((stk->errs = stack_ok(stk)) != STACK_OK)
                return ERROR;
        )
    return SUCCESS;
}

return_status stack_detor( stack_t *stk )
{
    if (!is_valid(stk))
    {
        return ERROR;
    }

    stk->capacity = 0;
    stk->size = 0;
    free(stk->data);

    ONDEBUG(
            stk->data_hash = 0;
            stk->errs = stk->errs | STACK_DESTROYED;
            )

    return SUCCESS;
}

return_status stack_push( stack_t *stk, stackelem_t x )
{
    if (stack_ok(stk) != STACK_OK)
        return ERROR;

    if (stk->size >= stk->capacity)
        stack_resize_up(stk);
    stk->data[1 + stk->size++] = x;

    ONDEBUG(
            stack_count_data_hash(stk);
            if ((stk->errs = stack_ok(stk)) != STACK_OK)
                return ERROR;
            )
    return SUCCESS;
}

return_status stack_pop( stack_t *stk, stackelem_t *x )
{
    if (stack_ok(stk) != STACK_OK)
    {
        *x = POIZON;
        return ERROR;
    }
    if (stk->size <= 0)
    {
        *x = POIZON;
        ONDEBUG(stk->errs = stk->errs | NEGATIVE_STACK_SIZE;)
        return ERROR;
    }

    if (stk->size == stk->capacity / 4)
        stack_resize_down(stk);

    stackelem_t val = stk->data[1 +--stk->size];
    stk->data[1 + stk->size]= POIZON;

    *x = val;
    ONDEBUG(
            stack_count_data_hash(stk);
            if ((stk->errs = stack_ok(stk)) != STACK_OK)
                return ERROR;
            )

    return SUCCESS;
}

return_status stack_resize_up( stack_t *stk )
{
    if (stack_ok(stk) != STACK_OK)
        return ERROR;
    stk->data = (stackelem_t*)realloc(stk->data,
                                       (stk->capacity * 2 + 2) * sizeof(stackelem_t));

    stk->capacity *= 2;
    for (size_t i = stk->size; i < stk->capacity; i++)
        stk->data[1 + i] = POIZON;

    ONDEBUG(
            stk->data[1 + stk->capacity] = RIGHT_DATA_CANARY;
            if ((stk->errs = stack_ok(stk)) != STACK_OK)
                return ERROR;
            )
    return SUCCESS;
}

return_status stack_resize_down( stack_t *stk )
{
    if (stack_ok(stk) != STACK_OK)
        return ERROR;

    stk->data = (stackelem_t*)realloc(stk->data,
                                       (stk->capacity / 2 + 2) * sizeof(stackelem_t));

    stk->capacity /= 2;

    ONDEBUG(
            stk->data[1 + stk->capacity] = RIGHT_DATA_CANARY;
            if ((stk->errs = stack_ok(stk)) != STACK_OK)
                return ERROR;
            )
    return SUCCESS;
}

errs_t stack_ok( stack_t *stk )
{
    errs_t errs = STACK_OK;
    if (!is_valid(stk))
    {
        errs = errs | WRONG_STACK_PTR;
        return errs;
    }

    ONDEBUG
    (
    if (stk->capacity > SIZE_MAX / 2)
        errs = errs | NEGATIVE_STACK_CAPACITY;

    if (stk->size > SIZE_MAX / 2)
        errs = errs | NEGATIVE_STACK_SIZE;

    if (!is_equal(stk->left_struct_canary, LEFT_STRUCT_CANARY))
        errs = errs | LEFT_STRUCT_CANARY_DEAD;

    if (!is_equal(stk->right_struct_canary, RIGHT_STRUCT_CANARY))
        errs = errs | RIGHT_STRUCT_CANARY_DEAD;
    errs = errs | stack_data_ok(stk);

    return errs;
    )
}

ONDEBUG(
hash_t stack_count_data_hash( stack_t *stk )
{
    return stk->data_hash = djb2(stk->data, (stk->capacity + 2) * sizeof(stackelem_t)); //2 canaries
}

void stack_dump( stack_t *stk )
{
    if (!is_valid(stk))
    {
        printf(RED "ERROR:wrong stack pointer\n" WHITE);
        return;
    }
    stk->errs = stack_ok(stk);
    stack_print_error(stk);

    printf("struct_t %s [%p] created by %s at %s:%d\n"
           "\tcapacity = %zu\n"
           "\tsize = %zu\n"
           "\tdata [%p]\n",
            stk->name,
            stk,
            stk->filename,
            stk->filename,
            stk->line,
            stk->capacity,
            stk->size,
            stk->data + 1);
    if ((stk->errs & WRONG_STACK_DATA) == STACK_OK)
    {
//print left canary
    printf("\t\t[%d] %lg %s\n",
            -1,
            stk->data[0],
            is_equal(stk->data[0], LEFT_DATA_CANARY)
            ?
            YELLOW "canary" WHITE
            :
            RED "DEAD CANARY" WHITE);
//print data
    for (size_t i = 0; i < stk->capacity; i++)
        printf("\t\t[%zu] %lg %s\n",
                i,
                stk->data[i + 1],
                isnan(stk->data[i + 1]) ? RED "poison" WHITE : "");
//print right canary
    printf("\t\t[%zu] %lg %s\n",
            stk->capacity,
            stk->data[1 + stk->capacity],
            is_equal(stk->data[1 + stk->capacity], RIGHT_DATA_CANARY)
            ?
            YELLOW "canary" WHITE
            :
            RED "DEAD CANARY" WHITE);
    }
    printf(BLUE "\t\tdata hash = %lu\n" WHITE, stk->data_hash);

    return;
}

return_status stack_print_error( stack_t *stk )
{
        if (!is_valid(stk))
        {
            printf(RED "error: wrong stack pointer\n" WHITE);
            return ERROR;
        }
    printf("errs: %lu\n", stk->errs);
        if (!(stk->errs | STACK_OK))
        {
            printf(GREEN "correct stack\n" WHITE);
            return SUCCESS;
        }
        if (stk->errs & WRONG_STACK_PTR)
            printf(RED "fatal error: wrong stack pointer\n" WHITE);
        if (stk->errs & STACK_DESTROYED)
            printf(RED "destructed stack\n" WHITE);
        if (stk->errs & WRONG_STACK_DATA)
            printf(RED "wrong stack data pointer\n" WHITE);
        if (stk->errs & NEGATIVE_STACK_SIZE)
            printf(RED "negative stack size\n" WHITE);
        if (stk->errs & NEGATIVE_STACK_CAPACITY)
            printf(RED "negative stack capacity\n" WHITE);
        if (stk->errs & RIGHT_DATA_CANARY_DEAD)
            printf(RED "right data canary is dead, data may be corrupted\n" WHITE);
        if (stk->errs & LEFT_DATA_CANARY_DEAD)
            printf(RED "left data canary is dead, data may be corrupted\n" WHITE);
        if (stk->errs & RIGHT_STRUCT_CANARY_DEAD)
            printf(RED "right stack canary is dead, stack may be corrupted\n" WHITE);
        if (stk->errs & LEFT_STRUCT_CANARY_DEAD)
            printf(RED "left stack canary is dead, stack may be corrupted\n" WHITE);
        if (stk->errs & WRONG_DATA_HASH)
            printf(RED "wrong data hash, data may be corrupted\n" WHITE);

    return SUCCESS;
}

errs_t stack_data_ok( stack_t *stk )
{
    errs_t errs = STACK_OK;
    if (!is_valid(stk))
    {
        errs = errs | WRONG_STACK_PTR;
        return errs;
    }

    if (!is_valid(stk->data))
    {
        errs = errs | WRONG_STACK_DATA;
        return errs;
    }

    if (!is_equal(stk->data[0], LEFT_DATA_CANARY))
        errs = errs | LEFT_DATA_CANARY_DEAD;

    if (!is_equal(stk->data[1 + stk->capacity], RIGHT_DATA_CANARY))
        errs = errs | RIGHT_DATA_CANARY_DEAD;

    stack_count_data_hash(stk);
    if (stk->data_hash != stack_count_data_hash(stk))
        errs = errs | WRONG_DATA_HASH;

    return errs;
}

bool is_equal( stackelem_t x, stackelem_t y )
{
    switch (sizeof(stackelem_t))
    {
        case 1:
            return *(uint8_t*)(&x)  == *(uint8_t*)(&y);
        case 2:
            return *(uint16_t*)(&x) == *(uint16_t*)(&y);
        case 4:
            return *(uint32_t*)(&x) == *(uint32_t*)(&y);
        case 8:
            return *(uint64_t*)(&x) == *(uint64_t*)(&y);
        default:
            return *(uint64_t*)(&x) == *(uint64_t*)(&y);
    }
}

hash_t djb2( void *ptr, size_t size )
{
    hash_t hash = 0xEDA;
    for (size_t i = 0; i < size; i++)
    {
        hash = ((hash << 5) + hash) + *((unsigned char*)ptr + i);
    }

    return hash;
}
)

bool is_valid( const void *p )
{
return p != nullptr && (long long int)p % (long long int)sizeof(stackelem_t) == 0;
#ifdef __linux__
    extern char _etext;
   // printf("linux\n");
    return ((const char*)p - &_etext > 0 &&
            (const char*)((long long)16 * 1024 * 1024 * 1024) - (const char*)p > 0 );
#endif
#ifdef _WIN32
//printf("windows\n");
    return (const char*)p - (const char*)(4 * 1024 * 1024) > 0 &&
           (const char*)((long long)16 * 1024 * 1024 * 1024) - (const char*)p > 0;
#endif
}
