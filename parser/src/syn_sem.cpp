#include "syn_sem.h"
#include "stack.h"

extern struct Unit current_unit;
struct stack stack_operator;

FILE *fp_asm = NULL,*fp_inf = NULL;
/*for asm*/
#define MAX_SEGMENT 65536 //64K
char asm_data_seg[MAX_SEGMENT] = "",
	asm_code_seg[MAX_SEGMENT] = "";
char buf[256]="";
#define FormatSpace "		"
#define WordSpit "	"
unsigned int while_count = 0;

#define MAX_VAR_NUM 512
char vars[MAX_VAR_NUM][UNITVALUESIZE]={""};
unsigned int var_count = 0;

const int file_max_path_lengh = 1024;
char fp_asm_name[file_max_path_lengh]="SynSemO";
void synAndsem_start(){

	/*between scanner_start and scanner_end ,use scanner_get_unin
	to get a scanner word from src,return true means success ,or 
	the src file is end（current_unit.code = 0x130）;the scanner 
	word is stored in the extern gloable struct Unit variable 
	current_unit;to know more ,please see the head of scanner.h*/

	scanner_start();
	
	/*open dst file*/
	if(NULL != fp_inf){
		fclose(fp_inf);
		fp_inf = NULL;
	}
	if(NULL == (fp_inf=fopen("syn_sem_inf.txt","wt"))){
		printf("Cannot open syn_sem_inf.txt");
		exit(0);
		/*测试是否会清空，重写时，需要清空*/
	}
	else {
		fseek(fp_inf,0*sizeof(char),SEEK_SET);
	}

	/*create asm file*/
	if(0 == strcmp("",fp_asm_name)){
		printf("Please input the out asm file path!\n");
		scanf("%s",fp_asm_name);
	}
	strcat(fp_asm_name,".asm");
	if(NULL != fp_asm){
		fclose(fp_asm);
		fp_asm = NULL;
	}
	if(NULL == (fp_asm=fopen(fp_asm_name,"wt"))){
		printf("Cannot open %s",fp_asm_name);
		exit(0);
	}
	else {
		fseek(fp_asm,0*sizeof(char),SEEK_SET);
	}
	stack_init(&stack_operator);

	fprintf(fp_inf,"start analyze……\n");
}
void synAndsem_analyze(){
	synAndsem_start();
	scanner_get_unin();
	Vn_program();
	synAndsem_end();
}
void synAndsem_writeAsm(){
	//begin 
	fprintf(
		fp_asm,"%s%s",
		".MODEL	SMALL, C\n",
		"EXTRN	printf:NEAR\n\n"
		);
	//stack
	fprintf(
		fp_asm,"%s%s%s",
		"STACKSG	SEGMENT	STACK \'S\'\n",
		"		DW	512 DUP(?)\n",
		"STACKSG	ENDS\n\n"
		);
	//data 3 parts
	fprintf(
		fp_asm,"%s",
		"DATA	SEGMENT WORD PUBLIC \'DATA\'\n"
		);
	fprintf(fp_asm,"%s",asm_data_seg);
	fprintf(
		fp_asm,"%s%s",
		"DATA	ENDS\n\n",
		"DGROUP	GROUP DATA\n\n"
		);
	//code 3 parts
	fprintf(
		fp_asm,"%s%s%s",
		"CODE	SEGMENT	WORD PUBLIC \'CODE\'\n",
		"MAIN	PROC	NEAR\n",
		"		ASSUME	CS:CODE, DS:DGROUP, SS:STACKSG\n"
		);
	fprintf(fp_asm,"%s",asm_code_seg);
	fprintf(
		fp_asm,"%s%s%s%s%s",
		"		MOV	AX, 4C00H\n",
		"		INT	21H\n",
		"MAIN	ENDP\n",
		"CODE	ENDS\n\n",
		"		END     MAIN\n"
		);
}
void synAndsem_end(){

	synAndsem_writeAsm();
	scanner_end();
	fprintf(fp_inf,"analyze end！\n");

	if(NULL != fp_asm){
		fclose(fp_asm);
		fp_asm = NULL;
	}
	if(NULL != fp_inf){
		fclose(fp_inf);
		fp_inf = NULL;
	}
}

unsigned get_var_id(const char var[256]){
		/*add var define for the first time*/
	int i=0;
	for(i=0;i<var_count;i++){
		if(0 == strcmp(var,vars[i]))
		{
			break;
		}
	}
	if(i == var_count){
		strcpy(vars[var_count++],var);
		ASM_DEFINE_VAR(i);
	}
	/*add end*/
	return i;
}

