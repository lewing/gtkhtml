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
#include <atk/atkcomponent.h>

#include "htmltext.h"
#include "htmltextslave.h"

#include "html.h"
#include "text.h"

static void html_a11y_text_class_init    (HTMLA11YTextClass *klass);
static void html_a11y_text_init          (HTMLA11YText *a11y_text);
static void atk_component_interface_init (AtkComponentIface *iface);
static void html_a11y_text_get_extents   (AtkComponent *component,
					  gint *x, gint *y, gint *width, gint *height, AtkCoordType coord_type);
static void html_a11y_text_get_size      (AtkComponent *component, gint *width, gint *height);

static AtkObjectClass *parent_class = NULL;

GType
html_a11y_text_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo tinfo = {
			sizeof (HTMLA11YTextClass),
			NULL,                                                      /* base init */
			NULL,                                                      /* base finalize */
			(GClassInitFunc) html_a11y_text_class_init,           /* class init */
			NULL,                                                      /* class finalize */
			NULL,                                                      /* class data */
			sizeof (HTMLA11YText),                                /* instance size */
			0,                                                         /* nb preallocs */
			(GInstanceInitFunc) html_a11y_text_init,              /* instance init */
			NULL                                                       /* value table */
		};

		static const GInterfaceInfo atk_component_info = {
			(GInterfaceInitFunc) atk_component_interface_init,
			(GInterfaceFinalizeFunc) NULL,
			NULL
		};

		type = g_type_register_static (G_TYPE_HTML_A11Y, "HTMLA11YText", &tinfo, 0);
		g_type_add_interface_static (type, ATK_TYPE_COMPONENT, &atk_component_info);
	}

	return type;
}

static void 
atk_component_interface_init (AtkComponentIface *iface)
{
	g_return_if_fail (iface != NULL);

	iface->get_extents = html_a11y_text_get_extents;
	iface->get_size = html_a11y_text_get_size;
}

static void
html_a11y_text_finalize (GObject *obj)
{
}

static void
html_a11y_text_initialize (AtkObject *obj, gpointer data)
{
	printf ("html_a11y_text_initialize\n");

	if (ATK_OBJECT_CLASS (parent_class)->initialize)
		ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);
}

static void
html_a11y_text_class_init (HTMLA11YTextClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	AtkObjectClass *atk_class = ATK_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	atk_class->initialize = html_a11y_text_initialize;
	gobject_class->finalize = html_a11y_text_finalize;
}

static void
html_a11y_text_init (HTMLA11YText *a11y_text)
{
}

AtkObject* 
html_a11y_text_new (HTMLObject *html_obj)
{
	GObject *object;
	AtkObject *accessible;

	g_return_val_if_fail (HTML_IS_TEXT (html_obj), NULL);

	object = g_object_new (G_TYPE_HTML_A11Y_TEXT, NULL);

	accessible = ATK_OBJECT (object);
	atk_object_initialize (accessible, html_obj);

	accessible->role = ATK_ROLE_TEXT;

	/* printf ("created new html accessible text object\n"); */

	return accessible;
}

static void
get_size (HTMLObject *obj, gint *width, gint *height)
{
	HTMLObject *last;

	if (obj) {
		gint ax, ay;

		html_object_calc_abs_position (obj, &ax, &ay);
		last = obj;
		while (last->next && HTML_IS_TEXT_SLAVE (last->next))
			last = last->next;
		if (HTML_IS_TEXT_SLAVE (last)) {
			gint lx, ly;
			html_object_calc_abs_position (last, &lx, &ly);

			*width = lx + last->width - ax;
			*height = ly + last->descent - ay;
		}
	}
}

static void
html_a11y_text_get_extents (AtkComponent *component, gint *x, gint *y, gint *width, gint *height, AtkCoordType coord_type)
{
	HTMLObject *obj = HTML_A11Y_HTML (component);

	html_a11y_get_extents (component, x, y, width, height, coord_type);
	get_size (obj, width, height);
}

static void
html_a11y_text_get_size (AtkComponent *component, gint *width, gint *height)
{
	HTMLObject *obj = HTML_A11Y_HTML (component);

	html_a11y_get_size (component, width, height);
	get_size (obj, width, height);
}
