#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include "parse_file.h"
#include "mysql.h"


#define FILE_PATH "dpi_ini"
#define BUG_LEN_MAX 1024

int g_u32SupportVA = 0;
char cValue_Buffer[512];
dpi_data_t *g_dpi;


int main()
{
	FILE *pFile = NULL;
	int Ret;
	struct stat statbuf;
	int BufLen;
	int IniBuffLen;
	off_t tFileLen = 0;
	char *pcIniBuff = NULL;
	int Bufoffset = 0;
	char arrcLine[BUG_LEN_MAX];
	MYSQL *mysql;
	int status;
	MYSQL_RES *result;
	unsigned int num_fields;
	int i;
	MYSQL_ROW row;

	g_dpi = malloc(sizeof(dpi_data_t));
	if(g_dpi == NULL)
	{
		printf("malloc g_dpi failed");
		return -1;
	}
	
	init_root_node(g_dpi, &g_dpi->app_root);
	
	if((pFile = fopen(FILE_PATH,"r")) == NULL)
	{
		printf("open file err in main\r\n");
		goto ERROR;
	}
	
	Ret = stat(FILE_PATH, &statbuf);
	if(Ret != 0)
	{
		goto ERROR;
	}

	tFileLen = statbuf.st_size;
	pcIniBuff = (char *) malloc(tFileLen);
	if(NULL == pcIniBuff)
	{
		printf("create pcIniBuff error");
		goto ERROR;
	}

	memset(pcIniBuff, 0, tFileLen);
	
	//1:初始化连接句柄
	mysql = mysql_init(NULL);
	if(NULL == mysql)
	{
		printf("mysql init failed\n");
		return -1;
	}

	//2.实际进行连接
	mysql = mysql_real_connect(mysql, "127.0.0.1", "root", "root123", "app", 0, NULL, 0);
	if(NULL == mysql)
	{
		printf("connect failed\n");
		return -1;
	}
	
	status = mysql_query(mysql,"drop table if exists app");
	if(status)
	{
		printf("delete table failed\n");
		
		goto ERROR;
	}
	
	status = mysql_query(mysql,"CREATE TABLE app(\
  								app int(255) NOT NULL AUTO_INCREMENT,\
  								name varchar(255) NULL,\
  								zh_name varchar(255) NULL,\
  								appid int(255) NULL,\
  								PRIMARY KEY (app)\
								)ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1");
	if(status)
	{
		printf("create tables failed\n");
		
		goto ERROR;
	}
	
	if((IniBuffLen = Ini_File_Read(FILE_PATH, pcIniBuff, tFileLen)) < 0)
	{
		goto ERROR;
	}

	while(GetLineFromBuffer(pcIniBuff, IniBuffLen, &Bufoffset, arrcLine, BUG_LEN_MAX) > 0)
	{
		/*文件结束*/
		if(0 == strlen(arrcLine))
		{
			continue;
		}

		if(0 == IsNewApp(arrcLine, strlen(arrcLine)))
		{
			if(0 != ParseOneApp(g_dpi, pcIniBuff, IniBuffLen, &Bufoffset, mysql))
			{
				goto ERROR;
			}
		}
	}

#if 0
	/*从数据库中查找数据*/
	status = mysql_query(mysql,"select *from app");
	if(status)
	{
		printf("select error\n");
		
		goto ERROR;
	}
	
	do
	{
		result = mysql_store_result(mysql);		//返回数据
		if(result)
		{
			num_fields = mysql_num_fields(result);		//返回数据中的列数
			
			while((row = mysql_fetch_row(result)))		//获取数据中的下一行
			{
				unsigned long *lengths;
				lengths = mysql_fetch_lengths(result);	//返回数据的长度
				
				#if 0
				//全部打印
				for(i = 0; i < num_fields; i++)
				{
					printf("%s\t", row[i]);		
				}
				printf("\n");
				#endif
				printf("id:%s\t", row[0]);
				printf("name:%s\t", row[1]);
				printf("zh_name:%s\t", row[2]);
				printf("appid:%d\n", row[3]);
				
			}
		}
		else
		{
			if(mysql_field_count(mysql) == 0)	//返回最近语句结果的列数
			{
				printf("lld rows affected\n");
				break;
			}
		}
		if((status = mysql_next_result(mysql)) > 0)
		{
			printf("could not execture statemement\n");
		}
	}while(status == 0);
#endif

ERROR:
	if(pFile != NULL)
	{
		fclose(pFile);
	}
	if(pcIniBuff != NULL)
	{
		free(pcIniBuff);
	}
	
	mysql_close(mysql);
	return 0;
}

