#include "scanner.h"
/*for src buffer*/
static long src_line =0,//src line count
	word_line = 0,//unused
	src_column = 0,//unused
	src_words = 0,
	src_line_words = 0,//line word count
	More_inf_count = 0;//count the " more information" flags int scanner_output
	
 const int src_buf_size = 1024,
	half_buf_size = src_buf_size/2;// two half_buffers
 char src_buf[src_buf_size];
 char * Hbuf = &src_buf[0],//first half buffer
	*Tbuf = &src_buf[half_buf_size],//second half buffer
	*Bpoint = Hbuf,*Fpoint = Hbuf;//Bpoint unused

/*以下内容用英文解释不清楚，使用汉字注释*/
 bool forwordflage = false,//提前调入下一半缓冲区，Fpoint已回退到上一半缓冲区，下次无需从文件读入
	haveread = false,//文件
	wordend = true,//本单词已结束
	CRLF = false,
	newline = true,//新行开始标记（控制行号输出）
	sInquotation = false,//扫描指针在一对单引号内的标记
	dInquotation = false,//扫描指针在一对双引号内的标记
	src_file_done = false;
/*file names and FILE point for them*/
 const int file_max_path_lengh = 1024;
 char src_file[file_max_path_lengh]="";
 char scanner_output[15]="scanner_output";
 char scanner_more[15]="scanner_more";
 FILE *fp_src = NULL,*fp_dst=NULL,*fp_mor_inf=NULL;
 Unit current_unit;
/*初始化current_unit*/
void UnitInit(){
	sInquotation = false,
	dInquotation = false,
	current_unit.value[0] = '\0';
	current_unit.code = -1;
	current_unit.type[0] = '\0';
	current_unit.valuei = 0;
	current_unit.conmplete = true;
	current_unit.error = false;
	current_unit.erinfo[0] = '\0';
	if(true == newline){
		newline = false;
		src_words +=src_line_words;
		src_line_words = 0;
	}
	if(false == newline){
		nextchar();
		if(true == newline){
			newline = false;
			src_words +=src_line_words;
			src_line_words = 0;
		}
		untread();
	}
	current_unit.line = src_line;

}

/*给current_unit.value增加一个字符*/
void UnitInitAddAValueElem(char ch){
	if(ch<0) return ;//-1,-2,-3
	if(UNITVALUESIZE == current_unit.valuei){
		current_unit.value[current_unit.valuei-1] = '\0';
		current_unit.conmplete = false;
	}
	else {
		current_unit.value[current_unit.valuei++] = ch;
	}
}

/*辅助输出函数，以size-1宽度输出str，超出时采用注释的方法，只输出索引号*/
void FormatPrint(char str[],int size){
	int l = strlen(str);
	if(l <= size-1){
		fprintf(fp_dst," %s",str);
		int j= size -l-1;
		for(int i=0;i<j;i++){
			fprintf(fp_dst," ");
		}
	}
	else{
		//printf("M %-14d",++More_inf_count);
		//printf("%-8d :%s\n",str);
		fprintf(fp_dst,"M %-14ld",++More_inf_count);
		fprintf(fp_mor_inf,"%-8ld :%s\n",More_inf_count,str);
	}
}

/*输出单元*/
void DispalyUnit(){
	if(0 == current_unit.valuei){
		return;
	}
	else if(true == current_unit.error){
		//printf("| %-7d| %-7d| %-7d|",src_line,src_line_words,src_line_words+src_words);
		fprintf(fp_dst,"| %-7ld| %-7ld| %-7ld|",current_unit.line,src_line_words,src_line_words+src_words);
		FormatPrint(current_unit.erinfo,16);
		//printf("|");
		fprintf(fp_dst,"| 0x%-5x|",current_unit.code);
		current_unit.value[current_unit.valuei] = '\0';
		FormatPrint(current_unit.value,16);
		//printf("| 0x%-5x|",current_unit.code);
		fprintf(fp_dst,"|\n");
		current_unit.error = false;
	}
	else {
		//printf("| %-7d| %-7d| %-7d|",src_line,src_line_words,src_line_words+src_words);
		fprintf(fp_dst,"| %-7ld| %-7ld| %-7ld|",current_unit.line,src_line_words,src_line_words+src_words);
		FormatPrint(current_unit.type,16);
		//printf("| 0x%-5x|",current_unit.code);
		fprintf(fp_dst,"| 0x%-5x|",current_unit.code);
		FormatPrint(current_unit.value,16);
		//printf("|");
		//printf("\n");
		fprintf(fp_dst,"|\n");
	}
}

