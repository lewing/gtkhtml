/*  This file is part of the GtkHTML library.
 *
 *  Copyright 2002 Ximian, Inc.
 *
 *  Author: Radek Doulik
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <glib-object.h>
#include <atk/atkcomponent.h>
#include <atk/atkstateset.h>
#include <libgnome/gnome-i18n.h>

#include "html.h"
#include "utils.h"

#define HTML_ID "html-object"
#define HTML_A11Y_HTML(o) HTML_OBJECT (g_object_get_data (G_OBJECT (o), HTML_ID))

static void html_a11y_class_init (HTMLA11YClass *klass);
static void html_a11y_init       (HTMLA11Y *a11y_paragraph);

static void atk_component_interface_init (AtkComponentIface *iface);
static AtkObject*  html_a11y_get_parent (AtkObject *accessible);
static gint html_a11y_get_index_in_parent (AtkObject *accessible);
static AtkStateSet * html_a11y_ref_state_set (AtkObject *accessible);
static gint html_a11y_get_n_children (AtkObject *accessible);
static AtkObject * html_a11y_ref_child (AtkObject *accessible, gint index);

static AtkObjectClass *parent_class = NULL;

GType
html_a11y_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo tinfo = {
			sizeof (HTMLA11YClass),
			NULL,                                                      /* base init */
			NULL,                                                      /* base finalize */
			(GClassInitFunc) html_a11y_class_init,                     /* class init */
			NULL,                                                      /* class finalize */
			NULL,                                                      /* class data */
			sizeof (HTMLA11Y),                                         /* instance size */
			0,                                                         /* nb preallocs */
			(GInstanceInitFunc) html_a11y_init,                        /* instance init */
			NULL                                                       /* value table */
		};

		static const GInterfaceInfo atk_component_info = {
			(GInterfaceInitFunc) atk_component_interface_init,
			(GInterfaceFinalizeFunc) NULL,
			NULL
		};

		type = g_type_register_static (ATK_TYPE_OBJECT, "HTMLA11Y", &tinfo, 0);
		g_type_add_interface_static (type, ATK_TYPE_COMPONENT, &atk_component_info);
	}

	return type;
}

static void 
atk_component_interface_init (AtkComponentIface *iface)
{
	g_return_if_fail (iface != NULL);

	/* FIX2
	   iface->add_focus_handler = gail_widget_add_focus_handler;
	   iface->get_extents = gail_widget_get_extents;
	   iface->get_size = gail_widget_get_size;
	   iface->get_layer = gail_widget_get_layer;
	   iface->grab_focus = gail_widget_grab_focus;
	   iface->remove_focus_handler = gail_widget_remove_focus_handler;
	   iface->set_extents = gail_widget_set_extents;
	   iface->set_position = gail_widget_set_position;
	   iface->set_size = gail_widget_set_size;
	*/
}

static void
html_a11y_finalize (GObject *obj)
{
}

static void
html_a11y_initialize (AtkObject *obj, gpointer data)
{
	printf ("html_a11y_initialize\n");

	g_object_set_data (G_OBJECT (obj), HTML_ID, data);

	if (ATK_OBJECT_CLASS (parent_class)->initialize)
		ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);
}

static void
html_a11y_class_init (HTMLA11YClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	AtkObjectClass *atk_class = ATK_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	atk_class->initialize = html_a11y_initialize;
	atk_class->get_parent = html_a11y_get_parent;
	atk_class->get_index_in_parent = html_a11y_get_index_in_parent;
	atk_class->ref_state_set = html_a11y_ref_state_set;
	atk_class->get_n_children = html_a11y_get_n_children;
	atk_class->ref_child = html_a11y_ref_child;

	gobject_class->finalize = html_a11y_finalize;
}

static void
html_a11y_init (HTMLA11Y *a11y_paragraph)
{
}

static HTMLObject *
get_parent_html (AtkObject *accessible)
{
	HTMLObject *obj;

	obj = HTML_A11Y_HTML (accessible);

	return obj ? obj->parent : NULL;
}

static AtkObject* 
html_a11y_get_parent (AtkObject *accessible)
{
	AtkObject *parent;

	printf ("html_a11y_get_parent\n");
	parent = accessible->accessible_parent;

	if (parent != NULL)
		g_return_val_if_fail (ATK_IS_OBJECT (parent), NULL);
	else {
		HTMLObject *parent_obj;

		parent_obj = get_parent_html (accessible);
		if (parent_obj) {
			parent = HTML_OBJECT_ACCESSIBLE (parent_obj);
		}
	}

	printf ("html_a11y_get_parent resolve to %p\n", parent);

	return parent;
}

static gint
html_a11y_get_index_in_parent (AtkObject *accessible)
{
	HTMLObject *obj;
	gint index = -1;

	obj = HTML_A11Y_HTML (accessible);
	if (obj && obj->parent) {
		index = html_object_get_child_index (obj->parent, obj);
	}

	printf ("html_a11y_get_index_in_parent resolve to %d\n", index);

	return index;  
}

static AtkStateSet *
html_a11y_ref_state_set (AtkObject *accessible)
{
	AtkStateSet *state_set;

	if (ATK_OBJECT_CLASS (parent_class)->ref_state_set)
		state_set = ATK_OBJECT_CLASS (parent_class)->ref_state_set (accessible);
	if (!state_set)
		state_set = atk_state_set_new ();

	atk_state_set_add_state (state_set, ATK_STATE_VISIBLE);
	atk_state_set_add_state (state_set, ATK_STATE_ENABLED);

	printf ("html_a11y_ref_state_set resolves to %p\n", state_set);

	return state_set;
}

static gint
html_a11y_get_n_children (AtkObject *accessible)
{
	HTMLObject *clue;
	gint n_children = 0;

	/* clue = GTK_HTML_A11Y_GTKHTML (accessible)->engine->clue;
	if (clue)
	n_children = html_object_get_n_children (GTK_HTML_A11Y_GTKHTML (accessible)->engine->clue); */

	printf ("html_a11y_get_n_children resolves to %d\n", n_children);

	return n_children;
}

static AtkObject *
html_a11y_ref_child (AtkObject *accessible, gint index)
{
	HTMLObject *child;
	AtkObject *accessible_child = NULL;
	
	/* if (GTK_HTML_A11Y_GTKHTML (accessible)->engine->clue) {
		child = html_object_get_child (GTK_HTML_A11Y_GTKHTML (accessible)->engine->clue, index);
		if (child) {
			accessible_child = html_utils_get_accessible (child, accessible);
		}
		} */

	printf ("html_a11y_ref_child %d resolves to %p\n", index, accessible_child);

	return accessible_child;
}
