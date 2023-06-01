#ifndef __CHPARSER_H__
#define __CHPARSER_H__

#include <glib-object.h>
#include <gtk/gtk.h>
#include <pango2/pango.h>

#include <chtree.h>

G_BEGIN_DECLS

typedef gboolean(*ChParserFun)(ChTree* tree, GtkTextIter* iter, gpointer user_data);

#define CH_TYPE_PARSER (ch_parser_get_type())
G_DECLARE_FINAL_TYPE (ChParser, ch_parser, CH, PARSER, GObject)

ChParser* ch_parser_new(void);

// void ch_parser_add_new_on_newline_fun(ChParser* self, gint32 priority, ChParserFun fun, gpointer user_data, GDestroyNotify user_data_destroy_fun);
void ch_parser_add_new_on_char_fun(ChParser* self, gunichar character, gint32 priority, ChParserFun fun, gpointer user_data, GDestroyNotify user_data_destroy_fun);

ChTree* ch_parser_parse(ChParser* self, GtkTextBuffer* buf, Pango2Context* context);

G_END_DECLS

#endif // __CHPARSER_H__