int Ini_File_Read(char *pcFileName, char *Line, int uLen)
{
	FILE *pFile = NULL;
	int itmp = 0;
	int FileLen = 0;
	
	if((pFile = fopen(pcFileName, "r")) == NULL)
	{
		printf("open file error in Ini_File_Read\r\n");
		return -1;
	}

	fread(Line, uLen, 1, pFile);
	fseek(pFile, 0L, SEEK_END);
	FileLen = ftell(pFile);

	fclose(pFile);
	
	return FileLen;
}

int GetLineFromBuffer(char *pcsrc, int uSrcLen, int *puSrcOffset, char *pcDst, int uDstLen)
{
	int uidx;
	int uOrioffset = *puSrcOffset;

	for(uidx = 0; uidx < uDstLen -1 && *puSrcOffset < uSrcLen; uidx++, (*puSrcOffset)++)
	{
		if(pcsrc[*puSrcOffset] == '\r' || pcsrc[*puSrcOffset] == '\n')
		{
			break;
		}
		else
		{
			pcDst[uidx] = pcsrc[*puSrcOffset];
		}
	}
	pcDst[uidx] = '\0';
	
	/*跳过\r\n*/
	while(*puSrcOffset < uSrcLen - 1 && (pcsrc[*puSrcOffset] == '\r' || pcsrc[*puSrcOffset] == '\n'))
	{
		(*puSrcOffset)++;
	}

	return (*puSrcOffset) - uOrioffset;
}

int IsNewApp(char *pucBuff, int uLen)
{
	char *cptr;
	cptr = getFirstNOnBlankChar(pucBuff,uLen);
	if(!cptr || *cptr != '[')
	{
		return -1;
	}
	
	cptr = getFirstNOnBlankCharRev(pucBuff, uLen);
	if(!cptr || *cptr != ']')
	{
		return -1;
	}

	return 0;
}

