#include "scanner.h"

/*functions*/
/*part1:program control*/
void synAndsem_start();//init work,open files and start scanner
void synAndsem_end();//end work,close files and end scanner
void synAndsem_analyze();//main conrole,called by main function
void synAndsem_writeAsm();//write codes and datas to asm file
/*part1:end*/

/*part2:Vn state*/
void Vn_program();
void Vn_sentence();
void Vn_while();
void Vn_assignment();
void Vn_judgment();
void Vn_exp_lev_0();
void Vn_exp_lev_0p();
void Vn_exp_lev_1();
void Vn_exp_lev_1p();
void Vn_op_lev_0(bool is_sign,struct Unit* ret);
void Vn_op_lev_1(struct Unit* ret);
void Vn_data(struct Unit* ret);
void Vn_integer(struct Unit* ret);
void Vn_jud_op(struct Unit* ret);
/*part2:end*/

unsigned int get_var_id(const char var[256]);
void Asm_Expression(char sec_op);

/*part 3:mac defines to judge wheather the current_unit belong to the 
first collections or follow collectios of Vn*/
	//first(op_lev_0),comst_int
#define bel_fir_integer (bel_fir_op_lev_0||\
	0x107 == current_unit.code)

	//identifier,first(integer)
#define bel_fir_data (bel_fir_integer||\
	0x104 == current_unit.code)

	//*,/
#define bel_fir_op_lev_1  (	0x11b == current_unit.code && \
							( 0 == strcmp(current_unit.value,"*")\
							||0 == strcmp(current_unit.value,"/")) \
						)

	//+,-
#define bel_fir_op_lev_0 0x11a == current_unit.code

	//first(op_lev_1)
	/*follow(exp_lev_1p) = first(exp_lev_0p) */
#define bel_fir_exp_lev_1p (bel_fir_op_lev_1||bel_fir_exp_lev_0p)

	//first(op_lev_0)+';',used for follow(exp_lev_1p)
#define bel_fir_exp_lev_0p (bel_fir_op_lev_0||\
	0x122 == current_unit.code)

	//first(data)
#define bel_fir_exp_lev_1 bel_fir_data

	//first(exp_lev_1)
	/*follow(exp_lev_0)=follow(judgment)+first(jud_op)*/

	/*#define bel_fol_judgment 0x11d == current_unit.code &&
	0 == strcmp(current_unit.value,")");*/
	//)
#define bel_fir_exp_lev_0 bel_fir_exp_lev_1

	//first(exp_lev_0)
#define bel_fir_judgment bel_fir_exp_lev_0

	//<,>
#define bel_fir_jud_op (	0x118 == current_unit.code && \
							( 0 == strcmp(current_unit.value,"<")\
							||0 == strcmp(current_unit.value,">"))\
						)

	//identifier,';'
#define bel_fir_assignment (0x104 == current_unit.code ||\
						0 == strcmp(current_unit.value,";"))

	//while
#define bel_fir_while (0x103 == current_unit.code && 0 ==\
						strcmp(current_unit.value,"while"))

	//first(while),first(assignment)

	/*#define bel_fir_program bel_fir_sentence|| 
						  0x130 == current_unit.code*/
	//first(sentence),program end
#define bel_fir_sentence (bel_fir_while||bel_fir_assignment)
/*part3:end*/


/*part 4:print information*/
#define PRINT_INF_F(STR) \
	fprintf(fp_inf,"Line %ld:word \'%s\';%s\n",\
	current_unit.line,current_unit.value,STR)
#define PRINT_INF_S(STR) \
	fprintf(fp_inf,"%s\n",STR)
/*part4:end*/



/*part4:for output asm code*/
	//check whether the asm var is declared,and invoke declaring
#define CHECK_DEFINE(STR) get_var_id(STR)
	
#define PRINT_ASM_COM(SEG,STR) \
	sprintf(buf, "%s;%s\n",FormatSpace,STR);\
	strcat(SEG,buf)

#define ASM_POP_TO_REG(REG)\
    sprintf(buf, "%sPOP%s%s\n",FormatSpace,WordSpit,REG);\
    strcat(asm_code_seg, buf)

	/*asm data define*/
	/*java 与汇编命名规则不尽相同（如是否区分大小写），这里需要重新命名*/
