/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2001 Ximian, Inc.
    Authors:           Radek Doulik (rodo@ximian.com)

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
#include <string.h>
#include <glade/glade.h>
#include "gi-color-combo.h"

#include "gtkhtml.h"

#include "htmlclue.h"
#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlengine-edit-table.h"
#include "htmlengine-save.h"
#include "htmlimage.h"
#include "htmltable.h"
#include "htmlsettings.h"

#include "properties.h"
#include "table.h"
#include "utils.h"

typedef struct
{	
	GtkHTMLControlData *cd;

	HTMLTable *table;

	GtkWidget *combo_bg_color;
	GtkWidget *entry_bg_pixmap;
	GtkWidget *spin_spacing;
	GtkWidget *spin_padding;
	GtkWidget *spin_border;
	GtkWidget *option_align;

	gboolean   has_width;
	gboolean   changed_width;
	gint       width;
	gboolean   width_percent;
	GtkWidget *spin_width;
	GtkWidget *check_width;
	GtkWidget *option_width;

	GtkWidget *spin_cols;
	GtkWidget *spin_rows;

	gint       template;
	GtkWidget *option_template;

	gboolean   disable_change;
	gboolean   insert;
} GtkHTMLEditTableProperties;

static void set_insert_ui (GtkHTMLEditTableProperties *d);

#define TEMPLATES 3
typedef struct {
	gchar *name;
	gint offset;

	gint default_width;
	gboolean default_percent;

	HTMLHAlignType default_align;

	gint default_border;
	gint default_spacing;
	gint default_padding;

	gint default_rows;
	gint default_columns;
	gboolean set_rows, set_columns;

	gchar *table_begin;
	gchar *table_end;
	gchar *cell_begin;
	gchar *cell_end;
} TableInsertTemplate;


static TableInsertTemplate table_templates [TEMPLATES] = {
	{
		N_("Plain"), 1,
		100, TRUE, HTML_HALIGN_CENTER, 1, 2, 1, 3, 3, FALSE, FALSE,
		"<table border=@border@ cellspacing=@spacing@ cellpadding=@padding@@align@@width@>",
		"</table>",
		"<td>",
		"</td>"
	},
	{
		N_("Flat gray"), 2,
		100, TRUE, HTML_HALIGN_CENTER, 0, 1, 3, 3, 3, FALSE, FALSE,
		"<table cellspacing=0 cellpadding=@border@ bgcolor=\"#bfbfbf\"@width@@align@><tr><td>"
		"<table bgcolor=\"#f2f2f2\" cellspacing=@spacing@ cellpadding=@padding@ width=\"100%\">",
		"</table></td></tr></table>",
		"<td>",
		"</td>"
	},
	{
		N_("Dark header"), 11,
		100, TRUE, HTML_HALIGN_CENTER, 0, 1, 3, 3, 3, FALSE, FALSE,
		"<table bgcolor=\"#7f7f7f\" cellpadding=3 cellspacing=0>"
		"<tr><td><font color=\"#ffffff\"><b>Header</td></tr></table>"
		"<table cellspacing=0 cellpadding=@border@ bgcolor=\"#bfbfbf\"@width@@align@><tr><td>"
		"<table bgcolor=\"#f2f2f2\" cellspacing=@spacing@ cellpadding=@padding@ width=\"100%\">",
		"</table></td></tr></table>",
		"<td>",
		"</td>"
	},
};

static gchar *
substitute_int (gchar *str, const gchar *var_name, gint value)
{
	gchar *substr;

	substr = strstr (str, var_name);
	if (substr) {
		gchar *new_str;

		*substr = 0;
		new_str = g_strdup_printf ("%s%d%s", str, value, substr + strlen (var_name));
		g_free (str);
		str = new_str;
	}

	return str;
}

static gchar *
substitute_char (gchar *str, const gchar *var_name, const gchar *value)
{
	gchar *substr;

	substr = strstr (str, var_name);
	if (substr) {
		gchar *new_str;

		*substr = 0;
		new_str = g_strdup_printf ("%s%s%s", str, value, substr + strlen (var_name));
		g_free (str);
		str = new_str;
	}

	return str;
}

static GtkHTMLEditTableProperties *
data_new (GtkHTMLControlData *cd)
{
	GtkHTMLEditTableProperties *data = g_new0 (GtkHTMLEditTableProperties, 1);

	/* fill data */
	data->cd                = cd;
	data->table             = NULL;
	data->has_width         = TRUE;
	data->width_percent     = TRUE;
	data->width             = 100;

	return data;
}

static void
changed_bg_color (GtkWidget *w, GdkColor *color, gboolean custom, gboolean by_user, gboolean is_default, GtkHTMLEditTableProperties *d)
{
	/* If the color was changed programatically there's not need to set things */
	if (!by_user)
		return;

	html_engine_table_set_bg_color (d->cd->html->engine, d->table, color);		
}