/*全局初始化*/
void InitWork()
{
	/*open src file*/
	if(0 == strcmp("",src_file)){
		printf("Please input the src file path!\n");
		scanf("%s",src_file);
	}
	if(NULL != fp_src){
		fclose(fp_src);
		fp_src = NULL;
	}
	if(NULL == (fp_src=fopen(src_file,"rt"))){
		printf("Cannot open %s",src_file);
		exit(0);
	}
	else {
		fseek(fp_src,0*sizeof(char),SEEK_SET);
	}

	/*open dst file*/
	if(NULL != fp_dst){
		fclose(fp_dst);
		fp_dst = NULL;
	}
	if(NULL == (fp_dst=fopen(scanner_output,"wt"))){
		printf("Cannot open %s",scanner_output);
		exit(0);
	}
	else {
		fseek(fp_dst,0*sizeof(char),SEEK_SET);
	}

	/*open inf file*/
	if(NULL != fp_mor_inf){
		fclose(fp_mor_inf);
		fp_mor_inf = NULL;
	}
	if(NULL == (fp_mor_inf=fopen(scanner_more,"wt"))){
		printf("Cannot open %s",scanner_more);
		exit(0);
	}
	else {
		fseek(fp_mor_inf,0*sizeof(char),SEEK_SET);
	}

	/*init line and column,Cpoint*/
	src_line = 1;
	src_column = 0;
	Fpoint = half_buf_size + Tbuf-1;
	current_unit.valuei = 0;
	current_unit.conmplete = true;
	fprintf(fp_dst,"|  Line  |Ns/line |Ns/file |      Type      |  Code  |     Value      |\n");
}

/*底层回退一个字符，直接操作文件buf*/
void BackACharFromCurrentOfSrc()
{
	if(false == haveread){
		printf("Can't back a char,because file read is not start!\n");
		exit(0);
	}
	if(Hbuf == Fpoint||Hbuf ==Tbuf){
		forwordflage = true;
	}
	if(Hbuf == Fpoint){
		Fpoint = Tbuf + half_buf_size -1;
	}
	else {
		Fpoint--;
	}
}

/*底层读取一个字符，控制缓冲区，不过滤无效字符*/
char GetACharFromSrc(){//with preopreator
	if(true == src_file_done){
		return EOF;
	}
	if((half_buf_size + Hbuf-1) == Fpoint )
	{
		if(false == forwordflage){
			fread(Tbuf,sizeof(char),half_buf_size,fp_src);
		}
		else{
			forwordflage = false;
		}
		Fpoint ++;
	}
	else if((half_buf_size + Tbuf-1) == Fpoint)
	{
		if(false == forwordflage){
			if(false == haveread){
				haveread = true;
			}
			fread(Hbuf,sizeof(char),half_buf_size,fp_src);
		}
		else {
			forwordflage = false;
		}
		Fpoint = Hbuf;
	}
	else {
		Fpoint++;
	}
	if(0 == (*Fpoint))
	{
		src_file_done =true;
		return EOF;
	}
	if(CR == *Fpoint||LF == *Fpoint){
		/*if(LF == GetACharFromSrc()){
			src_line_words++;
			//printf("src [line :%d,words :%d]\n",src_line,src_line_words);
			src_line++;
			newline = true;
			src_column = 0;
			src_words +=src_line_words;
			src_line_words =0;
			CRLF = true;
		}
		else {
			BackACharFromCurrentOfSrc();
			src_column++;
		}*/
		char nextch = GetACharFromSrc();
		if(CR == *Fpoint&&LF == nextch){
			src_line++;
			//read LF;
		}
		else if(LF == *Fpoint&&CR == nextch){
			src_line++;
			//read CR;
		}
		else{
			if(LF==nextch||CR==nextch){
				src_line--;
			}
			src_line++;
			BackACharFromCurrentOfSrc();
		}
		//printf("src [line :%d,words :%d]\n",src_line,src_line_words);
		/*if(true == newline){
			src_line++;
		}*/
		newline = true;
		src_column = 0;
		wordend = true;
		/*word_line = src_line;
		src_line++;
		newline = true;
		src_column = 0;
		if(false == wordend&&
			false==sInquotation&&
			false==dInquotation){
			src_line_words++;
			wordend = true;
		}
		src_words +=src_line_words;
		src_line_words =0;*/
		CRLF = true;
	}
	else {
		src_column++;
	}
	//printf("/*			*Fpoint = %d,%c;*/\n ",int(*Fpoint),*Fpoint);
	return *Fpoint;
}

