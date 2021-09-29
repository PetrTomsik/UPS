#ifndef _STACK_H
#define _STACK_H


typedef struct STACK{
    int sp;
    int current_amount;
    int max_amount;
    int *indexes;

}stack;

int stack_create(stack **f_stack);
int stack_resize(stack *f_stack);
int stack_push(stack *f_stack, int index);
int stack_pop(stack *f_stack);
void stack_free(stack **f_stack);

#endif