static void
changed_bg_pixmap (GtkWidget *w, GtkHTMLEditTableProperties *d)
{
	char *url;
	const char *file;

	if (d->disable_change || !editor_has_html_object (d->cd, HTML_OBJECT (d->table)))
		return;

	file = gtk_entry_get_text (GTK_ENTRY (w));
	if (file && *file)
		url = g_strconcat ("file://", file, NULL);
	else
		url = NULL;
	html_engine_table_set_bg_pixmap (d->cd->html->engine, d->table, url);
	g_free (url);
}

static void
changed_spacing (GtkWidget *w, GtkHTMLEditTableProperties *d)
{
	if (d->disable_change || !editor_has_html_object (d->cd, HTML_OBJECT (d->table)))
		return;

	html_engine_table_set_spacing (d->cd->html->engine, d->table, gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_spacing)), FALSE);
}

static void
changed_padding (GtkWidget *w, GtkHTMLEditTableProperties *d)
{
	if (d->disable_change || !editor_has_html_object (d->cd, HTML_OBJECT (d->table)))
		return;

	html_engine_table_set_padding (d->cd->html->engine, d->table, gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_padding)), FALSE);
}

static void
changed_border (GtkWidget *w, GtkHTMLEditTableProperties *d)
{
	if (d->disable_change || !editor_has_html_object (d->cd, HTML_OBJECT (d->table)))
		return;

	html_engine_table_set_border_width (d->cd->html->engine, d->table, gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_border)), FALSE);
}

static void
changed_align (GtkWidget *w, GtkHTMLEditTableProperties *d)
{
	if (d->disable_change || !editor_has_html_object (d->cd, HTML_OBJECT (d->table)))
		return;

	html_engine_table_set_align (d->cd->html->engine, d->table,
					     g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w))) + HTML_HALIGN_LEFT);
}

static void
changed_width (GtkWidget *w, GtkHTMLEditTableProperties *d)
{
	d->width = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_width));
	if (!d->disable_change) {
		d->disable_change = TRUE;
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_width), TRUE);
		d->disable_change = FALSE;
		d->changed_width = TRUE;
	}
}

static void
set_has_width (GtkWidget *check, GtkHTMLEditTableProperties *d)
{
	d->has_width = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (d->check_width));
	if (!d->disable_change)
		d->changed_width = TRUE;
}

static void
changed_width_percent (GtkWidget *w, GtkHTMLEditTableProperties *d)
{
	d->width_percent = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w))) ? TRUE : FALSE;
	if (!d->disable_change)
		d->changed_width = TRUE;
}

static void
changed_cols (GtkWidget *w, GtkHTMLEditTableProperties *d)
{
	if (d->disable_change || !editor_has_html_object (d->cd, HTML_OBJECT (d->table)))
		return;

	html_engine_table_set_cols (d->cd->html->engine, gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_cols)));
}

static void
changed_rows (GtkWidget *w, GtkHTMLEditTableProperties *d)
{
	if (d->disable_change || !editor_has_html_object (d->cd, HTML_OBJECT (d->table)))
		return;

	html_engine_table_set_rows (d->cd->html->engine, gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_rows)));
}

static void
changed_template (GtkWidget *w, GtkHTMLEditTableProperties *d)
{
	/*d->template = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w)));

	d->border  = table_templates [d->template].default_border;
	d->spacing = table_templates [d->template].default_spacing;
	d->padding = table_templates [d->template].default_padding;

	if (table_templates [d->template].set_rows)
		d->rows    = table_templates [d->template].default_rows;
	if (table_templates [d->template].set_columns)
		d->cols    = table_templates [d->template].default_columns;

		set_insert_ui (d);*/
}

/*
 * FIX: set spin adjustment upper to 100000
 *      as glade cannot set it now
 */
#define UPPER_FIX(x) gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (d->spin_ ## x))->upper = 100000.0

