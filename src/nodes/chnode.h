#ifndef __CHNODE_H__
#define __CHNODE_H__

#include <glib-object.h>
#include <pango2/pango.h>
#include <gsk/gsk.h>

G_BEGIN_DECLS

typedef Pango2Rectangle ChMeasurementRect;

typedef struct {
	GType creator;
	void(*free)(gpointer line);
	
	gboolean is_last;
	gboolean has_cursor;
	ChMeasurementRect extents;

	gint offset;
	ChMeasurementRect extended;
	gint line_no;
} ChMeasurementLine;


#define CH_TYPE_NODE (ch_node_get_type())
G_DECLARE_DERIVABLE_TYPE (ChNode, ch_node, CH, NODE, GInitiallyUnowned)

struct _ChNodeClass {
	GInitiallyUnownedClass parent_class;
	
	ChMeasurementLine*(*get_line)(ChNode* self, gint x, gint width, gpointer* iter);
	GskRenderNode*(*render_line)(ChNode* self, ChMeasurementLine* line, gboolean visual_debug);

	void(*print_debug)(ChNode* self);
};

ChMeasurementLine* ch_node_get_line(ChNode* self, gint x, gint width, gpointer* iter);
GskRenderNode* ch_node_render_line(ChNode* self, ChMeasurementLine* line, gboolean visual_debug);

void ch_node_print_debug(ChNode* self);

G_END_DECLS

#endif // __CHNODE_H__
