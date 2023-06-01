#include "chpangonode.h"
#include "chpangonode-parser.h"

typedef struct  {
	gchar* text;
	gchar* inner;
	Pango2AttrList* attrs;
	gunichar magic_char;
} ChPangoNodePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (ChPangoNode, ch_pango_node, CH_TYPE_NODE)

enum {
 	PROP_TEXT = 1,
	PROP_INNER_TEXT,
	PROP_ATTRS,
	PROP_MAGIC_CHAR,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static void ch_pango_node_object_finalize(GObject* object) {
	ChPangoNodePrivate* priv = ch_pango_node_get_instance_private(CH_PANGO_NODE(object));
	g_free(priv->text);
	g_free(priv->inner);
	pango2_attr_list_unref(priv->attrs);
	
	G_OBJECT_CLASS(ch_pango_node_parent_class)->finalize(object);
}

static void ch_pango_node_object_dispose(GObject* object) {
	ChPangoNodePrivate* priv = ch_pango_node_get_instance_private(CH_PANGO_NODE(object));
	g_clear_pointer(&priv->attrs, pango2_attr_list_unref);

	G_OBJECT_CLASS(ch_pango_node_parent_class)->dispose(object);
}

static void ch_pango_node_object_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	ChPangoNode* self = CH_PANGO_NODE(object);
	
