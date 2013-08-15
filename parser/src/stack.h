#include <stdio.h>
#include <stdlib.h>
#define STACK_SIZE 512

/*stack oprater*/
#define stacktype char

stacktype push(stacktype i,struct stack *sp);
stacktype pop(struct stack *sp);
stacktype top(struct stack *sp);
bool empty(struct stack *sp);
void stack_init(struct stack *sp);
struct stack{
	 int top,
		 bot;
	stacktype buf[STACK_SIZE];
};