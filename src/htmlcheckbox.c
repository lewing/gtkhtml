/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Jonas Borgstr�m <jonas_b@bitsmart.com>.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include "htmlcheckbox.h"

HTMLCheckBoxClass html_checkbox_class;

static void
reset (HTMLEmbedded *e)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(e->widget), HTML_CHECKBOX(e)->default_checked);
}

void
html_checkbox_type_init (void)
{
	html_checkbox_class_init (&html_checkbox_class, HTML_TYPE_CHECKBOX);
}

void
html_checkbox_class_init (HTMLCheckBoxClass *klass,
			HTMLType type)
{
	HTMLEmbeddedClass *element_class;
	HTMLObjectClass *object_class;


	element_class = HTML_EMBEDDED_CLASS (klass);
	object_class = HTML_OBJECT_CLASS (klass);

	html_embedded_class_init (element_class, type);

	/* HTMLEmbedded methods.   */
	element_class->reset = reset;
}

void
html_checkbox_init (HTMLCheckBox *checkbox, 
		    HTMLCheckBoxClass *klass, 
		    GtkWidget *parent, 
		    gchar *name, 
		    gchar *value, 
		    gboolean checked) 
{
	HTMLEmbedded *element;
	HTMLObject *object;
	GtkRequisition req;

	element = HTML_EMBEDDED (checkbox);
	object = HTML_OBJECT (checkbox);

	html_embedded_init (element, HTML_EMBEDDED_CLASS (klass), parent, name, value);

	element->widget = gtk_toggle_button_new();

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(element->widget), checked);

	checkbox->default_checked = checked;

	gtk_widget_size_request(element->widget, &req);

	object->descent = 0;
	object->width = req.width;
	object->ascent = req.height;

	/*	gtk_widget_show(element->widget);
		gtk_layout_put(GTK_LAYOUT(parent), element->widget, 0, 0);*/
}

HTMLObject *
html_checkbox_new (GtkWidget *parent, 
		   gchar *name, 
		   gchar *value, 
		   gboolean checked)
{
	HTMLCheckBox *checkbox;

	checkbox = g_new0 (HTMLCheckBox, 1);
	html_checkbox_init (checkbox, &html_checkbox_class, parent, name, value, checked);

	return HTML_OBJECT (checkbox);
}
