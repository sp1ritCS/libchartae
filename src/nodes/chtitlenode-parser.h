#ifndef __CHTITLENODE_PARSER_H__
#define __CHTITLENODE_PARSER_H__

#include <nodes/chtitlenode.h>
#include <chparser.h>

G_BEGIN_DECLS

gboolean ch_title_node_parse(ChTree* tree, GtkTextIter* iter, Pango2Context* context);

G_END_DECLS

#endif // __CHTITLENODE_PARSER_H__
