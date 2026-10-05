#include "stack.h"
#include <string.h>

void stack_ugabuga_test( stack_t *stk );

int main( void )
{
    stack_t stk1 = {};

    if (stack_ctor(&stk1, 4 ONDEBUG(, __FILE__, "stk1", __LINE__)) != SUCCESS)
    {
   // printf("che");
        ONDEBUG(stack_print_error(&stk1);)
        return 1;
    }
    stackelem_t x = 0;

    //stack_push(&stk1, 777);
    //memset(&stk1, -1, sizeof(stk1));
    while (true)
    {
        printf(YELLOW "q - exit\n"
               "1 - push element\n"
               "2 - pop element\n"
               "4 - dump stack (DEBUG MODE ONLY)\n"
               "5 - run test // ?(DEBUG MODE ONLY)? \\\\\n" WHITE);
        switch(getchar())
        {
            case 'q':
                if (stack_detor(&stk1) != SUCCESS)
                {
                    ONDEBUG(stack_print_error(&stk1);)
                    return 1;
                }
                return 0;
                break;
            case '1':
                printf("Enter value:\n");
                scanf("%lf", &x);
                if (stack_push(&stk1, x) != SUCCESS) {ONDEBUG(stack_dump(&stk1));}
                break;
            case '2':
                if (stack_pop(&stk1, &x) != SUCCESS) {ONDEBUG(stack_dump(&stk1));}
                printf("popped %lg\n", x);
                break;
            ONDEBUG(
                    case '4':
                        stack_dump(&stk1);
                        break;
                    )
                case '5':
                    stack_ugabuga_test(&stk1);
                    break;

            default:
                printf("Invalid input, try again\n");
                break;
        }
        getchar();
    }

    stack_detor(&stk1);
    return 0;
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
