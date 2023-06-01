#ifndef __CHPANGONODE_PARSER_H__
#define __CHPANGONODE_PARSER_H__

#include <nodes/chpangonode.h>
#include <chparser.h>

G_BEGIN_DECLS

gboolean ch_pango_node_parse_asterisk(ChTree* tree, GtkTextIter* iter, gpointer);
gboolean ch_pango_node_parse_underscore(ChTree* tree, GtkTextIter* iter, gpointer);
gboolean ch_pango_node_parse_tilde(ChTree* tree, GtkTextIter* iter, gpointer);

G_END_DECLS

#endif // __CHPANGONODE_PARSER_H__
