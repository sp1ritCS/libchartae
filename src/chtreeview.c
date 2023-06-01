#include "chtreeview.h"

static inline gint ch_tree_view_int_max(gint a, gint b) {
	return a < b ? b : a;
}

typedef struct {
	ChMeasurementLine* line;
	ChNode* node;
} ChTreeViewRunContainer;
static void ch_tree_view_run_container_free_inner(ChTreeViewRunContainer* self) {
	self->line->free(self->line);
	// self->node is currently not owned here (TODO: is that a good idea?)
}

typedef struct {
	// GArray<ChTreeViewRunContainer>
	GArray* runs;
	gint offset;
	gint height;
} ChTreeViewLineContainer;
static ChTreeViewLineContainer* ch_tree_view_line_container_new(void) {
	ChTreeViewLineContainer* self = g_new(ChTreeViewLineContainer, 1);
	self->runs = g_array_new(FALSE, FALSE, sizeof(ChTreeViewRunContainer));
	g_array_set_clear_func(self->runs, (GDestroyNotify)ch_tree_view_run_container_free_inner);
	self->offset = -1;
	self->height = -1; 
	return self;
}
static void ch_tree_view_line_container_free(ChTreeViewLineContainer* self) {
	g_return_if_fail(self != NULL);
	g_array_unref(self->runs);
	g_free(self);
}

struct _ChTreeView {
	GtkWidget parent_instance;
	
	ChTree* tree;
	// GPtrArray<ChTreeViewLineContainer>
	GPtrArray* lines;
};

G_DEFINE_TYPE (ChTreeView, ch_tree_view, GTK_TYPE_WIDGET)

enum {
 	PROP_TREE = 1,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static void ch_tree_view_object_dispose(GObject* object) {
	ChTreeView* self = CH_TREE_VIEW(object);
	g_clear_object(&self->tree);
	
	G_OBJECT_CLASS(ch_tree_view_parent_class)->dispose(object);
}

static void ch_tree_view_object_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	ChTreeView* self = CH_TREE_VIEW(object);
	
