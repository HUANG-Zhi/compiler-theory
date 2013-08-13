#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define CR 13
#define LF	10
#define KEYSIZE 50
#define KEYWORDLEN 20
#define UNITVALUESIZE 256//单元存储字符的最大值
/*关键字*/
const char keyword[KEYSIZE][KEYWORDLEN]={
					"abstract","boolean","break","byte","case","catch","char","class",
					"const","continue","default","do","double","else","extends","false",
					"final","finally","float","for","goto","if","implements","import",
					"instanceof","int","interface","long","native","new","null","package",
					"private","protected","public","return","short","static","super","switch",
					"synchronized","this","throw","throws","transient","true","try","void",
					"volatile","while"
					};
/*functions*/

/*初始化current_unit*/
void UnitInit();

/*给current_unit.value增加一个字符*/
void UnitInitAddAValueElem(char ch);

/*辅助输出函数，以size-1宽度输出str，超出时采用注释的方法，只输出索引号*/
void FormatPrint(char str[],int size);

/*输出单元*/
void DispalyUnit();

/*全局初始化*/
void InitWork();

/*底层回退一个字符，直接操作文件buf*/
void BackACharFromCurrentOfSrc();

/*底层读取一个字符，控制缓冲区，不过滤无效字符*/
char GetACharFromSrc();

/*高层读取一个字符，过滤无效字符*/
char nextchar();

/*单词解析出错时，读取剩余字符，保持其完整性*/
char GetRestWords();

/*高层回退一个字符*/
void untread();
/*检查字符类型，函数命名对照DFA*/
bool IsLetter(char ch);
bool IsDigital(char ch);
bool IsDRemove0(char ch);
bool IsDE(char ch);
bool IsDX(char ch);
char IsChar(char ch);

/*判断单词是标识符还是关键字*/
void CheckType();

/*主流程控制，没调用一次，返回一个单词，每个状态严格对应于DFA图*/
bool GetAUnit();