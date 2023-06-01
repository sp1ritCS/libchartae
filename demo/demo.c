#include <chartae.h>
#include <gtk/gtk.h>

static void buffer_changed_cb(GtkTextBuffer* buf, ChParser* parser) {
	ChTreeView* view = g_object_get_data(G_OBJECT(buf), "chartae::tree-view");

	Pango2Context* ctx = g_object_get_data(G_OBJECT(parser), "chartae::pango-context");
	ChTree* tree = ch_parser_parse(parser, buf, ctx);
	ch_tree_view_set_tree(view, tree);
	ch_tree_debug_print(tree);
	
	g_object_unref(tree);
} 

static void activate(GtkApplication* app, ChParser* parser) {
	GtkWidget* win = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(win), "Chartae Demo");
	
	GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	
	GtkWidget* lhs = gtk_text_view_new();
	gtk_widget_set_hexpand(lhs, TRUE);
	GtkTextBuffer* buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(lhs));
	g_signal_connect(buf, "changed", G_CALLBACK(buffer_changed_cb), parser);
	gtk_box_append(GTK_BOX(box), lhs);
	
	GtkWidget* sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
	gtk_box_append(GTK_BOX(box), sep);
	
	GtkWidget* rhs = ch_tree_view_new(NULL);
	gtk_widget_set_hexpand(rhs, TRUE);
	g_object_set_data(G_OBJECT(buf), "chartae::tree-view", rhs);
	gtk_box_append(GTK_BOX(box), rhs);
	
	gtk_window_set_child(GTK_WINDOW(win), box);

	gtk_window_present(GTK_WINDOW(win));
}

int main(int argc, char** argv) {
	g_autoptr(ChParser) parser = ch_parser_new();
	g_autoptr(Pango2Context) pctx = pango2_context_new();
	g_object_set_data(G_OBJECT(parser), "chartae::pango-context", pctx);

	ch_parser_add_new_on_char_fun(parser, '*', 500, ch_pango_node_parse_asterisk, NULL, NULL);
	ch_parser_add_new_on_char_fun(parser, '_', 500, ch_pango_node_parse_underscore, NULL, NULL);
	ch_parser_add_new_on_char_fun(parser, '~', 500, ch_pango_node_parse_tilde, NULL, NULL);
	ch_parser_add_new_on_char_fun(parser, '#', 10, (ChParserFun)ch_title_node_parse, pctx, NULL);
	
	g_autoptr(GtkApplication) app = gtk_application_new("arpa.sp1rit.chartae.demo", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), parser);
	return g_application_run(G_APPLICATION(app), argc, argv);
}