/*高层读取一个字符，过滤无效字符*/
char nextchar(){
	/*remove notes*/
	if(true == dInquotation||true == sInquotation){
		return GetACharFromSrc();
	}
	char cur = GetACharFromSrc();
	wordend = false;
	while(true){
		if('/'== cur){
			char ncur = GetACharFromSrc();
			if('*' == ncur){
				while(true){
					char ret = GetACharFromSrc();
					while('*' != ret&&EOF!= ret){
						ret = GetACharFromSrc();
					}
					if(EOF==ret){
						return EOF;
					}
					else {
						char nret = GetACharFromSrc();
						if(EOF == nret){
							return EOF;
						}
						else if('/'==nret){
							break;
						}
					}
				}
			}
			else if('/' == ncur){
				while(true){
					char ret = GetACharFromSrc();
					while(true != CRLF&&EOF!= ret){
						ret = GetACharFromSrc();
					}
					if(EOF ==ret){
						return EOF;
					}
					else {
						CRLF = false;
						break;
					}
				}
			}
			else{
				untread();
				return cur;
			}
			wordend = true;
			cur = GetACharFromSrc();
		}
		else if(true == CRLF||' '== cur||'	'==cur){
			CRLF = false;
			wordend = true;
			cur = GetACharFromSrc();
		}
		else{
			break;
		}
	}
	if(true == wordend){
		return -2;
	}
	return cur;
}

/*单词解析出错时，读取剩余字符，保持其完整性*/
char GetRestWords(){//error out Inquotations
		/*remove notes*/
	/*if(true == Inquotation){
		return GetACharFromSrc();
	}*/
	char cur = GetACharFromSrc();
	wordend = false;
	while(false == wordend){
		if('/'== cur){
			char ncur = GetACharFromSrc();
			if('*' == ncur){
				while(true){
					char ret = GetACharFromSrc();
					while('*' != ret&&EOF!= ret){
						ret = GetACharFromSrc();
					}
					if(EOF ==ret){
						return EOF;
					}
					else {
						char nret = GetACharFromSrc();
						if(EOF == nret){
							return EOF;
						}
						else if('/'==nret){
							break;
						}
					}
				}
			}
			else if('/' == ncur){
				while(true){
					char ret = GetACharFromSrc();
					while(true != CRLF&&EOF!= ret){
						ret = GetACharFromSrc();
					}
					if(EOF ==ret){
						return EOF;
					}
					else {
						CRLF = false;
					}
				}
			}
			wordend = true;
			cur = GetACharFromSrc();
		}
		else if(true == CRLF||' '== cur||'	'==cur||EOF == cur){
			wordend = true;
			cur = GetACharFromSrc();
		}
		else{
			GetACharFromSrc();
		}
	}
	return cur;
}

/*高层回退一个字符*/
void untread(){
	BackACharFromCurrentOfSrc();
}


