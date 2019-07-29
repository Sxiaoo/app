#ifndef __TREE_H_
#define __TREE_H_

struct node_t *get_children(struct node_t *parent, int id);
int expand_capacity(struct alloc_t *ptr, int idx);
int insert_node(dpi_data_t *dpi, struct node_t *parent, struct node_t *child, int idx);
int insert_group(dpi_data_t *dpi, struct node_t *root, struct node_t *node_ptr);
int insert_version(dpi_data_t *dpi, struct node_t *root, struct node_t *node_ptr);
struct node_t *allocate_node(int capacity);
int expand_capacity(struct alloc_t *ptr, int idx);


#endif

