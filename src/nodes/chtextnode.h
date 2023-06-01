#ifndef __CHTEXTNODE_H__
#define __CHTEXTNODE_H__

#include <glib-object.h>
#include <nodes/chnode.h>

G_BEGIN_DECLS

typedef struct {
	ChMeasurementLine line;
	Pango2Line* pango_line;
	gint offset;
} ChMeasurementLineTextNode;

#define CH_TYPE_TEXT_NODE (ch_text_node_get_type())
G_DECLARE_DERIVABLE_TYPE (ChTextNode, ch_text_node, CH, TEXT_NODE, ChNode)

struct _ChTextNodeClass {
	ChNodeClass parent_class;
};

ChNode* ch_text_node_new(Pango2Context* context, const gchar* text);

Pango2Context* ch_text_node_get_pango_context(ChTextNode* self);
void ch_text_node_set_pango_context(ChTextNode* self, Pango2Context* context);

const gchar* ch_text_node_get_text(ChTextNode* self);
void ch_text_node_set_text(ChTextNode* self, const gchar* text);

G_END_DECLS

#endif // __CHTEXTNODE_H__
