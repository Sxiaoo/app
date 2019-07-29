#include <stdio.h>
#include "parse_file.h"
#include "tree.h"
#include <stdlib.h>
#include <string.h>


int expand_capacity(struct alloc_t *ptr, int idx)
{
	int new_capacity;
	struct node_t **node_ver_ptr;

	if(ptr->capacity >= idx + 1)
	{
		return 0;
	}

	if(ptr->capacity < 16)
	{
		new_capacity = 16;
	}
	else
	{
		new_capacity = ptr->capacity;
	}

	while(new_capacity < idx + 1)
	{
		new_capacity *= 2;
	}

	node_ver_ptr = (struct node_t **)malloc(new_capacity * (int)sizeof(struct node_t *));
	if(node_ver_ptr == NULL)
	{
		return 1;
	}

	memset(node_ver_ptr, 0 ,new_capacity * (int)sizeof(struct node_t *));
	memmove(node_ver_ptr, ptr->vec, (ptr->capacity)* sizeof(struct node_t *));
	ptr->capacity = new_capacity;

	free(ptr->vec);
	ptr->vec = node_ver_ptr;
	
	return 0;
}


int insert_version(dpi_data_t *dpi, struct node_t *root, struct node_t *node_ptr)
{
	int ret;
	struct node_t *new_node_ptr;
	int app_id = (int)node_ptr->app.app_id;
	int group_idx = (int)GROUP_INDEX(app_id);
	int app_idx = (int)APP_INDEX(app_id);
	int sub_app_idx = SUB_APP_INDEX(app_id);
	int version_dex = VERSION_INDEX(app_id);
	struct node_t *app_ptr;
	struct node_t *sub_app_ptr;
	struct node_t *group_ptr = get_children(root, (group_idx -1));

	if(group_ptr == NULL)
	{
		new_node_ptr = allocate_node(DEFFAULT_CHILDREN_NUM);
		if(new_node_ptr == NULL)
		{
			return 1;
		}

		new_node_ptr->app.app_id = (int)GROP_ID(app_id);

		strncpy(new_node_ptr->app.name, node_ptr->app.name, 63);

		ret = insert_group(dpi,root,new_node_ptr);
	}
}

int insert_group(dpi_data_t *dpi, struct node_t *root, struct node_t *node_ptr)
{
	int group_idx = GROUP_INDEX(node_ptr->app.app_id);
	return insert_node(dpi, root, node_ptr, group_idx - 1);
}

int insert_node(dpi_data_t *dpi, struct node_t *parent, struct node_t *child, int idx)
{
	int ret;

	if(NULL == parent || NULL == child || 0XFFFFFFFF == idx)
	{
		return 1;
	}

	if(parent->children.capacity >= idx + 1)
	{
		if(parent->children.vec[idx] != NULL)
		{
			printf("有重复的appid");
		}
	}
	else
	{
		if((ret = expand_capacity(&parent->children, idx)) == 1)
		{
			return 1;
		}
	}

	parent->children.vec[idx] = child;
	parent->children.size++;
	child->app.parent_id = parent->app.app_id;
}

struct node_t *get_children(struct node_t *parent, int id)
{
	if(parent->children.capacity < id +1)
	{
		return NULL;
	}
	
	return parent->children.vec[id];
}

int expand_capacity(struct alloc_t *ptr, int idx)
{
	int new_capacity;
	struct node_t **node_ver_ptr;

	if(ptr->capacity >= idx + 1)
	{
		return 0;
	}

	if(ptr->capacity < 16)
	{
		new_capacity = 16;
	}
	else
	{
		new_capacity = ptr->capacity;
	}

	while(new_capacity < idx + 1)
	{
		new_capacity *= 2;
	}

	node_ver_ptr = (struct node_t **)malloc(new_capacity * (int)sizeof(struct node_t *));
	if(node_ver_ptr == NULL)
	{
		return 1;
	}

	memset(node_ver_ptr, 0 ,new_capacity * (int)sizeof(struct node_t *));
	memmove(node_ver_ptr, ptr->vec, (ptr->capacity)* sizeof(struct node_t *));
	ptr->capacity = new_capacity;

	free(ptr->vec);
	ptr->vec = node_ver_ptr;
	
	return 0;
}