/*检查字符类型，函数命名对照DFA*/
bool IsLetter(char ch){//DFA L
	if(ch>='a'&&ch<='z') return true;
	else if(ch>='A'&&ch<='Z') return true;
	else return false;
}
bool IsDigital(char ch){//DFA D
	if(ch>='0'&&ch<='9') return true;
	else return false;
}
bool IsDRemove0(char ch){//DFA D-0；数字去除0
	if(ch>='1'&&ch<='9') return true;
	else return false;
}
bool IsDE(char ch){//DFA DE，八进制数字
	if(ch>='0'&&ch<='7') return true;
	else return false;
}
bool IsDX(char ch){//十六进制数字
	if(ch>='0'&&ch<='9') return true;
	else if(ch>='A'&&ch<='F') return true;
	else if(ch>='a'&&ch<='f') return true;
	else return false;
}
char IsChar(char ch){//是有效字符常量
	char char1;
	int state = 0;
	int tem = 0;
	if('\"'==ch&&true == dInquotation){
		untread();
		dInquotation = false;
		return -3;
	}
	else if('\''==ch&&true == sInquotation){
		untread();
		sInquotation = false;
		return -3;
	}
	while(true){
		switch(state){
		case 0:switch(ch)
			  {
					case '\\':state = 2;
							UnitInitAddAValueElem(ch);
							break;
					default: state =1;
							break;
			   }
			   break;
		case 1:
			UnitInitAddAValueElem(ch);
			return ch;
			break;
		case 2:
			UnitInitAddAValueElem(char1);
			if(IsDE(char1)){
				state = 16;break;
			}
			switch(char1)
			   {
					case '\'':return 39;break;
					case '\\':return 92;break;
					case 'r':return 13;break;
					case 'n':return 10;break;
					case 'f':return 12;break;
					case 't':return 9;break;
					case 'b':return 8;break;
					case 'u':state = 11;break;
					default:return -3;
				}
			 break;
		case 11:UnitInitAddAValueElem(char1);
			for(int i=0;i<4;i++){
					if(!IsDX(char1)){
						untread();
						return tem;
					}
					else {
						UnitInitAddAValueElem(char1);
						int x =0;
						if(ch>='0'&&ch<='9'){
							x= ch - '0';
						}
						else if(ch>='a'&&ch<='f'){
							x= ch - 'a';
						}
						else {
							x= ch - 'A';
						}
						tem = tem*16 +x;
					}
					char1 = nextchar();
					if('\"'==ch&&true == dInquotation){
							dInquotation = false;
							untread();
							return -3;
					}
					else if('\''==ch&&true == sInquotation){
							sInquotation = false;
							untread();
							return -3;
					}
				}
				return tem;
				break;
		case 16:UnitInitAddAValueElem(char1);
			for(int i=0;i<3;i++){
					if(!IsDE(char1)){
						untread();
						return tem;
					}
					else {
						UnitInitAddAValueElem(char1);
						int x =0;
						x = ch -'0';
						tem = tem*8 +x;
					}
					char1 = nextchar();
					if('\"'==ch&&true == dInquotation){
							dInquotation = false;
							untread();
							return -3;
					}
					else if('\''==ch&&true == sInquotation){
							sInquotation = false;
							untread();
							return -3;
					}
				}
				return tem;
				break;
		default :UnitInitAddAValueElem(char1);
			return -3;
			break;
		}
		if(1!=state)char1 = nextchar();
	}
	return -3;
}

/*判断单词是标识符还是关键字*/
void CheckType(){
	int i;
	for(i = 0;i <KEYSIZE;i++){
		if(0 == strcmp(keyword[i],current_unit.value)){
			current_unit.code = 0x103;
			strcpy(current_unit.type,"key");
			break;
		}
	}
	if(KEYSIZE ==i){
		current_unit.code = 0x104;
		strcpy(current_unit.type,"identifier");
	}
}

