/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.
    Authors:           Radek Doulik (rodo@helixcode.com)

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

#include <config.h>
#include <libgnome/gnome-i18n.h>
#include "htmlengine-edit-clueflowstyle.h"
#include "htmlengine-save.h"
#include "htmlselection.h"
#include "paragraph.h"
#include "properties.h"
#include "utils.h"

struct _GtkHTMLEditParagraphProperties {
	GtkHTMLControlData *cd;
	GtkWidget *style_option;

	GtkHTMLParagraphAlignment align;
	gboolean align_changed;

	GtkHTMLParagraphStyle     style;
	gboolean style_changed;

	GtkHTML *sample;

	HTMLClueFlow *flow;
};
typedef struct _GtkHTMLEditParagraphProperties GtkHTMLEditParagraphProperties;

static void
fill_sample (GtkHTMLEditParagraphProperties *d)
{
	gchar *body, *bg, *style, *align;

	bg    = html_engine_save_get_sample_body (d->cd->html->engine, NULL);
	align = html_engine_save_get_paragraph_align (d->align)
		? g_strdup_printf ("<div align=%s>", html_engine_save_get_paragraph_align (d->align)) : g_strdup ("");
	style = html_engine_save_get_paragraph_style (d->style)
		? g_strdup_printf ("<%s>", html_engine_save_get_paragraph_style (d->style)) : g_strdup ("");
	body  = g_strconcat (bg,
			     style,
			     align,
			     _("The quick brown fox jumps over the lazy dog.</div>"),
			     NULL);

	gtk_html_load_from_string (d->sample, body, -1);
	g_free (style);
	g_free (align);
	g_free (bg);
	g_free (body);
}

static void
set_style (GtkWidget *w, GtkHTMLEditParagraphProperties *data)
{
	GtkHTMLParagraphStyle style = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (w), "style"));

	if (data->style != style) {
		gtk_html_edit_properties_dialog_change (data->cd->properties_dialog);
		data->style_changed = TRUE;
		data->style = style;
		fill_sample (data);
	}
}

static void
set_align (GtkWidget *w, GtkHTMLEditParagraphProperties *data)
{
	GtkHTMLParagraphAlignment align = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (w), "align"));
	if (align != data->align && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w))) {
		data->align = align;
		data->align_changed = TRUE;
		gtk_html_edit_properties_dialog_change (data->cd->properties_dialog);
		fill_sample (data);
	}
}

GtkWidget *
paragraph_properties (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkHTMLEditParagraphProperties *data = g_new0 (GtkHTMLEditParagraphProperties, 1);
	GtkWidget *hbox, *menu, *menuitem, *frame, *radio, *table;
	GSList *group;
	gint h=0, i=0;

	*set_data = data;
	data->cd = cd;
	data->align = gtk_html_get_paragraph_alignment (cd->html);
	data->style = gtk_html_get_paragraph_style     (cd->html);
	data->flow  = HTML_CLUEFLOW (cd->html->engine->cursor->object->parent);

	table = gtk_table_new (2, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 12);
	gtk_table_set_col_spacings (GTK_TABLE (table), 12);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);

	frame = gtk_frame_new (_("Style"));
	hbox = gtk_hbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
	menu = gtk_menu_new ();