void Vn_program(){
	if (bel_fir_sentence)
    {
        Vn_sentence();
		Vn_program();
    }
	else if (0x130 == current_unit.code)//program end!
    {
        return;
    }
    else
    {
		fprintf(fp_inf,"Line %ld:invalid  start \'%s\'\n",current_unit.line,current_unit.value);
    }

}
void Vn_sentence(){
	if(bel_fir_while){
		Vn_while();
	}
	else if(bel_fir_assignment){
		Vn_assignment();
	}
	else {
		fprintf(fp_inf,"Line %ld:invalid sentence start \'%s\'\n",current_unit.line,current_unit.value);
	}
}
void Vn_while(){
	PRINT_INF_S("while start");
	PRINT_ASM_COM(asm_code_seg,"while start");
	sprintf(buf,"WLOPSTAR_%u:\n",++while_count);
	strcat(asm_code_seg,buf);
	scanner_get_unin();//get next word
	if(0 == strcmp(current_unit.value,"("))
	{
		scanner_get_unin();
		if(bel_fir_judgment){
			Vn_judgment();
		}
		else {
			PRINT_INF_F("must be judgment here");
		}
		if(0 == strcmp(current_unit.value,")"))
		{
			scanner_get_unin();
			Vn_assignment();
			sprintf(buf,"%sJMP%sWLOPSTAR_%u\n",FormatSpace,WordSpit,while_count);
			strcat(asm_code_seg,buf);

			sprintf(buf, "WLOPEND_%u:\n",while_count);
			strcat(asm_code_seg, buf);
		}
		else {
			PRINT_INF_F("need \")\"");
		}
	}
	else {
		PRINT_INF_F("need \"(\"");
	}
	PRINT_ASM_COM(asm_code_seg,"while end");
	PRINT_INF_S("while end");
}
void Vn_assignment(){
	char var[UNITVALUESIZE]="";
	strcpy(var,current_unit.value);
	PRINT_INF_S("assignment start");
	PRINT_ASM_COM(asm_code_seg,"assignment start");
	CHECK_DEFINE(var);//get_var_id();//for asm define
	scanner_get_unin();
	if(0 == strcmp("=",current_unit.value))
	{
		scanner_get_unin();
		Vn_exp_lev_0();
		if(0 == strcmp(";",current_unit.value))
		{
			ASM_ASSIGN(var);
			scanner_get_unin();
		}
		else {
			PRINT_INF_F("need \';\'");
		}
	}
	else {
		PRINT_INF_F("must be \';\' here");
	}
	PRINT_ASM_COM(asm_code_seg,"assignment end");
	PRINT_INF_S("assignment end");
}
void Vn_judgment(){
	
    PRINT_INF_S("judgment start");
	PRINT_ASM_COM(asm_code_seg,"judgment start");
	struct Unit lefdata,cmp_op,rightdata; 

	if(bel_fir_data)
	{
		Vn_data(&lefdata);
		if(bel_fir_jud_op)
		{
			Vn_jud_op(&cmp_op);

			if(bel_fir_data)
			{
				Vn_data(&rightdata);
			}
			else {
				PRINT_INF_F("must be Vn_data here");
			}
		}
		else {
			PRINT_INF_F("must be Vn_jud_op here");
		}
	}
	else {
		PRINT_INF_F("must be Vn_data here");
	}
    
	if (0x107 == lefdata.code)//identifier and const_int
    {
		ASM_IN_STACK(lefdata.value);
    }
	else if(0x104 == lefdata.code)
	{
		char VarId[256] = "",VarName[256]="VAR_";
		sprintf(VarId,"%d",get_var_id(lefdata.value));
		strcat(VarName,VarId);
		ASM_IN_STACK(VarName);
	}

	if (0x107 == rightdata.code)//identifier and const_int
    {
		ASM_IN_STACK(rightdata.value);
    }
	else if(0x104 == rightdata.code)
	{
		char VarId[256] = "",VarName[256]="VAR_";
		sprintf(VarId,"%d",get_var_id(rightdata.value));
		strcat(VarName,VarId);
		ASM_IN_STACK(VarName);
	}

	if (0 == strcmp(cmp_op.value,"<"))
    {
        ASM_WHILE_HEAD("JNL");
    }
    else if (0 == strcmp(cmp_op.value,">"))
    {
        ASM_WHILE_HEAD("JNG");
    }
	PRINT_ASM_COM(asm_code_seg,"judgment end");
    PRINT_INF_S("assignment end");
}
void Vn_exp_lev_0(){
			
    PRINT_INF_S("Vn_exp_lev_0 start");

	push('#',&stack_operator);

	if(bel_fir_exp_lev_1)
	{
		Vn_exp_lev_1();
		if(bel_fir_exp_lev_0p)
		{
			Vn_exp_lev_0p();
		}
		else {
			PRINT_INF_F("must be Vn_exp_lev_0p here");
		}
	}
	else {
		PRINT_INF_F("must be Vn_exp_lev_1 or Vn_jud_op here");
	}

	ASM_EXPRESSION('#');
    //opnd.pop();

	PRINT_INF_S("Vn_exp_lev_0p end");
}
void Vn_exp_lev_0p(){
		
	struct Unit toperator; 

	if(bel_fir_op_lev_0)
	{
		Vn_op_lev_0(false,&toperator);
		ASM_EXPRESSION(toperator.value[0]);
		if(bel_fir_exp_lev_1)
		{
			Vn_exp_lev_1();
			if(bel_fir_exp_lev_0p)
			{
				Vn_exp_lev_0p();
			}
			else {
				PRINT_INF_F("must be Vn_exp_lev_0p here");
			}
		}
		else {
			PRINT_INF_F("must be Vn_exp_lev_1 here");
		}
	}
	else if(0x122 == current_unit.code){//;
		return;
	}
	else {
		PRINT_INF_F("must be Vn_op_lev_0 or Vn_jud_op here");
	}
	
	PRINT_INF_S("Vn_exp_lev_0p end");
}
void Vn_exp_lev_1(){
				
	struct Unit ret; 

	if(bel_fir_data)
	{
		Vn_data(&ret);
		if (0x107 == ret.code)//identifier and const_int
		{
			ASM_IN_STACK(ret.value);
		}
		else if(0x104 == ret.code)
		{
			char VarId[256] = "",VarName[256]="VAR_";
			sprintf(VarId,"%d",get_var_id(ret.value));
			strcat(VarName,VarId);
			ASM_IN_STACK(VarName);
		}

		if(bel_fir_exp_lev_1p)
		{
			Vn_exp_lev_1p();
		}
		else {
			PRINT_INF_F("must be Vn_exp_lev_1p here");
		}
	}
	else {
		PRINT_INF_F("must be Vn_data here");
	}
	
	PRINT_INF_S("Vn_exp_lev_1 end");
}
void Vn_exp_lev_1p(){
    if (bel_fir_op_lev_1)
    {
        struct Unit toperator,ret;
        Vn_op_lev_1(&toperator);
		ASM_EXPRESSION(toperator.value[0]);
        Vn_data(&ret);
        if (0x107 == ret.code)//identifier and const_int
		{
			ASM_IN_STACK(ret.value);
		}
		else if(0x104 == ret.code)
		{
			char VarId[256] = "",VarName[256]="VAR_";
			sprintf(VarId,"%d",get_var_id(ret.value));
			strcat(VarName,VarId);
			ASM_IN_STACK(VarName);
		}
        Vn_exp_lev_1p();
    }
    else if(bel_fir_exp_lev_0p)
    {
        return;//ε
    }
	else {
		PRINT_INF_F("must be Vn_op_lev_1 or Vn_exp_lev_0p here");
	}
}

