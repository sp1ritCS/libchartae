#ifndef __CHTREEVIEW_H__
#define __CHTREEVIEW_H__

#include <glib-object.h>
#include <gtk/gtk.h>
#include <chtree.h>

G_BEGIN_DECLS

#define CH_TYPE_TREE_VIEW (ch_tree_view_get_type())
G_DECLARE_FINAL_TYPE (ChTreeView, ch_tree_view, CH, TREE_VIEW, GtkWidget)

GtkWidget* ch_tree_view_new(ChTree* tree);

ChTree* ch_tree_view_get_tree(ChTreeView* self);
void ch_tree_view_set_tree(ChTreeView* self, ChTree* tree);

G_END_DECLS

#endif // __CHTREEVIEW_H__
