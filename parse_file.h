#ifndef __PARSE_FILE_H_
#define __PARSE_FILE_H_

#define MAX_BUFF_LEN 64
#define DEFFAULT_CHILDREN_NUM 16

#define UNKNOWN_DFA_ID ((int)~0)

#define ULONG_MAX1 (~0UL)
#define GROUP_MASK	0x00FF0000U
#define APP_MASK	0x0000FF00U
#define SUB_APP_MASK	0x000000FFU
#define	VERSION_MASK	0xFF000000U

#define GROP_ID(id)		((id) & (GROUP_MASK))
#define APP_ID(id)		((id) & (APP_MASK))
#define SUB_APP_ID(id)	((id) & (SUB_APP_MASK))
#define VERSION_ID(id)  ((id) & (VERSION_MASK))

#define GROUP_INDEX(id)		((GROP_ID(id)) >> 16)
#define APP_INDEX(id)		((APP_ID(id)) >> 8)
#define SUB_APP_INDEX(id)	(SUB_APP_ID(id))
#define VERSION_INDEX(id)	((VERSION_ID(id)) >> 24)

int Ini_File_Read(char *pcFileName, char *Line, int uLen);
int GetLineFromBuffer(char *pcsrc, int uSrcLen, int *puSrcOffset, char *pcDst, int uDstLen);
char *getFirstNOnBlankChar(char *buffer, int len);
char *getFirstNOnBlankCharRev(char * buffer, int len);


enum APP_ATTR_TYPE
{
	APP_UNKNOWN = 0,
	APP_NAME,
	APP_ZH_NAME,
	APP_VERSION,
	APP_GROUP,
	APP_APP_ID,
	APP_ATTR_MAX
};

struct app_t
{
	char name[MAX_BUFF_LEN];
	char zh_name[MAX_BUFF_LEN];
	char version[MAX_BUFF_LEN];
	char group[MAX_BUFF_LEN];
	int app_id;
	int parent_id;
};

struct alloc_t
{
	int size;
	int capacity;
	struct node_t **vec;
};

struct app_node_t
{
	struct app_t app;
	struct alloc_t children;
};

struct node_t
{
	struct app_t app;
	struct alloc_t children;
};

typedef struct _dpi_data
{
	struct node_t app_root;
}dpi_data_t;

#endif

