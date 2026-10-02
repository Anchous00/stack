#include "stack.h"

#pragma region stack ctor
return_code stack_ctor( stack_t *stk, size_t capacity ONDEBUG(, const char *filename, const char *name, int line))
{
    if (stk == NULL)
        return ERROR;
    if (capacity > SIZE_MAX / 2)
    {
        ONDEBUG(stk->err = NEGATIVE_STACK_CAPACITY;)
        return ERROR;
    }

    stk->capacity = capacity;
    stk->memory = (stackelem_t*)calloc(stk->capacity + 2, sizeof(stackelem_t));
    if (stk->memory == NULL)
    {
        ONDEBUG(stk->err = NULL_STACK_DATA;)
        return ERROR;
    }
    stk->data = stk->memory + 1;

    for (size_t i = 0; i < capacity; i++)
        stk->data[i] = POIZON;
    stk->size = 0;

ONDEBUG(
            stk->data[-1]            = LEFT_DATA_CANARY;
            stk->data[stk->capacity] = RIGHT_DATA_CANARY;
            stk->left_struct_canary  = LEFT_STRUCT_CANARY;
            stk->right_struct_canary = RIGHT_STRUCT_CANARY;

            stack_count_data_hash(stk);

            stk->filename = filename;
            stk->name     = name;
            stk->line     = line;

            if ((stk->err = stack_ok(stk)) != STACK_OK)
                return ERROR;
        )
    return SUCCESS;
}

return_code stack_detor( stack_t *stk )
{
    if (stk == NULL)
    {
        return ERROR;
    }

    stk->capacity = 0;
    stk->size = 0;
    free(stk->memory);

    ONDEBUG(
            stk->data_hash = 0;
            stk->err = STACK_DESTROYED;
            )

    return SUCCESS;
}

return_code stack_push( stack_t *stk, stackelem_t x )
{
    if (stk == NULL)
        return ERROR;

    if (stk->size >= stk->capacity)
        stack_resize_up(stk);
    stk->data[stk->size++] = x;

    ONDEBUG(
            stack_count_data_hash(stk);
            if ((stk->err = stack_ok(stk)) != STACK_OK)
                return ERROR;
            )
    return SUCCESS;
}

return_code stack_pop( stack_t *stk, stackelem_t *x )
{
    if (stk == NULL)
    {
        *x = POIZON;
        return ERROR;
    }
    if (stk->size <= 0)
    {
        *x = POIZON;
        ONDEBUG(stk->err = NEGATIVE_STACK_SIZE;)
        return ERROR;
    }

    if (stk->size == stk->capacity / 4)
        stack_resize_down(stk);

    stackelem_t val = stk->data[--stk->size];
    stk->data[stk->size]= POIZON;

    *x = val;
    ONDEBUG(
            stack_count_data_hash(stk);
            if ((stk->err = stack_ok(stk)) != STACK_OK)
                return ERROR;
            )

    return SUCCESS;
}

return_code stack_resize_up( stack_t *stk )
{
    if (stk == NULL)
        return ERROR;
    stk->memory = (stackelem_t*)realloc(stk->memory,
                                       (stk->capacity * 2 + 2) * sizeof(stackelem_t));

    stk->data = stk->memory + 1;
    stk->capacity *= 2;
    for (size_t i = stk->size; i < stk->capacity; i++)
        stk->data[i] = POIZON;

    ONDEBUG(
            stk->data[stk->capacity] = RIGHT_DATA_CANARY;
            if ((stk->err = stack_ok(stk)) != STACK_OK)
                return ERROR;
            )
    return SUCCESS;
}

return_code stack_resize_down( stack_t *stk )
{
    if (stk == NULL)
        return ERROR;

    stk->memory = (stackelem_t*)realloc(stk->memory,
                                       (stk->capacity / 2 + 2) * sizeof(stackelem_t));

    stk->data = stk->memory + 1;
    stk->capacity /= 2;

    ONDEBUG(
            stk->data[stk->capacity] = RIGHT_DATA_CANARY;
            if ((stk->err = stack_ok(stk)) != STACK_OK)
                return ERROR;
            )
    return SUCCESS;
}

void stack_ugabuga_test( stack_t *stk )
{
    stackelem_t x;
    for (int i = 1; i < TEST_AMOUNT; i++)
    {
        switch (rand() % 2)
        {
            case 0:
                stack_push(stk, rand() % i);
                break;
            case 1:
                stack_pop(stk, &x);
                break;
            default:
                break;
        }
    }

    return;
}

