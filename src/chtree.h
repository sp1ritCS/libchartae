#ifndef __CHTREE_H__
#define __CHTREE_H__

#include <glib-object.h>
#include <nodes/chnode.h>

G_BEGIN_DECLS

#define CH_TYPE_TREE (ch_tree_get_type())
G_DECLARE_FINAL_TYPE (ChTree, ch_tree, CH, TREE, GObject)

ChTree* ch_tree_new(void);

guint ch_tree_get_nodes(ChTree* self, ChNode*** nodes);
void ch_tree_add_node(ChTree* self, ChNode* node);

void ch_tree_debug_print(ChTree* self);

G_END_DECLS

#endif // __CHTREE_H__
