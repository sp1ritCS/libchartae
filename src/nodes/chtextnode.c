#include "chtextnode.h"
#include <pango2/pangocairo.h>

typedef struct {
	Pango2Context* context;
	gchar* text;
} ChTextNodePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (ChTextNode, ch_text_node, CH_TYPE_NODE)

enum {
	PROP_PANGO_CONTEXT = 1,
 	PROP_TEXT,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static void ch_text_node_object_finalize(GObject* object) {
	ChTextNodePrivate* priv = ch_text_node_get_instance_private(CH_TEXT_NODE(object));
	g_free(priv->text);
	
	G_OBJECT_CLASS(ch_text_node_parent_class)->finalize(object);
}

static void ch_text_node_object_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	ChTextNode* self = CH_TEXT_NODE(object);
	
	switch (prop_id) {
		case PROP_PANGO_CONTEXT:
			g_value_set_object(value, ch_text_node_get_pango_context(self));
			break;
		case PROP_TEXT:
			g_value_set_string(value, ch_text_node_get_text(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void ch_text_node_object_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
	ChTextNode* self = CH_TEXT_NODE(object);
	
	switch (prop_id) {
		case PROP_PANGO_CONTEXT:
			ch_text_node_set_pango_context(self, g_value_get_object(value));
			break;
		case PROP_TEXT:
			ch_text_node_set_text(self, g_value_get_string(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void ch_text_node_measurement_line_free(gpointer line) {
	g_return_if_fail(((ChMeasurementLine*)line)->creator == CH_TYPE_TEXT_NODE);
	ChMeasurementLineTextNode* int_line = (ChMeasurementLineTextNode*)line;

	pango2_line_free(int_line->pango_line);	
	g_free(int_line);
}
static ChMeasurementLine* ch_text_node_node_get_line(ChNode* node, gint x, gint width, gpointer* iter) {
	ChTextNodePrivate* priv = ch_text_node_get_instance_private(CH_TEXT_NODE(node));
	
	g_autoptr(Pango2AttrList) attrs = pango2_attr_list_new();
	
	Pango2LineBreaker* breaker;
	if (!*iter) {
		*iter =  pango2_line_breaker_new(priv->context);
		pango2_line_breaker_add_text(*iter, priv->text, strlen(priv->text), attrs);
	}
	breaker = PANGO2_LINE_BREAKER(*iter);

	Pango2Line* pango_line = pango2_line_breaker_next_line(breaker, x, width, PANGO2_WRAP_WORD_CHAR, PANGO2_ELLIPSIZE_NONE);
	if (!pango_line) {
		g_object_unref(breaker);
		*iter = NULL;
		return NULL;
	}
	
	ChMeasurementLineTextNode* line = g_new(ChMeasurementLineTextNode, 1);
	line->line.creator = CH_TYPE_TEXT_NODE;
	line->line.free = ch_text_node_measurement_line_free;

	line->pango_line = pango_line;
	
	Pango2Rectangle log_ext;
	pango2_line_get_extents(pango_line, NULL, &log_ext);
	
	line->line.extents = log_ext;
	line->line.extents.x += x;
	line->offset = -log_ext.y;
	
	line->line.is_last = !pango2_line_breaker_has_line(breaker);
	
	return (ChMeasurementLine*)line;
}

static GskRenderNode* ch_text_node_node_render_line(ChNode*, ChMeasurementLine* line, __attribute__((unused)) gboolean visual_debug) {
	g_return_val_if_fail(line->creator == CH_TYPE_TEXT_NODE, NULL);
	ChMeasurementLineTextNode* int_line = (ChMeasurementLineTextNode*)line;
	
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

static void ch_text_node_node_print_debug(ChNode* node) {
	ChTextNodePrivate* priv = ch_text_node_get_instance_private(CH_TEXT_NODE(node));
	g_print("%s(%s)", G_OBJECT_TYPE_NAME(node), priv->text);
}

static void ch_text_node_class_init(ChTextNodeClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	ChNodeClass* node_class = CH_NODE_CLASS(class);

	object_class->finalize = ch_text_node_object_finalize;
	object_class->get_property = ch_text_node_object_get_property;
	object_class->set_property = ch_text_node_object_set_property;
	
	node_class->get_line = ch_text_node_node_get_line;
	node_class->render_line = ch_text_node_node_render_line;
	node_class->print_debug = ch_text_node_node_print_debug;
	
	obj_properties[PROP_PANGO_CONTEXT] = g_param_spec_object("pango-context", "Pango Context", "The Pango Context which will be used to render this Text Node", PANGO2_TYPE_CONTEXT, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);
	obj_properties[PROP_TEXT] = g_param_spec_string("text", "Text", "The entire text of this node", NULL, G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void ch_text_node_init(ChTextNode* self) {
	ChTextNodePrivate* priv = ch_text_node_get_instance_private(self);
	priv->context = NULL;
	priv->text = NULL;
}

ChNode* ch_text_node_new(Pango2Context* context, const gchar* text) {
	return g_object_new(CH_TYPE_TEXT_NODE, "pango-context", context, "text", text, NULL);
}

Pango2Context* ch_text_node_get_pango_context(ChTextNode* self) {
	g_return_val_if_fail(CH_IS_TEXT_NODE(self), NULL);
	ChTextNodePrivate* priv = ch_text_node_get_instance_private(self);
	return priv->context;
}

void ch_text_node_set_pango_context(ChTextNode* self, Pango2Context* context) {
	g_return_if_fail(CH_IS_TEXT_NODE(self));
	ChTextNodePrivate* priv = ch_text_node_get_instance_private(self);
	
	if (priv->context == context)
		return;
	
	if (priv->context)
		g_object_unref(priv->context);
	priv->context = context;
	
	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_PANGO_CONTEXT]);
}

const gchar* ch_text_node_get_text(ChTextNode* self) {
	g_return_val_if_fail(CH_IS_TEXT_NODE(self), NULL);
	ChTextNodePrivate* priv = ch_text_node_get_instance_private(self);
	return priv->text;
}

void ch_text_node_set_text(ChTextNode* self, const gchar* text) {
	g_return_if_fail(CH_IS_TEXT_NODE(self));
	ChTextNodePrivate* priv = ch_text_node_get_instance_private(self);
	
	if (g_strcmp0(priv->text, text) == 0)
		return;
	
	if (priv->text)
		g_free(priv->text);
	priv->text = g_strdup(text);
	
	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_TEXT]);
}