ONDEBUG(
uint64_t stack_count_data_hash( stack_t *stk )
{
    return stk->data_hash = djb2(stk->memory, (stk->capacity + 2) * sizeof(stackelem_t)); //2 canaries
}

void stack_dump( stack_t *stk )
{
        if (stk == NULL)
        {
            printf(RED "ERROR:NULL stack pointer\n" WHITE);
            return;
        }
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
            stk->data);
//print left canary
        printf("\t\t[%d] %lg %s\n",
                -1,
                stk->data[-1],
                is_zero(stk->data[-1] - LEFT_DATA_CANARY)
                ?
                YELLOW "canary" WHITE
                :
                RED "DEAD CANARY" WHITE);
//print data
    for (size_t i = 0; i < stk->capacity; i++)
        printf("\t\t[%zu] %lg %s\n",
                i,
                stk->data[i],
                isnan(stk->data[i]) ? RED "poison" WHITE : "");
//print right canary
        printf("\t\t[%zu] %lg %s\n",
                stk->capacity,
                stk->data[stk->capacity],
                is_zero(stk->data[stk->capacity] - RIGHT_DATA_CANARY)
                ?
                YELLOW "canary" WHITE
                :
                RED "DEAD CANARY" WHITE);
        printf(BLUE "\t\tdata hash = %lu\n" WHITE, stk->data_hash);

    return;
}

return_code stack_print_error( stack_t *stk )
{
    if (stk == NULL)
        return ERROR;

    switch (stk->err)
    {
        case STACK_OK:
            printf(GREEN "correct stack\n" WHITE);
            break;
        case NULL_STACK:
            printf(RED "null stack pointer\n" WHITE);
            break;
        case STACK_DESTROYED:
            printf(RED "destructed stack\n" WHITE);
            break;
        case NULL_STACK_DATA:
            printf(RED "null stack data pointer\n" WHITE);
            break;
        case NEGATIVE_STACK_SIZE:
            printf(RED "negative stack size\n" WHITE);
            break;
        case NEGATIVE_STACK_CAPACITY:
            printf(RED "negative stack capacity\n" WHITE);
            break;
        case RIGHT_DATA_CANARY_DEAD:
            printf(RED "right data canary is dead, data may be corrupted\n" WHITE);
            break;
        case LEFT_DATA_CANARY_DEAD:
            printf(RED "left data canary is dead, data may be corrupted\n" WHITE);
            break;
        case RIGHT_STRUCT_CANARY_DEAD:
            printf(RED "right stack canary is dead, stack may be corrupted\n" WHITE);
            break;
        case LEFT_STRUCT_CANARY_DEAD:
            printf(RED "right stack canary is dead, stack may be corrupted\n" WHITE);
            break;
        case WRONG_DATA_HASH:
            printf(RED "wrong data hash, data may be corrupted\n" WHITE);
            break;
        default:
            printf(RED "Unknown error\n" WHITE);
            break;
    }

    return SUCCESS;
}

stack_err_code_t stack_ok( stack_t *stk )
{
    if (stk == NULL)
        return NULL_STACK;

    if (stk->data == NULL)
        return NULL_STACK_DATA;

    if (stk->capacity > SIZE_MAX / 2)
        return NEGATIVE_STACK_CAPACITY;

    if (stk->size > SIZE_MAX / 2)
        return NEGATIVE_STACK_SIZE;

    if (!is_zero(stk->data[-1] - LEFT_DATA_CANARY))
        return LEFT_DATA_CANARY_DEAD;

    if (!is_zero(stk->data[stk->capacity] - RIGHT_DATA_CANARY))
        return RIGHT_DATA_CANARY_DEAD;

    if (!is_zero(stk->left_struct_canary - LEFT_STRUCT_CANARY))
        return LEFT_STRUCT_CANARY_DEAD;

    if (!is_zero(stk->right_struct_canary - RIGHT_STRUCT_CANARY))
        return RIGHT_STRUCT_CANARY_DEAD;

    stack_count_data_hash(stk);
    if (stk->data_hash != stack_count_data_hash(stk))
        return WRONG_DATA_HASH;

    return STACK_OK;
}

bool is_zero( stackelem_t x )
{
    return fabs(x) < EPSILON;
}

uint64_t djb2( void *ptr, size_t size )
{
    uint64_t hash = 0xEDA;
    for (size_t i = 0; i < size; i++)
    {
        hash = ((hash << 5) + hash) + *((unsigned char*)ptr + i);
    }

    return hash;
}
)
