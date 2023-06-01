#include "chtitlenode.h"
#include "chtitlenode-parser.h"

#include <pango2/pangocairo.h>

static const guint MAX_TITLE_LEVEL = 4;

struct _ChTitleNode {
	ChNode parent_instance;
	
	Pango2Context* context;
	guint level;
	gchar* text;
};

G_DEFINE_TYPE (ChTitleNode, ch_title_node, CH_TYPE_NODE)

enum {
	PROP_PANGO_CONTEXT = 1,
 	PROP_LEVEL,
	PROP_TEXT,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static void ch_title_node_object_finalize(GObject* object) {
	ChTitleNode* self = CH_TITLE_NODE(object);
	g_free(self->text);
	
	G_OBJECT_CLASS(ch_title_node_parent_class)->finalize(object);
}

static void ch_title_node_object_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	ChTitleNode* self = CH_TITLE_NODE(object);
	
	switch (prop_id) {
		case PROP_PANGO_CONTEXT:
			g_value_set_object(value, ch_title_node_get_pango_context(self));
			break;
		case PROP_LEVEL:
			g_value_set_uint(value, ch_title_node_get_level(self));
			break;
		case PROP_TEXT:
			g_value_set_string(value, ch_title_node_get_text(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void ch_title_node_object_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
	ChTitleNode* self = CH_TITLE_NODE(object);
	
	switch (prop_id) {
		case PROP_PANGO_CONTEXT:
			ch_title_node_set_pango_context(self, g_value_get_object(value));
			break;
		case PROP_LEVEL:
			ch_title_node_set_level(self, g_value_get_uint(value));
			break;
		case PROP_TEXT:
			ch_title_node_set_text(self, g_value_get_string(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void ch_title_node_measurement_line_free(gpointer line) {
	g_return_if_fail(((ChMeasurementLine*)line)->creator == CH_TYPE_TITLE_NODE);
	ChMeasurementLineTitleNode* int_line = (ChMeasurementLineTitleNode*)line;

	pango2_line_free(int_line->pango_line);	
	g_free(int_line);
}
static Pango2Color CH_TITLE_LEVEL_COLOR = { .red = 0xa0 * 0xFF, .green = 0xa8 * 0xFF, .blue = 0xc0 * 0xFF, .alpha = 0xFF * 0xFF };
static ChMeasurementLine* ch_title_node_node_get_line(ChNode* node, gint x, gint width, gpointer* iter) {
	ChTitleNode* self = CH_TITLE_NODE(node);
	
	Pango2LineBreaker* breaker;
	if (!*iter) {
		*iter =  pango2_line_breaker_new(self->context);
		
		g_autoptr(Pango2AttrList) attrs_title = pango2_attr_list_new();
		// 36pt at level 1, 18pt at level 3 --> $\frac{18-36}{3-1}(l - 1)+36$
		pango2_attr_list_insert(attrs_title, pango2_attr_size_new(pango2_units_from_double(-9*(self->level - 1) + 36)));
		pango2_attr_list_insert(attrs_title, pango2_attr_foreground_new(&CH_TITLE_LEVEL_COLOR));
		g_autoptr(GByteArray) levelstr = g_byte_array_new();
		const gchar POUNDSYM = '#';
		for (guint i = 0; i < self->level; i++)
			g_byte_array_append(levelstr, (guint8*)&POUNDSYM, 1);
		pango2_line_breaker_add_text(*iter, (gchar*)levelstr->data, levelstr->len, attrs_title);

		g_autoptr(Pango2AttrList) attrs_text = pango2_attr_list_new();
		// 36pt at level 1, 18pt at level 3 --> $\frac{18-36}{3-1}(l - 1)+36$
		pango2_attr_list_insert(attrs_text, pango2_attr_size_new(pango2_units_from_double(-9*(self->level - 1) + 36)));
		pango2_line_breaker_add_text(*iter, self->text, strlen(self->text), attrs_text);
	}
	breaker = PANGO2_LINE_BREAKER(*iter);

	Pango2Line* pango_line = pango2_line_breaker_next_line(breaker, x, width, PANGO2_WRAP_WORD_CHAR, PANGO2_ELLIPSIZE_NONE);
	if (!pango_line) {
		g_object_unref(breaker);
		*iter = NULL;
		return NULL;
	}
	
	ChMeasurementLineTitleNode* line = g_new(ChMeasurementLineTitleNode, 1);
	line->line.creator = CH_TYPE_TITLE_NODE;
	line->line.free = ch_title_node_measurement_line_free;

	line->pango_line = pango_line;
	
	Pango2Rectangle log_ext;
	pango2_line_get_extents(pango_line, NULL, &log_ext);
	
	line->line.extents = log_ext;
	line->line.extents.x += x;
	line->offset = -log_ext.y;
	
	line->line.is_last = !pango2_line_breaker_has_line(breaker);
	
	return (ChMeasurementLine*)line;
}

static GskRenderNode* ch_title_node_node_render_line(ChNode*, ChMeasurementLine* line, __attribute__((unused)) gboolean visual_debug) {
	g_return_val_if_fail(line->creator == CH_TYPE_TITLE_NODE, NULL);
	ChMeasurementLineTitleNode* int_line = (ChMeasurementLineTitleNode*)line;
	
	GskRenderNode* ret = gsk_cairo_node_new(&GRAPHENE_RECT_INIT(
		pango2_units_to_double(0),
		pango2_units_to_double(0),
		pango2_units_to_double(line->extents.width),
		pango2_units_to_double(line->extents.height)
	));
	cairo_t* ctx = gsk_cairo_node_get_draw_context(ret);
	
	cairo_set_source_rgba(ctx, 0.0, 0.0, 0.0, 1.0);
	cairo_move_to(ctx, 0, pango2_units_to_double(int_line->offset));
	pango2_cairo_show_line(ctx, int_line->pango_line);
	cairo_stroke(ctx);
	
	cairo_destroy(ctx);
	
	return ret;
}

static void ch_title_node_node_print_debug(ChNode* node) {
	ChTitleNode* self = CH_TITLE_NODE(node);
	g_print("%s(%d)(%s)", G_OBJECT_TYPE_NAME(node), self->level, self->text);
}

static void ch_title_node_class_init(ChTitleNodeClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	ChNodeClass* node_class = CH_NODE_CLASS(class);

	object_class->finalize = ch_title_node_object_finalize;
	object_class->get_property = ch_title_node_object_get_property;
	object_class->set_property = ch_title_node_object_set_property;
	
	node_class->get_line = ch_title_node_node_get_line;
	node_class->render_line = ch_title_node_node_render_line;
	node_class->print_debug = ch_title_node_node_print_debug;

	obj_properties[PROP_PANGO_CONTEXT] = g_param_spec_object("pango-context", "Pango Context", "The Pango Context which will be used to render this Text Node", PANGO2_TYPE_CONTEXT, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);
	obj_properties[PROP_TEXT] = g_param_spec_string("text", "Text", "The entire text of this node", NULL, G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);
	obj_properties[PROP_LEVEL] = g_param_spec_uint("level", "Level", "The level of this heading", 0, MAX_TITLE_LEVEL, 0, G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void ch_title_node_init(ChTitleNode* self) {
	self->context = NULL;
	self->level = 0;
	self->text = NULL;
}

ChNode* ch_title_node_new(guint level, const gchar* text, Pango2Context* context) {
	return g_object_new(CH_TYPE_TITLE_NODE, "level", level, "text", text, "pango-context", context, NULL);
}

Pango2Context* ch_title_node_get_pango_context(ChTitleNode* self) {
	g_return_val_if_fail(CH_IS_TITLE_NODE(self), NULL);
	return self->context;
}

void ch_title_node_set_pango_context(ChTitleNode* self, Pango2Context* context) {
	g_return_if_fail(CH_IS_TITLE_NODE(self));
	
	if (self->context == context)
		return;
	
	if (self->context)
		g_object_unref(self->context);
	self->context = context;
	
	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_PANGO_CONTEXT]);
}

guint ch_title_node_get_level(ChTitleNode* self) {
	g_return_val_if_fail(CH_IS_TITLE_NODE(self), 0);
	return self->level;
}

void ch_title_node_set_level(ChTitleNode* self, guint level) {
	g_return_if_fail(CH_IS_TITLE_NODE(self));
	
	if (self->level == level)
		return;
	self->level = level;

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_LEVEL]);
}

const gchar* ch_title_node_get_text(ChTitleNode* self) {
	g_return_val_if_fail(CH_IS_TITLE_NODE(self), NULL);
	return self->text;
}

void ch_title_node_set_text(ChTitleNode* self, const gchar* text) {
	g_return_if_fail(CH_IS_TITLE_NODE(self));
	
	if (g_strcmp0(self->text, text) == 0)
		return;
	if (self->text)
		g_free(self->text);
	self->text = g_strdup(text);

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_TEXT]);
}

// begin parser
gboolean ch_title_node_parse(ChTree* tree, GtkTextIter* iter, Pango2Context* context) {
	GtkTextIter active = *iter;
	
	guint level = 0;
	while (gtk_text_iter_get_char(&active) == '#' && level < MAX_TITLE_LEVEL) {
		level++; gtk_text_iter_forward_char(&active);
	}
	if (level == 0)
		return FALSE;
	
	GtkTextIter start = active;
	if (gtk_text_iter_get_char(&active) != '\n')
		gtk_text_iter_forward_to_line_end(&active);
	
	gchar* text = gtk_text_iter_get_text(&start, &active);

	ch_tree_add_node(tree, ch_title_node_new(level, text, context));
	
	g_free(text);

	*iter = active;
	return TRUE;
}