static GtkWidget *
table_widget (GtkHTMLEditTableProperties *d)
{
	HTMLColor *color;
	GtkWidget *table_page;
	GladeXML *xml;

	xml = glade_xml_new (GLADE_DATADIR "/gtkhtml-editor-properties.glade", "table_page", GETTEXT_PACKAGE);
	if (!xml)
		g_error (_("Could not load glade file."));

	table_page = glade_xml_get_widget (xml, "table_page");

	gtk_box_pack_start (GTK_BOX (glade_xml_get_widget (xml, "table_rows_hbox")),
			    gtk_image_new_from_file (ICONDIR "/table-row-16.png"),
			    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (glade_xml_get_widget (xml, "table_cols_hbox")),
			    gtk_image_new_from_file (ICONDIR "/table-column-16.png"),
			    FALSE, FALSE, 0);

        color = html_colorset_get_color (d->cd->html->engine->defaultSettings->color_set, HTMLBgColor);
	html_color_alloc (color, d->cd->html->engine->painter);
	d->combo_bg_color = color_combo_new (NULL, _("Transparent"), &color->color,
					     color_group_fetch ("table_bg_color", d->cd));
        color_combo_box_set_preview_relief (COLOR_COMBO (d->combo_bg_color), GTK_RELIEF_NORMAL); \
        g_signal_connect (d->combo_bg_color, "color_changed", G_CALLBACK (changed_bg_color), d);
	gtk_box_pack_start (GTK_BOX (glade_xml_get_widget (xml, "bg_color_hbox")), d->combo_bg_color, FALSE, FALSE, 0);

	d->entry_bg_pixmap = glade_xml_get_widget (xml, "entry_table_bg_pixmap");
	g_signal_connect (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (d->entry_bg_pixmap)),
			  "changed", G_CALLBACK (changed_bg_pixmap), d);

	d->spin_spacing = glade_xml_get_widget (xml, "spin_spacing");
	g_signal_connect (d->spin_spacing, "value_changed", G_CALLBACK (changed_spacing), d);
	d->spin_padding = glade_xml_get_widget (xml, "spin_padding");
	g_signal_connect (d->spin_padding, "value_changed", G_CALLBACK (changed_padding), d);
	d->spin_border  = glade_xml_get_widget (xml, "spin_border");
	g_signal_connect (d->spin_border, "value_changed", G_CALLBACK (changed_border), d);
	UPPER_FIX (padding);
	UPPER_FIX (spacing);
	UPPER_FIX (border);

	d->option_align = glade_xml_get_widget (xml, "option_table_align");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_align)), "selection-done",
			  G_CALLBACK (changed_align), d);

	d->spin_width   = glade_xml_get_widget (xml, "spin_table_width");
	g_signal_connect (d->spin_width, "value_changed", G_CALLBACK (changed_width), d);
	UPPER_FIX (width);
	d->check_width  = glade_xml_get_widget (xml, "check_table_width");
	g_signal_connect (d->check_width, "toggled", G_CALLBACK (set_has_width), d);
	d->option_width = glade_xml_get_widget (xml, "option_table_width");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_width)), "selection-done",
			  G_CALLBACK (changed_width_percent), d);

	d->spin_cols = glade_xml_get_widget (xml, "spin_table_columns");
	g_signal_connect (d->spin_cols, "value_changed", G_CALLBACK (changed_cols), d);
	d->spin_rows = glade_xml_get_widget (xml, "spin_table_rows");
	g_signal_connect (d->spin_rows, "value_changed", G_CALLBACK (changed_rows), d);
	UPPER_FIX (cols);
	UPPER_FIX (rows);

	gtk_widget_show_all (table_page);
	gnome_pixmap_entry_set_preview (GNOME_PIXMAP_ENTRY (d->entry_bg_pixmap), FALSE);

	return table_page;
}

static void
fill_templates (GtkHTMLEditTableProperties *d)
{
	GtkWidget *menu;
	gint i;

	menu = gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_template));

	for (i = 0; i < TEMPLATES; i ++)
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_menu_item_new_with_label (_(table_templates [i].name)));
	gtk_menu_set_active (GTK_MENU (menu), 0);
	gtk_container_remove (GTK_CONTAINER (menu), gtk_menu_get_active (GTK_MENU (menu)));
}

static GtkWidget *
table_insert_widget (GtkHTMLEditTableProperties *d)
{
	GtkWidget *table_page;
	GladeXML *xml;

	d->insert = TRUE;
	xml       = glade_xml_new (GLADE_DATADIR "/gtkhtml-editor-properties.glade", "table_insert_page", GETTEXT_PACKAGE);
	if (!xml)
		g_error (_("Could not load glade file."));

	table_page = glade_xml_get_widget (xml, "table_insert_page");

	d->spin_cols = glade_xml_get_widget (xml, "spin_table_columns");
	g_signal_connect (d->spin_cols, "value_changed", G_CALLBACK (changed_cols), d);
	d->spin_rows = glade_xml_get_widget (xml, "spin_table_rows");
	g_signal_connect (d->spin_rows, "value_changed", G_CALLBACK (changed_rows), d);
	UPPER_FIX (cols);
	UPPER_FIX (rows);

	d->spin_width   = glade_xml_get_widget (xml, "spin_table_width");
	UPPER_FIX (width);
	g_signal_connect (d->spin_width, "value_changed", G_CALLBACK (changed_width), d);
	d->check_width  = glade_xml_get_widget (xml, "check_table_width");
	g_signal_connect (d->check_width, "toggled", G_CALLBACK (set_has_width), d);
	d->option_width = glade_xml_get_widget (xml, "option_table_width");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_width)), "selection-done",
			  G_CALLBACK (changed_width_percent), d);

	d->spin_spacing = glade_xml_get_widget (xml, "spin_spacing");
	g_signal_connect (d->spin_spacing, "value_changed", G_CALLBACK (changed_spacing), d);
	d->spin_padding = glade_xml_get_widget (xml, "spin_padding");
	g_signal_connect (d->spin_padding, "value_changed", G_CALLBACK (changed_padding), d);
	d->spin_border  = glade_xml_get_widget (xml, "spin_border");
	g_signal_connect (d->spin_border, "value_changed", G_CALLBACK (changed_border), d);
	UPPER_FIX (padding);
	UPPER_FIX (spacing);
	UPPER_FIX (border);

	d->option_template = glade_xml_get_widget (xml, "option_table_template");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_template)), "selection-done",
			  G_CALLBACK (changed_template), d);
	fill_templates (d);

	gtk_widget_show_all (table_page);

	return table_page;
}

