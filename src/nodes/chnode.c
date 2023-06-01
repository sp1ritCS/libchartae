#include "chnode.h"

G_DEFINE_ABSTRACT_TYPE (ChNode, ch_node, G_TYPE_INITIALLY_UNOWNED)

static void ch_node_class_init(ChNodeClass* class) {
	class->get_line = NULL,
	class->render_line = NULL;
	class->print_debug = NULL;
}

static void ch_node_init(ChNode*) {}

ChMeasurementLine* ch_node_get_line(ChNode* self, gint x, gint width, gpointer* iter) {
	g_return_val_if_fail(CH_IS_NODE(self), NULL);
	ChNodeClass* class = CH_NODE_GET_CLASS(self);
	g_return_val_if_fail(class->get_line != NULL, NULL);
	return class->get_line(self, x, width, iter);
}

GskRenderNode* ch_node_render_line(ChNode* self, ChMeasurementLine* line, gboolean visual_debug) {
	g_return_val_if_fail(CH_IS_NODE(self), NULL);
	ChNodeClass* class = CH_NODE_GET_CLASS(self);
	g_return_val_if_fail(class->render_line != NULL, NULL);
	return class->render_line(self, line, visual_debug);
}


void ch_node_print_debug(ChNode* self) {
	g_return_if_fail(CH_IS_NODE(self));
	ChNodeClass* class = CH_NODE_GET_CLASS(self);
	g_return_if_fail(class->print_debug);
	class->print_debug(self);
}
