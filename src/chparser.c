#include "chparser.h"
#include "nodes/chtextnode.h"

typedef struct {
	ChParserFun fun;
	gpointer user_data;
	GDestroyNotify user_data_destroy_fun;
} ChParserFunContainer;

static ChParserFunContainer* ch_parser_fun_container_new(ChParserFun fun, gpointer user_data, GDestroyNotify user_data_destroy_fun) {
	ChParserFunContainer* self = g_new(ChParserFunContainer, 1);
	self->fun = fun;
	self->user_data = user_data;
	self->user_data_destroy_fun = user_data_destroy_fun;
	
	return self;
}

static gboolean ch_parser_fun_container_call(ChParserFunContainer* self, ChTree* tree, GtkTextIter* iter) {
	return self->fun(tree, iter, self->user_data);
}

static void ch_parser_fun_container_free(ChParserFunContainer* self) {
	if (self->user_data_destroy_fun)
		self->user_data_destroy_fun(self->user_data);
	g_free(self);
}

struct _ChParser {
	GObject parent_instance;
	
	// GTree<int priority, GPtrArray<ChParserFunContainer>>
	// GTree* on_newline;
	// GHashTable<char c, GTree<int priority, GPtrArray<ChParserFunContainer>>>
	GHashTable* on_char;
};

G_DEFINE_TYPE (ChParser, ch_parser, G_TYPE_OBJECT)

static void ch_parser_object_finalize(GObject* object) {
	ChParser* self = CH_PARSER(object);
	
	// g_tree_unref(self->on_newline);
	g_hash_table_unref(self->on_char);
	
	G_OBJECT_CLASS(ch_parser_parent_class)->finalize(object);
}

static void ch_parser_class_init(ChParserClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	
	object_class->finalize = ch_parser_object_finalize;
}

// Warning: Wierd behaviour might occur on arches that have pointers
// with less than four bytes 
static gint ch_gint32_cmp(gconstpointer a, gconstpointer b, gpointer) {
	return (gsize)a - (gsize)b;
}
static void ch_parser_init(ChParser* self) {
	// self->on_newline = g_tree_new_full(ch_gint32_cmp, NULL, NULL, (GDestroyNotify)ch_parser_fun_container_free);
	self->on_char = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)g_tree_unref);
}

ChParser* ch_parser_new(void) {
	return g_object_new(CH_TYPE_PARSER, NULL);
}

/*void ch_parser_add_new_on_newline_fun(ChParser* self, gint32 priority, ChParserFun fun, gpointer user_data, GDestroyNotify user_data_destroy_fun) {
	g_return_if_fail(CH_IS_PARSER(self));
	
	ChParserFunContainer* container = ch_parser_fun_container_new(fun, user_data, user_data_destroy_fun);

	GPtrArray* funs = g_tree_lookup(self->on_newline, (gconstpointer)(gsize)priority);
	if (!funs) {
		funs = g_ptr_array_new_with_free_func((GDestroyNotify)ch_parser_fun_container_free);
		g_tree_insert(self->on_newline, (gpointer)(gsize)priority, funs);
	}
	g_ptr_array_add(funs, container);
}*/

void ch_parser_add_new_on_char_fun(ChParser* self, gunichar character, gint32 priority, ChParserFun fun, gpointer user_data, GDestroyNotify user_data_destroy_fun) {
	g_return_if_fail(CH_IS_PARSER(self));
	
	ChParserFunContainer* container = ch_parser_fun_container_new(fun, user_data, user_data_destroy_fun);
	
	GTree* funs = g_hash_table_lookup(self->on_char, GINT_TO_POINTER(character));
	if (!funs) {
		funs = g_tree_new_full(ch_gint32_cmp, NULL, NULL, (GDestroyNotify)ch_parser_fun_container_free);
		g_hash_table_insert(self->on_char, GINT_TO_POINTER(character), funs);
	}

	GPtrArray* inner_funs = g_tree_lookup(funs, GINT_TO_POINTER(priority));
	if (!inner_funs) {
		inner_funs = g_ptr_array_new_with_free_func((GDestroyNotify)ch_parser_fun_container_free);
		g_tree_insert(funs, GINT_TO_POINTER(priority), inner_funs);
	}
	g_ptr_array_add(inner_funs, container);
}

typedef struct {
	GHashTable* on_char;
	GTree* resolved_value;
} ChParserIsOnCharUd;
static gboolean ch_parser_is_in_on_char(gunichar c, ChParserIsOnCharUd* user_data) {
	user_data->resolved_value = g_hash_table_lookup(user_data->on_char, GINT_TO_POINTER(c));
	return user_data->resolved_value != NULL;
}

typedef struct {
	ChTree* tree;
	GtkTextIter* iter;
	gboolean state;
} ChParserForeachNodeUd;
static gboolean ch_parser_foreach_node(gpointer, GPtrArray* funs, ChParserForeachNodeUd* user_data) {
	for (guint i = 0; i < __builtin_expect(funs->len, 1); i++)
		if (ch_parser_fun_container_call(funs->pdata[i], user_data->tree, user_data->iter)) {
			user_data->state = TRUE;
			return TRUE;
		}
	return FALSE;
}

ChTree* ch_parser_parse(ChParser* self, GtkTextBuffer* buf, Pango2Context* context) {
	g_return_val_if_fail(CH_IS_PARSER(self), NULL);
	
	ChTree* tree = ch_tree_new();
	
	GtkTextIter i,p;
	gtk_text_buffer_get_start_iter(buf, &i);
	p = i;
	
	ChParserIsOnCharUd cb_ud = {
		.on_char = self->on_char,
		.resolved_value = NULL
	};
	// workarround for first char being ignored
	if (ch_parser_is_in_on_char(gtk_text_iter_get_char(&i), &cb_ud)) {
		ChParserForeachNodeUd fe_ud = {
			.tree = tree,
			.iter = &i,
			.state = FALSE
		};

		g_tree_foreach(cb_ud.resolved_value, (GTraverseFunc)ch_parser_foreach_node, &fe_ud);
		if (fe_ud.state)
			p = i;
	}
	while (gtk_text_iter_forward_find_char(&i, (GtkTextCharPredicate)ch_parser_is_in_on_char, &cb_ud, NULL)) {
		GtkTextIter r = i;
		//gtk_text_iter_backward_char(&r);
		if (gtk_text_iter_compare(&p, &r) < 0) {
			gchar* text = gtk_text_iter_get_text(&p, &r);
			ch_tree_add_node(tree, ch_text_node_new(context, text));
			//g_print("Creating text node: %s\n", text);
			g_free(text);
		}
		
		ChParserForeachNodeUd fe_ud = {
			.tree = tree,
			.iter = &i,
			.state = FALSE
		};
		g_tree_foreach(cb_ud.resolved_value, (GTraverseFunc)ch_parser_foreach_node, &fe_ud);
		if (fe_ud.state)
			p = i;
	}

	if (gtk_text_iter_compare(&p, &i) < 0) {
		gchar* text = gtk_text_iter_get_text(&p, &i);
		ch_tree_add_node(tree, ch_text_node_new(context, text));
		//g_print("Creating text node: %s\n", text);
		g_free(text);
	}

	return tree;
}