#undef ADD_ITEM
#define ADD_ITEM(n,s) \
	menuitem = gtk_menu_item_new_with_label (n); \
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem); \
        gtk_widget_show (menuitem); \
        if (data->style == s) h=i; i++; \
        g_signal_connect (menuitem, "activate", G_CALLBACK (set_style), data); \
        g_object_set_data (G_OBJECT (menuitem), "style", GINT_TO_POINTER (s));

	ADD_ITEM (_("Normal"),       GTK_HTML_PARAGRAPH_STYLE_NORMAL);
	ADD_ITEM (_("Pre"),          GTK_HTML_PARAGRAPH_STYLE_PRE);
	ADD_ITEM (_("Header 1"),     GTK_HTML_PARAGRAPH_STYLE_H1);
	ADD_ITEM (_("Header 2"),     GTK_HTML_PARAGRAPH_STYLE_H2);
	ADD_ITEM (_("Header 3"),     GTK_HTML_PARAGRAPH_STYLE_H3);
	ADD_ITEM (_("Header 4"),     GTK_HTML_PARAGRAPH_STYLE_H4);
	ADD_ITEM (_("Header 5"),     GTK_HTML_PARAGRAPH_STYLE_H5);
	ADD_ITEM (_("Header 6"),     GTK_HTML_PARAGRAPH_STYLE_H6);
	ADD_ITEM (_("Address"),      GTK_HTML_PARAGRAPH_STYLE_ADDRESS);
	ADD_ITEM (_("Dot item"),     GTK_HTML_PARAGRAPH_STYLE_ITEMDOTTED);
	ADD_ITEM (_("Number item"),   GTK_HTML_PARAGRAPH_STYLE_ITEMDIGIT);
	ADD_ITEM (_("Roman item"),   GTK_HTML_PARAGRAPH_STYLE_ITEMROMAN);
	ADD_ITEM (_("Alphabeta item"),   GTK_HTML_PARAGRAPH_STYLE_ITEMALPHA);

	data->style_option = gtk_option_menu_new ();
	gtk_option_menu_set_menu (GTK_OPTION_MENU (data->style_option), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (data->style_option), h);
	gtk_box_pack_start (GTK_BOX (hbox), data->style_option, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), hbox);
	gtk_table_attach (GTK_TABLE (table), frame, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

	frame = gtk_frame_new (_("Align"));
	hbox = gtk_hbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);

#define ADD_RADIO(x,a) \
	radio = gtk_radio_button_new_with_label (group, x); \
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio)); \
	gtk_box_pack_start (GTK_BOX (hbox), radio, FALSE, FALSE, 0); \
        if (a == data->align) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE); \
        g_signal_connect (radio, "toggled", G_CALLBACK (set_align), data); \
        g_object_set_data (G_OBJECT (radio), "align", GINT_TO_POINTER (a));

	group = NULL;
	ADD_RADIO (_("Left"), GTK_HTML_PARAGRAPH_ALIGNMENT_LEFT);
	ADD_RADIO (_("Center"), GTK_HTML_PARAGRAPH_ALIGNMENT_CENTER);
	ADD_RADIO (_("Right"), GTK_HTML_PARAGRAPH_ALIGNMENT_RIGHT);

	gtk_container_add (GTK_CONTAINER (frame), hbox);
	gtk_table_attach (GTK_TABLE (table), frame, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	gtk_table_attach (GTK_TABLE (table), sample_frame (&data->sample), 0, 2, 1, 2, GTK_FILL | GTK_EXPAND, 0, 0, 0);
	fill_sample (data);

	gtk_widget_show_all (table);

	return table;
}

gboolean
paragraph_apply_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditParagraphProperties *data = (GtkHTMLEditParagraphProperties *) get_data;

	if (data->align_changed || data->style_changed) {
		HTMLEngine *e = cd->html->engine;
		gint position;

		position = e->cursor->position;

		if (!html_engine_is_selection_active (e) && e->cursor->object->parent != HTML_OBJECT (data->flow))
			if (!html_cursor_jump_to (e->cursor, e, html_object_head (HTML_OBJECT (data->flow)), 0)) {
				GtkWidget *dialog;
				printf ("d: %p\n", data->cd->properties_dialog);
				dialog = gtk_message_dialog_new (GTK_WINDOW (data->cd->properties_dialog->dialog),
								 GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
								 _("The editted paragraph was removed from the document.\nCannot apply your changes."));
				gtk_dialog_run (GTK_DIALOG (dialog));
				gtk_widget_destroy (dialog);
				html_cursor_jump_to_position (e->cursor, e, position);
				return FALSE;
			}

		if (data->align_changed)
			gtk_html_set_paragraph_alignment (cd->html, data->align);
		if (data->style_changed)
			gtk_html_set_paragraph_style (cd->html, data->style);
		html_cursor_jump_to_position (e->cursor, e, position);
	}

	return TRUE;
}

void
paragraph_close_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	g_free (get_data);
}