#define ASM_DEFINE_VAR(i)\
		sprintf(buf, "%sJVAR_NAME_%d%sDB%s\'%s\',0\n",\
			FormatSpace,i,WordSpit,WordSpit,vars[i]);\
        strcat(asm_data_seg, buf);\
        sprintf(buf, "%sVAR_%d%sDW%s0\n",\
			FormatSpace,i,WordSpit,WordSpit);\
        strcat(asm_data_seg, buf)

#define ASM_ASSIGN(VAR)\
    ASM_POP_TO_REG("AX");\
    sprintf(buf, "%sMOV%sVAR_%d, AX\n",\
		FormatSpace,WordSpit,get_var_id(VAR));\
    strcat(asm_code_seg, buf)

	/*force data to REG'size*/
#define ASM_IN_REG(REG,VAR)\
		sprintf(buf, "%sMOV%s%s, %s\n", \
			FormatSpace,WordSpit,REG,VAR);\
		strcat(asm_code_seg, buf)

	/*VAR is a string,const int or identifier*/
#define ASM_IN_STACK(VAR)\
    ASM_IN_REG("DX",VAR);\
    sprintf(buf,"%sPUSH%s%s\n",FormatSpace,WordSpit,"DX");\
    strcat(asm_code_seg, buf)

#define ASM_WHILE_HEAD(VAR)\
    ASM_POP_TO_REG("BX");\
    ASM_POP_TO_REG("AX");\
    sprintf(buf, "%sCMP%s%s, %s\n", FormatSpace,WordSpit,"AX","BX");\
	strcat(asm_code_seg, buf);\
	sprintf(buf,"%s%s%sWLOPEND_%u\n",\
		FormatSpace,VAR,WordSpit,while_count);\
    strcat(asm_code_seg, buf)
#define ASM_EXPRESSION(VAR) Asm_Expression(VAR)

#define ASM_OP_ADD() \
    sprintf(buf, "%sADD%sAX, BX\n",FormatSpace,WordSpit);\
    strcat(asm_code_seg, buf);\
    sprintf(buf, "%sPUSH%sAX\n",FormatSpace,WordSpit);\
    strcat(asm_code_seg, buf)

#define ASM_OP_SUB() \
    sprintf(buf, "%sSUB%sAX, BX\n",FormatSpace,WordSpit);\
    strcat(asm_code_seg, buf);\
    sprintf(buf, "%sPUSH%sAX\n",FormatSpace,WordSpit);\
    strcat(asm_code_seg, buf)

#define ASM_OP_MUL() \
    sprintf(buf, "%sIMUL%sBX\n",FormatSpace,WordSpit);\
    strcat(asm_code_seg, buf);\
    sprintf(buf, "%sXOR%sDX, DX\n",FormatSpace,WordSpit);\
    strcat(asm_code_seg, buf);\
    sprintf(buf, "%sPUSH%sAX\n",FormatSpace,WordSpit);\
    strcat(asm_code_seg, buf)

#define ASM_OP_DIV() \
	sprintf(buf, "%sXOR%sDX, DX\n",FormatSpace,WordSpit);\
    strcat(asm_code_seg, buf);\
    sprintf(buf, "%sIDIV%sBX\n",FormatSpace,WordSpit);\
    strcat(asm_code_seg, buf);\
    sprintf(buf, "%sPUSH%sAX\n",FormatSpace,WordSpit);\
    strcat(asm_code_seg, buf)
/*part 5: end*/

/*wheather the current_unit belong to first or 
follow(needed) collection of Vn*/

//bool bel_fir_jud_op();//<,>
//bool bel_fir_integer();//first(op_lev_0),comst_int
//bool bel_fir_data();//identifier,first(integer)
//bool bel_fir_op_lev_1();//*,/
//bool bel_fir_op_lev_0();//+,-
//bool bel_fir_exp_lev_1p();//first(op_lev_1)
///*follow(exp_lev_1p) = first(exp_lev_0p) */
//bool bel_fir_exp_lev_0p();
//first(op_lev_0)+fir(jud_op),used for follow(exp_lev_1p)
//bool bel_fir_exp_lev_1();//first(data)
//bool bel_fir_exp_lev_0();//first(op_lev_0)
///*follow(exp_lev_0)=follow(judgment)+first(jud_op)*/
//bool bel_fol_judgment();//)
//bool bel_fir_judgment();//first(exp_lev_0)
//bool bel_fir_jud_op();//<,>
//bool bel_fir_assignment();//identifier,';'
//bool bel_fir_while();//while
//bool bel_fir_sentence();//first(while),first(assignment)
//bool bel_fir_program();//first(sentence),program end