int IsBlankChar(char c)
{
	if(c == ' ' || c == '\t')
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
char *getFirstNOnBlankChar(char *buffer, int len)
{
	int idx = 0;

	for(; idx < len; idx++)
	{
		if(IsBlankChar(buffer[idx]))
		{
			continue;
		}
		else
		{
			return buffer + idx;
		}
	}

	return NULL;
}

char *getFirstNOnBlankCharRev(char * buffer, int len)
{
	int idx = len - 1;

	for(; idx >= 0; idx++)
	{
		if(IsBlankChar(buffer[idx]))
		{
			continue;
		}
		else
		{
			return buffer + idx;
		}
	}

	return NULL;
}

int allocate_node_imp(struct node_t *node_ptr, int capacity)
{
	struct node_t **node_ver_ptr = (struct node_t **)malloc(capacity * sizeof(struct node_t *));
	if(NULL == node_ver_ptr)
	{
		free(node_ptr);
		return 1;
	}

	memset(node_ver_ptr, 0, capacity * sizeof(struct node_t *));

	node_ptr->children.vec = node_ver_ptr;
	node_ptr->children.capacity = capacity;
	node_ptr->children.size = 0;

	return 0;
}

struct node_t *allocate_node(int capacity)
{
	struct node_t *node_ptr = malloc(sizeof(struct node_t));
	if(NULL == node_ptr)
	{
		return NULL;
	}

	memset(node_ptr, 0, sizeof(struct node_t));

	allocate_node_imp(node_ptr, capacity);

	return node_ptr;
	
}

enum APP_ATTR_TYPE GetAttrType(char *buffer, int len)
{
	char *cptr;
	int ret;

	cptr = getFirstNOnBlankChar(buffer, len);

	if((ret = strncmp(cptr, "name", 4)) == 0)
	{
		return APP_NAME;
	}
	
	if((ret = strncmp(cptr, "zh_name", 7)) == 0)
	{
		return APP_ZH_NAME;
	}
	
	if((ret = strncmp(cptr, "version", 7)) == 0)
	{
		return APP_VERSION;
	}

	if((ret = strncmp(cptr, "group", 5)) == 0)
	{
		return APP_GROUP;
	}
		
	if((ret = strncmp(cptr, "app_id", 6)) == 0)
	{
		return APP_APP_ID;
	}
	return APP_ATTR_MAX;
}

char *GetAttrValue(char *buffer, int len)
{
	char *pos, *pos_post;

	memset(cValue_Buffer, 0, 512);

	pos = strchr(buffer, '=');
	if(!pos)
	{
		goto ERROR;
	}

	pos = getFirstNOnBlankChar(pos + 1, len - (pos - buffer + 1));
	if(!pos)
	{
		goto ERROR;
	}

	pos_post = getFirstNOnBlankCharRev(buffer, len);

	if(*pos == '"')
	{
		if(pos_post > pos && *pos_post == '"')
		{
			strncpy(cValue_Buffer, pos + 1, pos_post - pos -1);
			cValue_Buffer[pos_post - pos -1] = '\0';
			goto SUCCESS;
		}
		else
		{
			goto ERROR;
		}
	}

	if(*pos == '/')
	{
		if(pos_post > pos && *pos_post == '/')
		{
			strncpy(cValue_Buffer, pos + 1, pos_post - pos -1);
			cValue_Buffer[pos_post - pos - 1] = '\0';
			goto SUCCESS;
		}
		else
		{
			goto ERROR;
		}
	}

	strncpy(cValue_Buffer, pos, pos_post - pos + 1);
	cValue_Buffer[pos_post - pos + 1] = '\0';
	
SUCCESS:
	return cValue_Buffer;
	
ERROR:
	return NULL;
}

unsigned long strtoul_parse(const char *nptr, char **endptr, int base)
{
	const char *s = nptr;
	unsigned long acc;
	unsigned char c;
	unsigned long cutoff;
	int neg = 0, any, cutlim;

	do
	{
		c = *s++;
	}while(isspace(c));	//isspacel 判断输入字符是否为回车、空格、制表符

	if(c == '-')
	{
		neg = 1;
		c = *s++;
	}
	else if(c == '+')
	{
		c = *s++;
	}
	
	if((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X'))
	{
		c = s[1];
		s += 2;
		base = 16;
	}
	
	if(base == 0)
	{
		base = c == '0' ? 8 : 10;
	}

	cutoff = (unsigned long) ULONG_MAX1 / (unsigned long)base;
	cutlim = (unsigned long) ULONG_MAX1 & (unsigned long)base;

	for(acc =0, any = 0; ; c = *s++)
	{
		if(!isascii(c))	//判断是否为asciil码，也就是是否在0~127之间
		{
			break;
		}
		if(isdigit(c))		//判断是否为十进制字符
		{
			c -= '0';
		}
		else if (isalpha(c))	//判断是否为英文字母
		{
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		}
		else
		{
			break;
		}
		if(c >= base)
		{
			break;
		}
		if(any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
		{
			any = -1;
		}
		else
		{
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if(any < 0)
	{
		acc = ULONG_MAX1;
	}
	else if(neg)
	{
		acc = -acc;
	}
	if(endptr != 0)
	{
		*endptr = (char *)(any ? s - 1 : nptr);
	}
	return (acc);
}

/*编码格式转换*/
/* to:原编码、from:要转换的编码*/
unsigned long Code_conversion(const char *to, const char *from, char *in, unsigned long in_len, char *out, unsigned long out_len)
{
	unsigned int orig_out_len = out_len;
	unsigned long siRet;
	iconv_t cd = iconv_open(to, from);

	if(cd == (iconv_t)-1)
	{
		return 0;
	}

	siRet = iconv(cd, &in, &in_len, &out, &out_len);

	iconv_close(cd);

	if(((size_t)-1) == siRet)
	{
		return 0;
	}

	return orig_out_len - out_len;
}

int insert_root(dpi_data_t *dpi, struct node_t *root, struct node_t *node_ptr)
{
	int app_id = (int)node_ptr->app.app_id;
	int group_idx = (int)GROUP_INDEX(app_id);
	int app_idx = (int)APP_INDEX(app_id);
	int sub_app_idx = SUB_APP_INDEX(app_id);
	int version_dex = VERSION_INDEX(app_id);

	if(version_dex != 0)
	{
		return;
	}
	if(sub_app_idx != 0)
	{
		return;
	}
	if(app_idx != 0)
	{
		return;
	}
	if(group_idx != 0)
	{
		return;
	}
	else
	{
		return 1;
	}
	
}

int ParseOneApp(dpi_data_t *pstDpi, char *pcIniBuff, int uIniBuffLen, int *puBffoffset, MYSQL *mysql)
{
	char sql[512] = {0};
	char arrcLine[BUG_LEN_MAX];
	enum APP_ATTR_TYPE enuattr_type;
	char *pcValue = NULL;
	int uVersionIdx = 0;
	char pstBuff[8];
	struct node_t *pstNode;
	struct app_t *pstApp;
	char *pcIndex;
	int uCount;
	int uSupportVAApp = g_u32SupportVA;
	
	pstNode = allocate_node(DEFFAULT_CHILDREN_NUM);
	if(NULL == pstNode)
	{
		return 1;
	}

	pstApp = &pstNode->app;

START:
	while((uCount = GetLineFromBuffer(pcIniBuff, uIniBuffLen, puBffoffset, arrcLine, BUG_LEN_MAX)) > 0)
	{
		/*文件结束*/
		if(strlen(arrcLine) == 0)
		{
			break;
		}

		pcIndex = arrcLine;

		while(*pcIndex == ' ' || *pcIndex == '\t')
		{
			pcIndex++;
			if(pcIndex - arrcLine >= strlen(arrcLine))
			{
				/*这一行没有任何东西*/
				goto START;
			}
		}
		/*无内容或者注释，忽略*/
		if(*pcIndex == '\r' || *pcIndex == '\n' || *pcIndex == '#')
		{
			continue;
		}

		/*遇到新的app，停止分析并回退一行*/
		if(IsNewApp(pcIndex, strlen(pcIndex)) == 0)
		{
			*puBffoffset = *puBffoffset - uCount;
			break;
		}
		enuattr_type = GetAttrType(pcIndex, strlen(pcIndex));
		if(APP_ATTR_MAX == enuattr_type)
		{
			goto START;
		}
		pcValue = (char*)GetAttrValue(pcIndex, strlen(pcIndex));
		switch(enuattr_type)
		{
			case APP_NAME:
				if(pcValue)
				{
					strncpy(pstApp->name, pcValue, 63);
				}
				break;
			case APP_ZH_NAME:
				if(pcValue)
				{
					strncpy(pstApp->zh_name, pcValue, 63);	
				}
				break;
			case APP_VERSION:
				if(pcValue)
				{
					strncpy(pstApp->version, pcValue, 63);
				}
				break;
			case APP_GROUP:
				if(pcValue)
				{
					strncpy(pstApp->group, pcValue, 63);
				}
				break;
			case APP_APP_ID:
				if(pcValue)
				{
					pstApp->app_id = (int)strtoul_parse(pcValue, NULL, 16);
				}
				else
				{
					pstApp->app_id = UNKNOWN_DFA_ID;
				}
				break;
		}
	}
	
	mysql_query(mysql, "set names 'utf8'");		//设置编码格式
	memset(sql, 0, 512);
	sprintf(sql, "insert into app(name,zh_name,appid) values('%s','%s',%d)", pstApp->name, pstApp->zh_name, pstApp->app_id );
	//3.插入数据
	if(0 != mysql_query(mysql, sql))
	{
		printf("insert data failed\n");
	}

	insert_root(pstDpi, &pstDpi->app_root, pstNode);
	
	return 0;
}


int init_root_node(dpi_data_t *dpi, struct node_t *root)
{
	allocate_node_imp(root, DEFFAULT_CHILDREN_NUM);
	
	root->app.app_id = 0;
	strncpy(root->app.name, "app_root", 64);
	strncpy(root->app.zh_name, "app_root", 64);

	return 0;
}

