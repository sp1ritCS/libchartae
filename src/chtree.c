#include "chtree.h"

struct _ChTree {
	GObject parent_instance;
	
	GPtrArray* nodes;
};

G_DEFINE_TYPE (ChTree, ch_tree, G_TYPE_OBJECT)

static void ch_tree_object_finalize(GObject* object) {
	ChTree* self = CH_TREE(object);
	g_ptr_array_unref(self->nodes);
	
	G_OBJECT_CLASS(ch_tree_parent_class)->finalize(object);
}

static void ch_tree_class_init(ChTreeClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	
	object_class->finalize = ch_tree_object_finalize;
}

static void ch_tree_init(ChTree* self) {
	self->nodes = g_ptr_array_new_with_free_func(g_object_unref);
}

ChTree* ch_tree_new(void) {
	return g_object_new(CH_TYPE_TREE, NULL);
}

guint ch_tree_get_nodes(ChTree* self, ChNode*** nodes) {
	g_return_val_if_fail(CH_IS_TREE(self), 0);
	*nodes = (ChNode**)self->nodes->pdata;
	return self->nodes->len;
}

void ch_tree_add_node(ChTree* self, ChNode* node) {
	g_return_if_fail(CH_IS_TREE(self));
	g_return_if_fail(CH_IS_NODE(node));
	
	g_ptr_array_add(self->nodes, g_object_ref(node));
}

void ch_tree_debug_print(ChTree* self) {
	g_return_if_fail(CH_IS_TREE(self));
	g_print("%s {", G_OBJECT_TYPE_NAME(self));
	for (guint i = 0; i < self->nodes->len; i++) {
		ch_node_print_debug(self->nodes->pdata[i]);
		if (i+1 < self->nodes->len)
			g_print(", ");
	}
	g_print("}\n");
}
