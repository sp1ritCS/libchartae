#ifndef __CHTITLENODE_H__
#define __CHTITLENODE_H__

#include <glib-object.h>
#include <nodes/chnode.h>

G_BEGIN_DECLS

typedef struct {
	ChMeasurementLine line;
	Pango2Line* pango_line;
	gint offset;
} ChMeasurementLineTitleNode;

#define CH_TYPE_TITLE_NODE (ch_title_node_get_type())
G_DECLARE_FINAL_TYPE (ChTitleNode, ch_title_node, CH, TITLE_NODE, ChNode)

ChNode* ch_title_node_new(guint level, const gchar* text, Pango2Context* context);

Pango2Context* ch_title_node_get_pango_context(ChTitleNode* self);
void ch_title_node_set_pango_context(ChTitleNode* self, Pango2Context* context);

guint ch_title_node_get_level(ChTitleNode* self);
void ch_title_node_set_level(ChTitleNode* self, guint level);

const gchar* ch_title_node_get_text(ChTitleNode* self);
void ch_title_node_set_text(ChTitleNode* self, const gchar* text);

G_END_DECLS

#endif // __CHTITLENODE_H__