	switch (prop_id) {
		case PROP_TEXT:
			g_value_set_string(value, ch_pango_node_get_text(self));
			break;
		case PROP_INNER_TEXT:
			g_value_set_string(value, ch_pango_node_get_inner_text(self));
			break;
		case PROP_ATTRS:
			g_value_set_boxed(value, ch_pango_node_get_attrs(self));
			break;
		case PROP_MAGIC_CHAR:
			g_value_set_uint(value, ch_pango_node_get_magic_char(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void ch_pango_node_object_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
	ChPangoNode* self = CH_PANGO_NODE(object);
	
	switch (prop_id) {
		case PROP_TEXT:
			ch_pango_node_set_text(self, g_value_get_string(value));
			break;
		case PROP_INNER_TEXT:
			ch_pango_node_set_inner_text(self, g_value_get_string(value));
			break;
		case PROP_ATTRS:
			ch_pango_node_set_attrs(self, g_value_get_boxed(value));
			break;
		case PROP_MAGIC_CHAR:
			ch_pango_node_set_magic_char(self, g_value_get_uint(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void ch_pango_node_node_print_debug(ChNode* node) {
	ChPangoNodePrivate* priv = ch_pango_node_get_instance_private(CH_PANGO_NODE(node));
	g_print("%s(%s)", G_OBJECT_TYPE_NAME(node), priv->text);
}

static void ch_pango_node_class_init(ChPangoNodeClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	ChNodeClass* node_class = CH_NODE_CLASS(class);

	object_class->finalize = ch_pango_node_object_finalize;
	object_class->dispose = ch_pango_node_object_dispose;
	object_class->get_property = ch_pango_node_object_get_property;
	object_class->set_property = ch_pango_node_object_set_property;
	
	node_class->print_debug = ch_pango_node_node_print_debug;
	
	obj_properties[PROP_TEXT] = g_param_spec_string("text", "Text", "The entire text of this node", NULL, G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);
	obj_properties[PROP_INNER_TEXT] = g_param_spec_string("inner-text", "Inner Text", "The inner text contents of this node", NULL, G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);
	obj_properties[PROP_ATTRS] = g_param_spec_boxed("attrs", "Pango Attributes", "Pango Attributes that apply to this node", PANGO2_TYPE_ATTR_LIST, G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);
	obj_properties[PROP_MAGIC_CHAR] = g_param_spec_unichar("magic-char", "Magic Character", "Character whose modification will result in reparse", '\0', G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void ch_pango_node_init(ChPangoNode* self) {
	ChPangoNodePrivate* priv = ch_pango_node_get_instance_private(self);
	priv->text = NULL;
	priv->inner = NULL;
	priv->attrs = NULL;
	priv->magic_char = '\0';
}

ChNode* ch_pango_node_new(const gchar* text, const gchar* inner, Pango2AttrList* attrs, gunichar magic_char) {
	return g_object_new(CH_TYPE_PANGO_NODE, "text", text, "inner-text", inner, "attrs", attrs, "magic-char", magic_char, NULL);
}

const gchar* ch_pango_node_get_text(ChPangoNode* self) {
	g_return_val_if_fail(CH_IS_PANGO_NODE(self), NULL);
	ChPangoNodePrivate* priv = ch_pango_node_get_instance_private(self);
	return priv->text;
}
void ch_pango_node_set_text(ChPangoNode* self, const gchar* text) {
	g_return_if_fail(CH_IS_PANGO_NODE(self));
	ChPangoNodePrivate* priv = ch_pango_node_get_instance_private(self);
	
	if (g_strcmp0(priv->text, text) == 0)
		return;
	
	if (priv->text)
		g_free(priv->text);
	priv->text = g_strdup(text);
	
	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_TEXT]);
}

const gchar* ch_pango_node_get_inner_text(ChPangoNode* self) {
	g_return_val_if_fail(CH_IS_PANGO_NODE(self), NULL);
	ChPangoNodePrivate* priv = ch_pango_node_get_instance_private(self);
	return priv->inner;
}
void ch_pango_node_set_inner_text(ChPangoNode* self, const gchar* inner_text) {
	g_return_if_fail(CH_IS_PANGO_NODE(self));
	ChPangoNodePrivate* priv = ch_pango_node_get_instance_private(self);
	
	if (g_strcmp0(priv->inner, inner_text) == 0)
		return;
	
	if (priv->inner)
		g_free(priv->inner);
	priv->inner = g_strdup(inner_text);
	
	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_INNER_TEXT]);
}

Pango2AttrList* ch_pango_node_get_attrs(ChPangoNode* self) {
	g_return_val_if_fail(CH_IS_PANGO_NODE(self), NULL);
	ChPangoNodePrivate* priv = ch_pango_node_get_instance_private(self);
	return priv->attrs;
}
void ch_pango_node_set_attrs(ChPangoNode* self, Pango2AttrList* attrs) {
	g_return_if_fail(CH_IS_PANGO_NODE(self));
	ChPangoNodePrivate* priv = ch_pango_node_get_instance_private(self);
	
	if (priv->attrs == attrs)
		return;
	
	if (priv->attrs)
		pango2_attr_list_unref(priv->attrs);
	priv->attrs = pango2_attr_list_ref(attrs);

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_ATTRS]);
}

gunichar ch_pango_node_get_magic_char(ChPangoNode* self) {
	g_return_val_if_fail(CH_IS_PANGO_NODE(self), '\0');
	ChPangoNodePrivate* priv = ch_pango_node_get_instance_private(self);
	return priv->magic_char;
}
void ch_pango_node_set_magic_char(ChPangoNode* self, gunichar magic_char) {
	g_return_if_fail(CH_IS_PANGO_NODE(self));
	ChPangoNodePrivate* priv = ch_pango_node_get_instance_private(self);
	
	if (priv->magic_char == magic_char)
		return;
	
	priv->magic_char = magic_char;

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_MAGIC_CHAR]);
}

// BEGIN PARSERS

static gboolean ch_pango_node_check_char(gunichar ch, gpointer user_data) {
	return ch == GPOINTER_TO_UINT(user_data);
}

typedef gboolean(*ChPangoParseGenericCb)(
	guint level,
	const GtkTextIter* start,
	const GtkTextIter* start_inner,
	const GtkTextIter* end_inner,
	const GtkTextIter* end,
	gpointer user_data
);
static gboolean ch_pango_parse_generic(GtkTextIter* iter, gunichar c, guint max_level, ChPangoParseGenericCb callback, gpointer user_data) {
	GtkTextIter active = *iter;
	
	guint level = 0;
	while (gtk_text_iter_get_char(&active) == c && level < max_level) {
		level++; gtk_text_iter_forward_char(&active);
	}
	if (level == 0)
		return FALSE;
	
	GtkTextIter begin_inner = active;
	gboolean end_is_valid = FALSE;
	GtkTextIter end_inner = active;

	while (gtk_text_iter_forward_find_char(&active, ch_pango_node_check_char, GUINT_TO_POINTER(c), NULL)) {
		end_inner = active;

		guint endlevel = 0;
		while (gtk_text_iter_get_char(&active) == c && endlevel <= max_level) {
			endlevel++; gtk_text_iter_forward_char(&active);
		}
		if (endlevel == level) {
			end_is_valid = TRUE;
			break;
		}
	}

	if (!end_is_valid)
		end_inner = active;
	
	if (!callback(level, iter, &begin_inner, &end_inner, &active, user_data))
		return FALSE;
	*iter = active;
	return TRUE;
}


gboolean ch_pango_node_parse_asterisk_cb(
	guint level,
	const GtkTextIter* start,
	const GtkTextIter* start_inner,
	const GtkTextIter* end_inner,
	const GtkTextIter* end,
	ChTree* tree
) {
	gchar* text = gtk_text_iter_get_text(start, end);
	gchar* inner_text = gtk_text_iter_get_text(start_inner, end_inner);
	
	g_autoptr(Pango2AttrList) attrs = pango2_attr_list_new();
	
	ChNode* node = NULL;
	switch (level) {
		case 1:
			pango2_attr_list_insert(attrs, pango2_attr_style_new(PANGO2_STYLE_ITALIC));
			node = ch_pango_node_new(text, inner_text, attrs, '*');
			break;
		case 2:
			pango2_attr_list_insert(attrs, pango2_attr_weight_new(PANGO2_WEIGHT_BOLD));
			node = ch_pango_node_new(text, inner_text, attrs, '*');
			break;
		case 3:
			pango2_attr_list_insert(attrs, pango2_attr_style_new(PANGO2_STYLE_ITALIC));
			pango2_attr_list_insert(attrs, pango2_attr_weight_new(PANGO2_WEIGHT_BOLD));
			node = ch_pango_node_new(text, inner_text, attrs, '*');
			break;
		default:
			g_warning("This should not be reached. Please file an issue.");
	}
	if (node)
		ch_tree_add_node(tree, node);
	
	g_free(inner_text);
	g_free(text);
	return TRUE;
}
gboolean ch_pango_node_parse_asterisk(ChTree* tree, GtkTextIter* iter, gpointer) {
	return ch_pango_parse_generic(iter, '*', 3, (ChPangoParseGenericCb)ch_pango_node_parse_asterisk_cb, tree);
	/*GtkTextIter active = *iter;
	
	guint level = 0;
	while (gtk_text_iter_get_char(&active) == '*' && level <= 3) {
		level++; gtk_text_iter_forward_char(&active);
	}

	GtkTextIter begin_inner = active;
	
	// Find matching
	while (gtk_text_iter_forward_find_char(&active, ch_pango_node_check_char, GUINT_TO_POINTER('*'), NULL)) {
		guint endlevel = 0;
		while (gtk_text_iter_get_char(&active) == '*' && level <= 3) {
			endlevel++; gtk_text_iter_forward_char(&active);
		}
		if (endlevel == level)
			break;
	}
	
	gchar* text = gtk_text_iter_get_text(iter, &active);
	g_print("Creating pango node level %d with text: %s\n", level, text);
	g_free(text);
	
	*iter = active;
	return TRUE;*/
}

gboolean ch_pango_node_parse_underscore_cb(
	guint level,
	const GtkTextIter* start,
	const GtkTextIter* start_inner,
	const GtkTextIter* end_inner,
	const GtkTextIter* end,
	ChTree* tree
) {
	if (level != 2)
		return FALSE;
	
	gchar* text = gtk_text_iter_get_text(start, end);
	gchar* inner_text = gtk_text_iter_get_text(start_inner, end_inner);
	
	g_autoptr(Pango2AttrList) attrs = pango2_attr_list_new();
	pango2_attr_list_insert(attrs, pango2_attr_underline_new(PANGO2_LINE_STYLE_SOLID));
	ChNode* node = ch_pango_node_new(text, inner_text, attrs, '_');
	ch_tree_add_node(tree, node);

	g_free(inner_text);
	g_free(text);
	return TRUE;
}
gboolean ch_pango_node_parse_underscore(ChTree* tree, GtkTextIter* iter, gpointer) {
	return ch_pango_parse_generic(iter, '_', 3, (ChPangoParseGenericCb)ch_pango_node_parse_underscore_cb, tree);
	/*GtkTextIter active = *iter;
	
	guint level = 0;
	while (gtk_text_iter_get_char(&active) == '_' && level <= 2) {
		level++; gtk_text_iter_forward_char(&active);
	}

	if (level != 2)
		return FALSE;

	GtkTextIter begin_inner = active;
	
	// Find matching
	while (gtk_text_iter_forward_find_char(&active, ch_pango_node_check_char, GUINT_TO_POINTER('_'), NULL)) {
		guint endlevel = 0;
		while (gtk_text_iter_get_char(&active) == '_' && level <= 2) {
			endlevel++; gtk_text_iter_forward_char(&active);
		}
		if (endlevel == level)
			break;
	}
	
	gchar* text = gtk_text_iter_get_text(iter, &active);
	g_print("Creating pango underscore node with text: %s\n", text);
	g_free(text);
	
	*iter = active;
	return TRUE;*/
}

gboolean ch_pango_node_parse_tilde_cb(
	guint level,
	const GtkTextIter* start,
	const GtkTextIter* start_inner,
	const GtkTextIter* end_inner,
	const GtkTextIter* end,
	ChTree* tree
) {
	if (level != 2)
		return FALSE;
	
	gchar* text = gtk_text_iter_get_text(start, end);
	gchar* inner_text = gtk_text_iter_get_text(start_inner, end_inner);

	g_autoptr(Pango2AttrList) attrs = pango2_attr_list_new();
	pango2_attr_list_insert(attrs, pango2_attr_strikethrough_new(PANGO2_LINE_STYLE_SOLID));
	ChNode* node = ch_pango_node_new(text, inner_text, attrs, '~');
	ch_tree_add_node(tree, node);

	g_free(inner_text);
	g_free(text);
	return TRUE;
}
gboolean ch_pango_node_parse_tilde(ChTree* tree, GtkTextIter* iter, gpointer) {
	return ch_pango_parse_generic(iter, '~', 3, (ChPangoParseGenericCb)ch_pango_node_parse_tilde_cb, tree);
	/*GtkTextIter active = *iter;
	
	guint level = 0;
	while (gtk_text_iter_get_char(&active) == '~' && level <= 2) {
		level++; gtk_text_iter_forward_char(&active);
	}

	if (level != 2)
		return FALSE;

	GtkTextIter begin_inner = active;
	
	// Find matching
	while (gtk_text_iter_forward_find_char(&active, ch_pango_node_check_char, GUINT_TO_POINTER('~'), NULL)) {
		guint endlevel = 0;
		while (gtk_text_iter_get_char(&active) == '~' && level <= 2) {
			endlevel++; gtk_text_iter_forward_char(&active);
		}
		if (endlevel == level)
			break;
	}
	
	gchar* text = gtk_text_iter_get_text(iter, &active);
	g_print("Creating pango strikethrough node with text: %s\n", text);
	g_free(text);
	
	*iter = active;
	return TRUE;*/
}