/*?*/
void Vn_op_lev_0(bool is_sign,struct Unit* ret){
	if(0x11a == current_unit.code)
	{
		if(NULL != ret)
		{
			strcpy(ret->value,current_unit.value);
			ret->conmplete = false;//here conmplete used for is_sign
		}
		PRINT_INF_S(current_unit.value);
		current_unit.conmplete = is_sign;
		scanner_get_unin();

	}
	else {
		PRINT_INF_F("need \'+\'or\'-\' here");
	}
}
void Vn_op_lev_1(struct Unit* ret){
	if(0x11b == current_unit.code)
	{
		if(NULL != ret)
		{
			strcpy(ret->value,current_unit.value);
		}
		PRINT_INF_S(current_unit.value);
		scanner_get_unin();
	}
	else {
		PRINT_INF_F("need \'*\'or\'/\' here");
	}
}
void Vn_data(struct Unit* ret){
	if(0x104 == current_unit.code)
	{
		CHECK_DEFINE(current_unit.value);
		if(NULL != ret)
		{
			ret->code = 0x104;
			strcpy(ret->value,current_unit.value);
		}
		PRINT_INF_S(current_unit.value);
		scanner_get_unin();
	}
	else if(bel_fir_op_lev_0||0x107 == current_unit.code)
	{
		struct Unit integer;
		Vn_integer(&integer);
		if(NULL != ret){
			ret->code = 0x107;
			strcpy(ret->value,integer.value);
		}
	}
	else 
	{
		PRINT_INF_F("invaild data");
	}
}
void Vn_integer(struct Unit* ret){
    char sign[3] ="";//""+;"-",-
    if (bel_fir_op_lev_0)
    {
        if(0 == strcmp(current_unit.value,"-")){
			strcpy(sign,"-");
		}
        Vn_op_lev_0(true,NULL);//next unit
		if (0x107 == current_unit.code)
        {
            if (NULL != ret)
            {
				ret->code = 0x107;
				strcpy(ret->value,sign);
				strcat(ret->value,current_unit.value);
            }
			PRINT_INF_S(current_unit.value);
            scanner_get_unin();
        }
        else
        {
            PRINT_INF_F("need a data");
        }
    }
    else if (0x107 == current_unit.code)
    {
        if (NULL != ret)
        {
			ret->code = 0x107;
			strcpy(ret->value,current_unit.value);
        }
        PRINT_INF_S(current_unit.value);
        scanner_get_unin();
    }
    else
    {
        PRINT_INF_F("need a data");
    }
}
void Vn_jud_op(struct Unit* ret){
	ret->code = current_unit.code;
	strcpy(ret->value,current_unit.value);
	if (0 == strcmp(current_unit.value,">"))
    {
        PRINT_INF_S(">");
		scanner_get_unin();
    }
    else if (0 == strcmp(current_unit.value,"<"))
    {
        PRINT_INF_S(">");
		scanner_get_unin();
    }
    else
    {
        PRINT_INF_F("must be \">\" or \"<\" here");
    }
}
void Asm_Expression(char sec_op)
{
	char fir_op = top(&stack_operator);
	int cmp_code = -2;
	bool loop_flag = true;
	while(loop_flag){
		/*判断操作符优先级*/
		fir_op = top(&stack_operator);

		if('#' == fir_op && '#' == sec_op)
			cmp_code = 0;
		else if ('#' == fir_op )
			cmp_code = -1;
		else if ('#' == sec_op )
			cmp_code = 1;
		else if ('*' == fir_op||'/' == fir_op )
			cmp_code = 1;
		else if ('*' == sec_op||'/' == sec_op )
			cmp_code = -1;
		else cmp_code = 1;

		switch(cmp_code){
		case -1://<
			push(sec_op,&stack_operator);
			loop_flag = false;
			break;
		case 1://>
        ASM_POP_TO_REG("BX");
        ASM_POP_TO_REG("AX");
        switch (fir_op)
        {
        case '+':
            ASM_OP_ADD();
            break;
        case '-':
            ASM_OP_SUB();
            break;
        case '*':
            ASM_OP_MUL();
            break;
        case '/':
            ASM_OP_DIV();
            break;
        default:
            break;
        }
        pop(&stack_operator);
		loop_flag = true;
        break;

		default://0,=
        pop(&stack_operator);
		loop_flag = false;
        break;
		}
	}
}
///*wheather the current_unit belong to first or follow(needed) collection of Vn*/
//bool bel_fir_jud_op();//<,>
//bool bel_fir_integer();//first(op_lev_0),comst_int
//bool bel_fir_data();//identifier,first(integer)
//bool bel_fir_op_lev_1();//*,/
//bool bel_fir_op_lev_0();//+,-
//bool bel_fir_exp_lev_1p();//first(op_lev_1)
///*follow(exp_lev_1p) = first(exp_lev_0p) */
//bool bel_fir_exp_lev_0p();//first(op_lev_0)+fir(jud_op),used for follow(exp_lev_1p)
//bool bel_fir_exp_lev_1();//first(data)
//bool bel_fir_exp_lev_0();//first(op_lev_0)
///*follow(exp_lev_0)=follow(judgment)+first(jud_op)*/
//bool bel_fol_judgment();//)
//bool bel_fir_judgment();//first(exp_lev_0)
//bool bel_fir_jud_op();//<,>
//bool bel_fir_assignment();//identifier,';'
//bool bel_fir_while()//while
//{
//	return (0x103 == current_unit.code && 0 == strcmp(current_unit.value,"while"));
//}
//bool bel_fir_sentence()//first(while),first(assignment)
//{
//	return (bel_fir_while()||bel_fir_assignment());
//}
//bool bel_fir_program();//first(sentence),program end