/*主流程控制，没调用一次，返回一个单词，每个状态严格对应于DFA图*/
bool GetAUnit(){//store in struct Unit current_unit
	int state = 0;
	char char1;
	char1 = nextchar();
	char char1_value = 0;
	while(true){
		/*以下各状态严格对应于DFA图，详细解释请参照图和各块的注释:带有注释“*state”的是终态*/
		switch(state)
		{
		/*first char contrl*/
		case 0:if(IsLetter(char1)||'$'==char1||'_'==char1){
					UnitInitAddAValueElem(char1);
					state = 1;break;
			   }
			   else if(IsDRemove0(char1)){
					UnitInitAddAValueElem(char1);
				   state = 3;break;
			   }
			   else {
				   switch(char1){
						case '0': state=10;	break;
						case '\'': state=15;break;
						case '\"': state=18;break;
						case '=': state= 21;break;
						case '?': state=24;	break;
						case '|': state=26;	break;
						case '&': state=30;	break;
						case '^': state=34;	break;
						case '!': state=37;	break;
						case '<': state=40;	break;
						case '>': state=46;	break;
						case '+': state=55;	break;
						case '-': state=59;	break;
						case '*': state=63;	break;
						case '/': state=66;	break;
						case '%': state=69;	break;
						case '~': state=72;	break;
						case '[': state=73;	break;
						case ']': state=74;	break;
						case '(': state=75;	break;
						case ')': state=76;	break;
						case '.': state=77;	break;
						case ',': state=78;	break;
						case '{': state=79;	break;
						case '}': state=80;	break;
						case ';': state=81;	break;
						case -2:  state=0;  break;
						case -1:  return false; break;//done;
						default : state=82;	break;
				 };
				   if(-2 == char1){
					   untread();//若是无效字符(space,enter,notes)，则重新读取一个有效字符
					   char1 = nextchar();
				   }
				   else{
					   UnitInitAddAValueElem(char1);
				   }
				 break;
			  }
			  break;
		/*key word or Identifier*/
		case 1:
			char1 = nextchar();
			while((IsLetter(char1)||IsDigital(char1)
							||'$'==char1||'_'==char1)){
					UnitInitAddAValueElem(char1);
					char1 = nextchar();
			   }
			state = 2;
			break;
		case 2:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');
			CheckType();//key word or other
			return true;
			break;
			/*int float*/
		case 3:
			char1 = nextchar();
			while(IsDigital(char1)){
					UnitInitAddAValueElem(char1);
					char1 = nextchar();
			   }
			if('.'==char1){
				state = 5;
				UnitInitAddAValueElem(char1);
			}
			else if('e'==char1||'E'==char1){
				state =7;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 4;
			}
			break;
		case 4:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x107;
			strcpy(current_unit.type,"const int");
			return true;
			break;
		case 5:
			char1 = nextchar();
			while(IsDigital(char1)){
					UnitInitAddAValueElem(char1);
					char1 = nextchar();
			}
			if('e'==char1||'E'==char1){
				state =7;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 6;
			}
			break;

		case 6:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x108;
			strcpy(current_unit.type,"const float");
			return true;
			break;

		case 7:char1 = nextchar();
			UnitInitAddAValueElem(char1);
			if('-'==char1) state =8;
			else if(IsDigital(char1)) state = 9;
			else {
				current_unit.error = true;
				strcpy(current_unit.erinfo,"invalid identifier");
				state = 168;//168 is error state
			}
			break;
		case 8:char1 = nextchar();
			UnitInitAddAValueElem(char1);
			if(IsDigital(char1)) state =9;
			else {
				current_unit.error = true;
				strcpy(current_unit.erinfo,"invalid identifier");
				state = 168;//168 is error state
			}
			break;
		case 9:char1 = nextchar();
			while(IsDigital(char1)){
					UnitInitAddAValueElem(char1);
					char1 = nextchar();
			}
			state = 6;
			break;
		case 10:char1 = nextchar();
			if(IsDE(char1)){
				UnitInitAddAValueElem(char1);
				state =11;
			}
			else if('x'==char1||'X'==char1){
				UnitInitAddAValueElem(char1);
				state = 13;//168 is error state
			}

			else {
				state = 4;
			}//state = 168;//"need 完善"
			break;
		case 11:char1 = nextchar();
			while(IsDE(char1)){
					UnitInitAddAValueElem(char1);
					char1 = nextchar();
				}
				state = 12;
				break;
		case 12:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x107;
			strcpy(current_unit.type,"const int8");
			return true;
			break;
		case 13:char1 = nextchar();
			while(IsDX(char1)){
					UnitInitAddAValueElem(char1);
					char1 = nextchar();
				}
				state = 14;
				break;
		case 14:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x107;
			strcpy(current_unit.type,"const int16");
			return true;
			break;
		/*char*/
		case 15:sInquotation = true;
			char1 = nextchar();
			if(-3 ==(char1_value =IsChar(char1))){
				current_unit.error = true;
				current_unit.code = 0x100;
				strcpy(current_unit.erinfo,"invalid char");
				//state = 168;//完善
			}
			//UnitInitAddAValueElem(char1);
			state = 16;
			break;
		case 16:char1 = nextchar();
			if('\''!=char1){
				current_unit.error = true;
				current_unit.code = 0x100;
				strcpy(current_unit.erinfo,"invalid char");
				//UnitInitAddAValueElem(char1);
				while(true == sInquotation){
					char1_value =IsChar(char1);
					if(false == sInquotation){
						char1 = nextchar();
						break;
					}
					char1 = nextchar();
					if(EOF == char1_value){
						break;
					}
				}
				//state = 168;//完善
			}
			if('\''!=char1){				
				current_unit.error = true;
				current_unit.code = 0x100;
				strcpy(current_unit.erinfo,"invalid string end,need a double quote");//state = 168;//完善
			}
			UnitInitAddAValueElem(char1);
			state = 17;
			break;

		case 17:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			if(false == current_unit.error)current_unit.code = 0x106;
			strcpy(current_unit.type,"const char");
			return true;
			break;
		/*string*/
		case 18:dInquotation = true;
			char1 = nextchar();
			if(-3 ==(char1_value =IsChar(char1))){
				if(true == current_unit.error){
					DispalyUnit();
					current_unit.error = false;
				}
				current_unit.error = true;
				current_unit.code = 0x100;
				strcpy(current_unit.erinfo,"invalid char");
				//state = 168;//完善
			}
			//UnitInitAddAValueElem(char1);
			state = 19;
			break;
		case 19:char1 = nextchar();
			while(true == dInquotation){
				if(-3 ==(char1_value =IsChar(char1))){
					if(false == dInquotation){
						char1 = nextchar();
						break;
					}
					current_unit.error = true;
					current_unit.code = 0x100;
					strcpy(current_unit.erinfo,"invalid string");
				}
				//UnitInitAddAValueElem(char1);
				char1 = nextchar();
				if(EOF == char1_value){
					break;
				}
			}
			if('\"'!=char1){
				current_unit.error = true;
				current_unit.code = 0x100;
				strcpy(current_unit.erinfo,"invalid string end,need a double quote");//state = 168;//完善
			}
			UnitInitAddAValueElem(char1);
			state = 20;
			break;

		case 20:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			if(false == current_unit.error)current_unit.code = 0x109;
			strcpy(current_unit.type,"const string");
			return true;
			break;
		/* =,== */
		case 21:char1 = nextchar();
			if('='==char1){
				state = 22;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 23;
			}			
			break;
		case 22:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x117;
			strcpy(current_unit.type,"==");
			return true;
			break;

		case 23:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x110;
			strcpy(current_unit.type,"=");
			return true;
			break;

		/*?:*/
		case 24:char1 = nextchar();
			if(':'!=char1){
				current_unit.error = true;
				strcpy(current_unit.erinfo,"unknown operator");
				state = 168;
			}
			UnitInitAddAValueElem(char1);
			state = 25;
			break;

		case 25:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x111;
			strcpy(current_unit.type,"?:");
			return true;
			break;

		/* |,||,|= */
		case 26:char1 = nextchar();
			if('|'==char1){
				state = 27;
				UnitInitAddAValueElem(char1);
			}
			else if('='==char1){
				state = 28;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 29;
			}			
			break;
		case 27:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x112;
			strcpy(current_unit.type,"||");
			return true;
			break;

		case 28:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x110;
			strcpy(current_unit.type,"|=");
			return true;
			break;

		case 29:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x114;
			strcpy(current_unit.type,"|");
			return true;
			break;

		/* &,&&,&= */
		case 30:char1 = nextchar();
			if('&'==char1){
				state = 31;
				UnitInitAddAValueElem(char1);
			}
			else if('='==char1){
				state = 32;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 33;
			}			
			break;
		case 31:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x113;
			strcpy(current_unit.type,"&&");
			return true;
			break;

		case 32:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x110;
			strcpy(current_unit.type,"&=");
			return true;
			break;

		case 33:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x116;
			strcpy(current_unit.type,"&");
			return true;
			break;

		/* ^,^= */
		case 34:char1 = nextchar();
			if('='==char1){
				state = 35;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 36;
			}			
			break;
		case 35:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x110;
			strcpy(current_unit.type,"^=");
			return true;
			break;

		case 36:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x115;
			strcpy(current_unit.type,"^");
			return true;
			break;

		/* !,!= */
		case 37:char1 = nextchar();
			if('='==char1){
				state = 38;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 39;
			}			
			break;
		case 38:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x117;
			strcpy(current_unit.type,"!=");
			return true;
			break;

		case 39:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x11c;
			strcpy(current_unit.type,"!");
			return true;
			break;

		/* <,<=,<<,<<= */
		case 40:char1 = nextchar();
			if('='==char1){
				state = 41;
				UnitInitAddAValueElem(char1);
			}
			else if('<'==char1){
				state = 42;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 45;
			}			
			break;
		case 41:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x118;
			strcpy(current_unit.type,"<=");
			return true;
			break;
		
		case 42:char1 = nextchar();
			if('='==char1){
				state = 43;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 44;
			}			
			break;
		case 43:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x110;
			strcpy(current_unit.type,"<<=");
			return true;
			break;

		case 44:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x119;
			strcpy(current_unit.type,"<<");
			return true;
			break;

		case 45:// *state
			untread();
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x118;
			strcpy(current_unit.type,"<");
			return true;
			break;

		/* >,>=,>>,>>= ,>>>,>>>=*/
		case 46:char1 = nextchar();
			if('='==char1){
				state = 47;
				UnitInitAddAValueElem(char1);
			}
			else if('>'==char1){
				state = 48;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 54;
			}			
			break;
		case 47:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x118;
			strcpy(current_unit.type,">=");
			return true;
			break;
		
		case 48:char1 = nextchar();
			if('='==char1){
				state = 49;
				UnitInitAddAValueElem(char1);
			}
			else if('>'==char1){
				state = 50;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 53;
			}			
			break;
		case 49:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x110;
			strcpy(current_unit.type,">>=");
			return true;
			break;

		case 50:char1 = nextchar();
			if('='==char1){
				state = 51;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 52;
			}			
			break;

		case 51:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x110;
			strcpy(current_unit.type,">>>=");
			return true;
			break;
		case 52:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x119;
			strcpy(current_unit.type,">>>");
			return true;
			break;

		case 53:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x119;
			strcpy(current_unit.type,">>");
			return true;
			break;

		case 54:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x118;
			strcpy(current_unit.type,">");
			return true;
			break;

		/* +,++,+= */
		case 55:char1 = nextchar();
			if('+'==char1){
				state = 56;
				UnitInitAddAValueElem(char1);
			}
			else if('='==char1){
				state = 57;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 58;
			}			
			break;
		case 56:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x11c;
			strcpy(current_unit.type,"++");
			return true;
			break;

		case 57:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x110;
			strcpy(current_unit.type,"+=");
			return true;
			break;

		case 58:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x11a;
			strcpy(current_unit.type,"+");
			return true;
			break;

		/* -,--,-= */
		case 59:char1 = nextchar();
			if('-'==char1){
				state = 60;
				UnitInitAddAValueElem(char1);
			}
			else if('='==char1){
				state = 61;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 62;
			}			
			break;
		case 60:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x11c;
			strcpy(current_unit.type,"--");
			return true;
			break;

		case 61:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x110;
			strcpy(current_unit.type,"-=");
			return true;
			break;

		case 62:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x11a;
			strcpy(current_unit.type,"-");
			return true;
			break;

		/* *,*= */
		case 63:char1 = nextchar();
			if('='==char1){
				state = 64;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 65;
			}			
			break;
		case 64:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x110;
			strcpy(current_unit.type,"*=");
			return true;
			break;

		case 65:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x11b;
			strcpy(current_unit.type,"*");
			return true;
			break;

		/* /,/= */
		case 66:char1 = nextchar();
			if('='==char1){
				state = 67;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 68;
			}			
			break;
		case 67:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x110;
			strcpy(current_unit.type,"/=");
			return true;
			break;

		case 68:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x11b;
			strcpy(current_unit.type,"/");
			return true;
			break;

		/* %,%= */
		case 69:char1 = nextchar();
			if('='==char1){
				state = 70;
				UnitInitAddAValueElem(char1);
			}
			else {
				state = 71;
			}			
			break;
		case 70:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x110;
			strcpy(current_unit.type,"%=");
			return true;
			break;

		case 71:untread();// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');

			current_unit.code = 0x11b;
			strcpy(current_unit.type,"%");
			return true;
			break;
		/*~*/
		case 72:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x11c;
			strcpy(current_unit.type,"~");
			return true;
			break;
		/*[*/
		case 73:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x11d;
			strcpy(current_unit.type,"[");
			return true;
			break;
		/*]*/
		case 74:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x11d;
			strcpy(current_unit.type,"]");
			return true;
			break;
		/*(*/
		case 75:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x11d;
			strcpy(current_unit.type,"(");
			return true;
			break;
		/*)*/
		case 76:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x11d;
			strcpy(current_unit.type,")");
			return true;
			break;
		/*.*/
		case 77:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x11d;
			strcpy(current_unit.type,".");
			return true;
			break;
		/*~*/
		case 78:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x120;
			strcpy(current_unit.type,",");
			return true;
			break;
		/*{*/
		case 79:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x121;
			strcpy(current_unit.type,"{");
			return true;
			break;
		/*}*/
		case 80:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x121;
			strcpy(current_unit.type,"}");
			return true;
			break;
		/*;*/
		case 81:// *state
			src_line_words++;
			UnitInitAddAValueElem('\0');//

			current_unit.code = 0x122;
			strcpy(current_unit.type,";");
			return true;
			break;
		
		case 168 ://read rest word set error;sentence stream error;
			src_line_words++;
			current_unit.code = 0x100;
			GetRestWords();
			UnitInitAddAValueElem('\0');
			if(EOF == char1){
					return false;
				}
			else return true;
			break;

		default ://state 82;main stream erorr;
			//state = 0;
			src_line_words++;
			current_unit.code = 0x100;
			strcpy(current_unit.erinfo,"unknown char");
			GetRestWords();
			UnitInitAddAValueElem('\0');
			if(EOF == char1){
					return false;
				}
			else return true;
			break;
		}///switch end


		/*if(true == newline){
			newline = false;
			src_line_words = 0;
		}*/
	}//while end

}
void scanner_start(){
	InitWork();//全局初始化
	UnitInit();//初始化单元
}
void scanner_end(){
		/*关闭文件*/
	fclose(fp_src);
	fclose(fp_dst);
	fclose(fp_mor_inf);
}

/*Get a unin store in current_unit*/
bool scanner_get_unin(){
	UnitInit();//初始化单元
	bool ret = GetAUnit();
	if(true == current_unit.error){
		current_unit.code = 0x100;
	}
	DispalyUnit();//输出
	if(false == ret && 0 == current_unit.valuei){
		current_unit.code = 0x130;
	}
	return ret;
}

void RunScanner(){
	InitWork();//全局初始化
	UnitInit();//单元初始化
	while(GetAUnit()){//获取单词
		if(true == current_unit.error){
			current_unit.code = 0x100;
		}
		DispalyUnit();
		UnitInit();//单元初始化
	}
	/*关闭文件*/
	fclose(fp_src);
	fclose(fp_dst);
	fclose(fp_mor_inf);
}