static void
set_ui (GtkHTMLEditTableProperties *d)
{
	if (editor_has_html_object (d->cd, HTML_OBJECT (d->table))) {
		HTMLHAlignType halign;

		d->disable_change = TRUE;

		color_combo_set_color (COLOR_COMBO (d->combo_bg_color), d->table->bgColor);

		if (d->table->bgPixmap) {
			int off = 0;

			if (!strncasecmp ("file://", d->table->bgPixmap->url, 7))
				off = 7;
			else if (!strncasecmp ("file:", d->table->bgPixmap->url, 5))
				off = 5;

			gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (d->entry_bg_pixmap))),
					    d->table->bgPixmap->url + off);
		}

		gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_spacing), d->table->spacing);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_padding), d->table->padding);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_border),  d->table->border);

		g_return_if_fail (HTML_OBJECT (d->table)->parent);
		halign = HTML_CLUE (HTML_OBJECT (d->table)->parent)->halign;
		if (halign == HTML_HALIGN_NONE)
			halign = HTML_HALIGN_LEFT;
		gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_align), halign - HTML_HALIGN_LEFT);

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_width), d->has_width);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_width),  d->width);
		gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_width), d->width_percent ? 1 : 0);

		gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_cols),  d->table->totalCols);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_rows),  d->table->totalRows);

		d->disable_change = FALSE;
	}
}

static void
set_insert_ui (GtkHTMLEditTableProperties *d)
{
}

static void
get_data (GtkHTMLEditTableProperties *d)
{
	d->table = html_engine_get_table (d->cd->html->engine);
	g_return_if_fail (d->table);

	if (HTML_OBJECT (d->table)->percent) {
		d->width = HTML_OBJECT (d->table)->percent;
		d->width_percent = TRUE;
		d->has_width = TRUE;
	} else if (d->table->specified_width) {
		d->width = d->table->specified_width;
		d->width_percent = FALSE;
		d->has_width = TRUE;
	} else
		d->has_width = FALSE;
}


GtkWidget *
table_properties (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkHTMLEditTableProperties *data = data_new (cd);
	GtkWidget *rv;

	get_data (data);
	*set_data = data;
	rv        = table_widget (data);
	set_ui (data);

	return rv;
}

GtkWidget *
table_insert (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkHTMLEditTableProperties *data = data_new (cd);
	GtkWidget *rv;

	*set_data = data;
	rv = table_insert_widget (data);
	set_insert_ui (data);
	gtk_html_edit_properties_dialog_change (data->cd->properties_dialog);

	return rv;
}

gboolean
table_apply_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditTableProperties *d = (GtkHTMLEditTableProperties *) get_data;
	HTMLEngine *e = d->cd->html->engine;
	gint position;

	position = e->cursor->position;

	if (html_engine_get_table (e) != d->table) {
		if (html_engine_goto_table_0 (e, d->table))
			html_cursor_forward (e->cursor, e);
		if (html_engine_get_table (e) != d->table) {
			GtkWidget *dialog;

			dialog = gtk_message_dialog_new (GTK_WINDOW (d->cd->properties_dialog->dialog),
							 GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
							 _("The editted table was removed from the document.\nCannot apply your changes."));
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
			html_cursor_jump_to_position (e->cursor, e, position);

			return FALSE;
		}
	}

	if (d->changed_width) {
		html_engine_table_set_width (d->cd->html->engine, d->table,
					     d->has_width ? d->width : 0, d->has_width ? d->width_percent : FALSE);
		d->changed_width = FALSE;
	}

	html_cursor_jump_to_position (e->cursor, e, position);

	return TRUE;
}

void
table_close_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	g_free (get_data);
}