	switch (prop_id) {
		case PROP_TREE:
			g_value_set_object(value, ch_tree_view_get_tree(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void ch_tree_view_object_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
	ChTreeView* self = CH_TREE_VIEW(object);
	
	switch (prop_id) {
		case PROP_TREE:
			ch_tree_view_set_tree(self, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void ch_tree_view_widget_snapshot(GtkWidget* widget, GtkSnapshot* snapshot) {
	ChTreeView* self = CH_TREE_VIEW(widget);
	
	gint height = 0;
	for (guint i = 0; i < self->lines->len; i++) {
		ChTreeViewLineContainer* container = self->lines->pdata[i];
		
		for (guint j = 0; j < container->runs->len; j++) {
			ChTreeViewRunContainer* run = &g_array_index(container->runs, ChTreeViewRunContainer, j);

			GskRenderNode* node = ch_node_render_line(run->node, run->line, FALSE);
			if (node) {
				GskTransform* translation = gsk_transform_new();
				translation = gsk_transform_translate(translation,
					&GRAPHENE_POINT_INIT( pango2_units_to_double(run->line->extents.x), pango2_units_to_double(height + run->line->extents.y) )
				);
				
				GskRenderNode* translated = gsk_transform_node_new(node, translation);
				gtk_snapshot_append_node(snapshot, translated);
				gsk_render_node_unref(translated);
			}
		}

		height += container->height;
		//height += priv->spacing;
		height += 6;
	}
}

static void ch_tree_view_class_init(ChTreeViewClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(class);
	
	object_class->dispose = ch_tree_view_object_dispose;
	object_class->get_property = ch_tree_view_object_get_property;
	object_class->set_property = ch_tree_view_object_set_property;
	
	widget_class->snapshot = ch_tree_view_widget_snapshot;
	
	obj_properties[PROP_TREE] = g_param_spec_object("tree", "Tree", "The tree that should be rendered", CH_TYPE_TREE, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void ch_tree_view_init(ChTreeView* self) {
	self->tree = NULL;
	self->lines = g_ptr_array_new_with_free_func((GDestroyNotify)ch_tree_view_line_container_free);
}

GtkWidget* ch_tree_view_new(ChTree* tree) {
	return g_object_new(CH_TYPE_TREE_VIEW, "tree", tree, NULL);
}

static gint ch_tree_view_make_line(ChTreeView* self, ChTreeViewLineContainer* line) {
	gint width = pango2_units_from_double(gtk_widget_get_width(GTK_WIDGET(self)));
	
	gint tallest_y = 0;
	gint tallest_baseline = 0;
	for (guint i = 0; i < line->runs->len; i++) {
		ChTreeViewRunContainer* run = &g_array_index(line->runs, ChTreeViewRunContainer, i);
		tallest_y = ch_tree_view_int_max(tallest_y, -run->line->extents.y);
		tallest_baseline = ch_tree_view_int_max(tallest_baseline, run->line->extents.height + run->line->extents.y);
	}

	line->offset = tallest_y;
	line->height = tallest_y + tallest_baseline;
	
	for (guint i = 0; i < line->runs->len; i++) {
		ChTreeViewRunContainer* run = &g_array_index(line->runs, ChTreeViewRunContainer, i);
		
		//run->line->extents.y = line->height - run->line->extents.height - tallest_baseline + (run->line->extents.height + run->line->extents.y);
		// optimized to:
		run->line->extents.y = line->height - tallest_baseline + run->line->extents.y;
		
		run->line->extended.x = run->line->extents.x;
		run->line->extended.y = 0;
		run->line->extended.height = line->height;
		if (i + 1 < line->runs->len)
			run->line->extended.width = run->line->extents.width;
		else
			run->line->extended.width = width - run->line->extents.x;

	}

	g_ptr_array_add(self->lines, line);																						

	return line->height;
}
static void ch_tree_view_remeasure(ChTreeView* self) {
	g_ptr_array_set_size(self->lines, 0);
	
	if (!self->tree)
		return;
	
	gint width = pango2_units_from_double(gtk_widget_get_width(GTK_WIDGET(self)));
	gint x = 0;
	gint y = 0;

	ChNode** nodes;
	guint n_nodes = ch_tree_get_nodes(self->tree, &nodes);
	
	ChTreeViewLineContainer* line_container = ch_tree_view_line_container_new();
	for (guint i = 0; i < n_nodes; i++) {
		gpointer iter = NULL;
		ChMeasurementLine* line = NULL;
		while ((line = ch_node_get_line(nodes[i], x, width - x, &iter))) {
			ChTreeViewRunContainer run = {
				.line = line,
				.node = nodes[i]
			};
			g_array_append_val(line_container->runs, run);
			
			if (line->is_last) {
				x = x + line->extents.width;
			} else {
				x = 0;
				y += ch_tree_view_make_line(self, line_container);
				line_container = ch_tree_view_line_container_new();
			}
		}
	}
	ch_tree_view_make_line(self, line_container);
}

ChTree* ch_tree_view_get_tree(ChTreeView* self) {
	g_return_val_if_fail(CH_IS_TREE_VIEW(self), NULL);
	return self->tree;
}
void ch_tree_view_set_tree(ChTreeView* self, ChTree* tree) {
	g_return_if_fail(CH_IS_TREE_VIEW(self));
	g_return_if_fail(tree == NULL || CH_IS_TREE(tree));
	
	if (self->tree == tree)
		return;
	if (self->tree)
		g_object_unref(self->tree);
	self->tree = g_object_ref(tree);
	
	ch_tree_view_remeasure(self);
	gtk_widget_queue_resize(GTK_WIDGET(self));
	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_TREE]);
}
