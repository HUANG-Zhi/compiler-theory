# include "syn_sem.h"
/*scanner output unit,Get a word from src*/
/*
	asm comand :
	compile:	ml.exe /c *.asm
	link:		link.exe c0s.obj *.obj,*,,cs.lib
	run:		*.exe
*/
int main()
{	//printf("\',\",/,\\\n");
	synAndsem_analyze();
	return 0;
}