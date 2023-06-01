#ifndef __CHPANGONODE_H__
#define __CHPANGONODE_H__

#include <glib-object.h>
#include <nodes/chnode.h>
#include <pango2/pango.h>

G_BEGIN_DECLS

#define CH_TYPE_PANGO_NODE (ch_pango_node_get_type())
G_DECLARE_DERIVABLE_TYPE (ChPangoNode, ch_pango_node, CH, PANGO_NODE, ChNode)

struct _ChPangoNodeClass {
	ChNodeClass parent_class;
};

ChNode* ch_pango_node_new(const gchar* text, const gchar* inner, Pango2AttrList* attrs, gunichar magic_char);

const gchar* ch_pango_node_get_text(ChPangoNode* self);
void ch_pango_node_set_text(ChPangoNode* self, const gchar* text);

const gchar* ch_pango_node_get_inner_text(ChPangoNode* self);
void ch_pango_node_set_inner_text(ChPangoNode* self, const gchar* inner_text);

Pango2AttrList* ch_pango_node_get_attrs(ChPangoNode* self);
void ch_pango_node_set_attrs(ChPangoNode* self, Pango2AttrList* attrs);

gunichar ch_pango_node_get_magic_char(ChPangoNode* self);
void ch_pango_node_set_magic_char(ChPangoNode* self, gunichar magic_char);

G_END_DECLS

#endif // __CHPANGONODE_H__
