#include <stdlib.h>
#include <pthread.h>

#include "stack.h"
#include "constants.h"

pthread_mutex_t stack_lock;

int stack_create(stack **f_stack){
    stack *s;

    s = malloc(sizeof(stack));

    if (pthread_mutex_init(&stack_lock, NULL) != 0){
        printf("Error Mutex failed!\n");
        return INITIALIZATION_UNSUCCESSFUL;
    }

    if(!s){
        pthread_mutex_destroy(&stack_lock);
        printf("Initialization unsuccessful!\n");
        return INITIALIZATION_UNSUCCESSFUL;
    }

    s->indexes = malloc(sizeof(int));
    if(!s->indexes){
        free(s);
        pthread_mutex_destroy(&stack_lock);

        printf("Indexes unsuccessfully resized!\n");
        return INITIALIZATION_UNSUCCESSFUL;
    }
    s->current_amount = INITIAL_POSITION_IN_ARRAY;
    s->max_amount = INITIAL_MAX_OF_GAME_ARRAY;
    s->sp = EMPTY;


    *f_stack = s;

    return INITIALIZATION_SUCCESSFUL;
}


int stack_resize(stack *f_stack){
    stack *s;
    int len;
    if(!f_stack){
        printf("Cannot resize an empty stack!\n");
        return INVALID_PARAMETER;
    }

    len = f_stack->max_amount + RESIZING_CONSTANT;

    s = realloc(f_stack, sizeof(stack) + len * sizeof(int));
    if(!s){
        printf("Cant resize!\n");
        return RESIZING_UNSUCCESSFUL;
    }

    f_stack = s;
    s->max_amount = len;

    return RESIZING_SUCCESSFUL;
}


int stack_push(stack *f_stack, int index){
    pthread_mutex_lock(&stack_lock);
    int curr;
    if(!f_stack || index < 0 ){
        printf("Invalid parameters! - stack push\n");
        return INVALID_PARAMETER;
    }
    curr = f_stack->current_amount;

    if(curr == f_stack->max_amount){
        if(stack_resize(f_stack) == RESIZING_UNSUCCESSFUL){
            printf("Resizing unsuccessful!\n");
            pthread_mutex_unlock(&stack_lock);
            return RESIZING_UNSUCCESSFUL;
        }
    }

    f_stack->indexes[curr] = index;

    f_stack->sp++;
    f_stack->current_amount++;

    pthread_mutex_unlock(&stack_lock);
    return INITIALIZATION_SUCCESSFUL;
}

int stack_pop(stack *f_stack){
    pthread_mutex_lock(&stack_lock);

    int curr;
    if(!f_stack){
        printf("Incorrect parameters! - stack pop\n ");
        pthread_mutex_unlock(&stack_lock);

        return INVALID_PARAMETER;
    }

    if((f_stack->sp == EMPTY)){
        pthread_mutex_unlock(&stack_lock);
        return EMPTY;
    }
    curr = f_stack->current_amount;

    f_stack->sp--;
    f_stack->current_amount--;

    pthread_mutex_unlock(&stack_lock);

    return f_stack->indexes[curr];
}


void stack_free(stack **f_stack){
    if(!*f_stack){
        printf("Cannot free empty stack!\n");
        return INVALID_PARAMETER;
    }
    pthread_mutex_destroy(&stack_lock);
    free((*f_stack)->indexes);
    free(*f_stack);
}
