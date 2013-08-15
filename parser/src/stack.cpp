#include "stack.h"
void stack_init(struct stack *sp){
	sp->bot = 0;
	sp->top = sp->bot;
}
stacktype push(stacktype i,struct stack *sp)
{
  (sp->top)++;
  if(sp->top == (sp->bot+STACK_SIZE)) {
    printf("Stack Overflow.\n");
    return stacktype(-1);
  }
  sp->buf[sp->top] = i;
  return sp->buf[sp->top];
}

stacktype pop(struct stack *sp)
{
  if(sp->top == sp->bot) {
    printf("Stack Underflow.\n");
    return stacktype(-1);;
  }
  (sp->top)--;
  return sp->buf[sp->top+1];
}
stacktype top(struct stack *sp){
  if(sp->top == sp->bot) {
    printf("Stack empty.\n");
    return stacktype(-1);
  }
  else return sp->buf[sp->top];
}
bool empty(struct stack *sp){
  if(sp->top == sp->bot) {
    return true;
  }
  else return false;
}
 