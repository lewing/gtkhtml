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

#include "htmlclue.h"
#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-table.h"
#include "htmlengine-edit-tablecell.h"
#include "htmlengine-save.h"
#include "htmlimage.h"
#include "htmltable.h"
#include "htmltablecell.h"
#include "htmlsettings.h"

#include "properties.h"
#include "cell.h"
#include "utils.h"

typedef enum
{
	CELL_SCOPE_CELL,
	CELL_SCOPE_ROW,
	CELL_SCOPE_COLUMN,
	CELL_SCOPE_TABLE
} CellScope;

typedef struct
{	
	GtkHTMLControlData *cd;
	HTMLTableCell *cell;
	HTMLTable *table;
	CellScope  scope;

	GtkWidget *combo_bg_color;
	GtkWidget *entry_bg_pixmap;

	gboolean        changed_halign;
	HTMLHAlignType  halign;
	GtkWidget      *option_halign;

	gboolean        changed_valign;
	HTMLVAlignType  valign;
	GtkWidget      *option_valign;

	gboolean   has_width;
	gboolean   changed_width;
	gint       width;
	gboolean   width_percent;
	GtkWidget *spin_width;
	GtkWidget *check_width;
	GtkWidget *option_width;

	gboolean   has_height;
	gboolean   changed_height;
	gint       height;
	gboolean   height_percent;
	GtkWidget *spin_height;
	GtkWidget *check_height;
	GtkWidget *option_height;

	gint cspan;
	gint rspan;
	GtkWidget *spin_cspan;
	GtkWidget *spin_rspan;

	GtkWidget *check_wrap;
	GtkWidget *check_header;

	gboolean   disable_change;

} GtkHTMLEditCellProperties;

static GtkHTMLEditCellProperties *
data_new (GtkHTMLControlData *cd)
{
	GtkHTMLEditCellProperties *data = g_new0 (GtkHTMLEditCellProperties, 1);

	/* fill data */
	data->cd = cd;
	data->scope = CELL_SCOPE_CELL;

	return data;
}

static void
cell_set_prop (GtkHTMLEditCellProperties *d, void (*set_fn)(HTMLTableCell *, GtkHTMLEditCellProperties *))
{
	HTMLEngine *e = d->cd->html->engine;
	guint position;
	if (d->disable_change || !editor_has_html_object (d->cd, HTML_OBJECT (d->table)))
		return;

	position = d->cd->html->engine->cursor->position;
	switch (d->scope) {
	case CELL_SCOPE_CELL:
		(*set_fn) (d->cell, d);
		break;
	case CELL_SCOPE_ROW:
		if (html_engine_table_goto_row (e, d->table, d->cell->row)) {
			HTMLTableCell *cell = html_engine_get_table_cell (e);

			while (cell && cell->row == d->cell->row) {
				if (HTML_OBJECT (cell)->parent == HTML_OBJECT (d->table))
					(*set_fn) (cell, d);
				html_engine_next_cell (e, FALSE);
				cell = html_engine_get_table_cell (e);
			}
		}
		break;
	case CELL_SCOPE_COLUMN:
		if (html_engine_table_goto_col (e, d->table, d->cell->col)) {
			HTMLTableCell *cell = html_engine_get_table_cell (e);

			while (cell) {
				if (cell->col == d->cell->col && HTML_OBJECT (cell)->parent == HTML_OBJECT (d->table))
					(*set_fn) (cell, d);
				html_engine_next_cell (e, FALSE);
				cell = html_engine_get_table_cell (e);
			}
		}
		break;
	case CELL_SCOPE_TABLE:
		if (html_engine_goto_table_0 (e, d->table)) {
			HTMLTableCell *cell;

			html_cursor_forward (e->cursor, e);
			cell = html_engine_get_table_cell (e);
			while (cell) {
				if (HTML_OBJECT (cell)->parent == HTML_OBJECT (d->table))
					(*set_fn) (cell, d);
				html_engine_next_cell (e, FALSE);
				cell = html_engine_get_table_cell (e);
			}
		}
		break;
	}

	html_cursor_jump_to_position (e->cursor, e, position);
}

static void
set_bg_color (HTMLTableCell *cell, GtkHTMLEditCellProperties *d)
{
	html_engine_table_cell_set_bg_color (d->cd->html->engine, cell, color_combo_get_color (COLOR_COMBO (d->combo_bg_color), NULL));
}

static void
changed_bg_color (GtkWidget *w, GdkColor *color, gboolean custom, gboolean by_user, gboolean is_default, GtkHTMLEditCellProperties *d)
{
	cell_set_prop (d, set_bg_color);
}

static void
set_bg_pixmap (HTMLTableCell *cell, GtkHTMLEditCellProperties *d)
{
	const char *file;
	char *url = NULL;

	file = gtk_entry_get_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (d->entry_bg_pixmap))));
	if (file && *file)
		url = g_strconcat ("file://", file, NULL);

	html_engine_table_cell_set_bg_pixmap (d->cd->html->engine, cell, url);
	g_free (url);
}

static void
changed_bg_pixmap (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	cell_set_prop (d, set_bg_pixmap);
}

static void
changed_halign (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->halign = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w))) + HTML_HALIGN_LEFT;
	if (!d->disable_change)
		d->changed_halign = TRUE;
}

static void
changed_valign (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->valign = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w))) + HTML_VALIGN_TOP;
	if (!d->disable_change)
		d->changed_valign = TRUE;
}

static void
changed_cspan (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->cspan = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_cspan));
}

static void
changed_rspan (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->rspan = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_rspan));
}

static void
changed_width (GtkWidget *w, GtkHTMLEditCellProperties *d)
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
set_has_width (GtkWidget *check, GtkHTMLEditCellProperties *d)
{
	d->has_width = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (d->check_width));
	if (!d->disable_change)
		d->changed_width = TRUE;
}

static void
changed_width_percent (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->width_percent = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w))) ? TRUE : FALSE;
	if (!d->disable_change)
		d->changed_width = TRUE;
}

static void
changed_height (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->height = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_height));
	if (!d->disable_change) {
		d->disable_change = TRUE;
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_height), TRUE);
		d->disable_change = FALSE;
		d->changed_height = TRUE;
	}
}

static void
set_has_height (GtkWidget *check, GtkHTMLEditCellProperties *d)
{
	d->has_height = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (d->check_height));
	if (!d->disable_change)
		d->changed_height = TRUE;
}

static void
changed_height_percent (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->height_percent = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w))) ? TRUE : FALSE;
	if (!d->disable_change)
		d->changed_height = TRUE;
}

static void
set_wrap (HTMLTableCell *cell, GtkHTMLEditCellProperties *d)
{
	html_engine_table_cell_set_no_wrap (d->cd->html->engine, cell, !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (d->check_wrap)));
}

static void
changed_wrap (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	cell_set_prop (d, set_wrap);
}

static void
set_header (HTMLTableCell *cell, GtkHTMLEditCellProperties *d)
{
	html_engine_table_cell_set_heading (d->cd->html->engine, cell, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (d->check_header)));
}

static void
changed_heading (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	cell_set_prop (d, set_header);
}

/*
 * FIX: set spin adjustment upper to 100000
 *      as glade cannot set it now
 */
#define UPPER_FIX(x) gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (d->spin_ ## x))->upper = 100000.0

static void
cell_scope_cell (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w)))
		d->scope = CELL_SCOPE_CELL;
}

static void
cell_scope_table (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w)))
		d->scope = CELL_SCOPE_TABLE;
}

static void
cell_scope_row (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w)))
		d->scope = CELL_SCOPE_ROW;
}

static void
cell_scope_column (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w)))
		d->scope = CELL_SCOPE_COLUMN;
}

static GtkWidget *
cell_widget (GtkHTMLEditCellProperties *d)
{
	GtkWidget *cell_page;
	GladeXML *xml;

	xml = glade_xml_new (GLADE_DATADIR "/gtkhtml-editor-properties.glade", "cell_page", GETTEXT_PACKAGE);
	if (!xml)
		g_error (_("Could not load glade file."));

	cell_page          = glade_xml_get_widget (xml, "cell_page");

	gtk_table_attach (GTK_TABLE (glade_xml_get_widget (xml, "cell_scope_table1")),
			  gtk_image_new_from_file (ICONDIR "/table-cell-16.png"),
			  0, 1, 0, 1, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (glade_xml_get_widget (xml, "cell_scope_table1")),
			  gtk_image_new_from_file (ICONDIR "/table-table-16.png"),
			  0, 1, 1, 2, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (glade_xml_get_widget (xml, "cell_scope_table2")),
			  gtk_image_new_from_file (ICONDIR "/table-row-16.png"),
			  0, 1, 0, 1, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (glade_xml_get_widget (xml, "cell_scope_table2")),
			  gtk_image_new_from_file (ICONDIR "/table-column-16.png"),
			  0, 1, 1, 2, 0, 0, 0, 0);

	d->combo_bg_color = color_combo_new (NULL, _("Transparent"), NULL,
					     color_group_fetch ("cell_bg_color", d->cd));
        color_combo_box_set_preview_relief (COLOR_COMBO (d->combo_bg_color), GTK_RELIEF_NORMAL); \
        g_signal_connect (d->combo_bg_color, "color_changed", G_CALLBACK (changed_bg_color), d);
	gtk_box_pack_start (GTK_BOX (glade_xml_get_widget (xml, "bg_color_hbox")), d->combo_bg_color, FALSE, FALSE, 0);

	d->entry_bg_pixmap = glade_xml_get_widget (xml, "entry_cell_bg_pixmap");
	g_signal_connect (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (d->entry_bg_pixmap)),
			    "changed", G_CALLBACK (changed_bg_pixmap), d);

	d->option_halign = glade_xml_get_widget (xml, "option_cell_halign");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_halign)), "selection-done",
			  G_CALLBACK (changed_halign), d);
	d->option_valign = glade_xml_get_widget (xml, "option_cell_valign");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_valign)), "selection-done",
			  G_CALLBACK (changed_valign), d);

	d->spin_width   = glade_xml_get_widget (xml, "spin_cell_width");
	UPPER_FIX (width);
	g_signal_connect (d->spin_width, "value_changed", G_CALLBACK (changed_width), d);
	d->check_width  = glade_xml_get_widget (xml, "check_cell_width");
	g_signal_connect (d->check_width, "toggled", G_CALLBACK (set_has_width), d);
	d->option_width = glade_xml_get_widget (xml, "option_cell_width");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_width)), "selection-done",
			  G_CALLBACK (changed_width_percent), d);

	d->spin_height   = glade_xml_get_widget (xml, "spin_cell_height");
	UPPER_FIX (height);
	g_signal_connect (d->spin_height, "value_changed", G_CALLBACK (changed_height), d);
	d->check_height  = glade_xml_get_widget (xml, "check_cell_height");
	g_signal_connect (d->check_height, "toggled", G_CALLBACK (set_has_height), d);
	d->option_height = glade_xml_get_widget (xml, "option_cell_height");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_height)), "selection-done",
			  G_CALLBACK (changed_height_percent), d);

	d->check_wrap = glade_xml_get_widget (xml, "check_cell_wrap");
	d->check_header = glade_xml_get_widget (xml, "check_cell_header");
	g_signal_connect (d->check_wrap, "toggled", G_CALLBACK (changed_wrap), d);
	g_signal_connect (d->check_header, "toggled", G_CALLBACK (changed_heading), d);

        g_signal_connect (glade_xml_get_widget (xml, "cell_radio"), "toggled", G_CALLBACK (cell_scope_cell), d);
        g_signal_connect (glade_xml_get_widget (xml, "table_radio"), "toggled", G_CALLBACK (cell_scope_table), d);
        g_signal_connect (glade_xml_get_widget (xml, "row_radio"), "toggled", G_CALLBACK (cell_scope_row), d);
        g_signal_connect (glade_xml_get_widget (xml, "col_radio"), "toggled", G_CALLBACK (cell_scope_column), d);

	d->spin_cspan   = glade_xml_get_widget (xml, "spin_cell_cspan");
	d->spin_rspan   = glade_xml_get_widget (xml, "spin_cell_rspan");
	g_signal_connect (d->spin_cspan, "value_changed", G_CALLBACK (changed_cspan), d);
	g_signal_connect (d->spin_rspan, "value_changed", G_CALLBACK (changed_rspan), d);

	gtk_widget_show_all (cell_page);
	gnome_pixmap_entry_set_preview (GNOME_PIXMAP_ENTRY (d->entry_bg_pixmap), FALSE);

	return cell_page;
}

static void
set_ui (GtkHTMLEditCellProperties *d)
{
	if (!editor_has_html_object (d->cd, HTML_OBJECT (d->table)))
		return;

	d->disable_change = TRUE;

	if (d->cell->have_bg)
		color_combo_set_color (COLOR_COMBO (d->combo_bg_color), &d->cell->bg);

	if (d->cell->have_bgPixmap) {
		int off = 0;

		if (!strncasecmp ("file://", d->cell->bgPixmap->url, 7))
			off = 7;
		else if (!strncasecmp ("file:", d->cell->bgPixmap->url, 5))
			off = 5;
		gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (d->entry_bg_pixmap))),
				    d->cell->bgPixmap->url + off);
	}

	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_halign), d->halign - HTML_HALIGN_LEFT);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_valign), d->valign - HTML_VALIGN_TOP);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_width), d->has_width);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_width),  d->width);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_width), d->width_percent ? 1 : 0);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_height), d->has_height);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_height),  d->height);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_height), d->height_percent ? 1 : 0);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_wrap), !d->cell->no_wrap);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_header), d->cell->heading);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_cspan),  d->cspan);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_rspan),  d->rspan);

	d->disable_change = FALSE;
}

static void
get_data (GtkHTMLEditCellProperties *d)
{
	d->cell = html_engine_get_table_cell (d->cd->html->engine);
	g_return_if_fail (d->cell);
	d->table = HTML_TABLE (HTML_OBJECT (d->cell)->parent);
	g_return_if_fail (d->table && HTML_IS_TABLE (d->table));

	d->halign   = HTML_CLUE (d->cell)->halign;
	d->valign   = HTML_CLUE (d->cell)->valign;

	if (d->cell->percent_width) {
		d->width = d->cell->fixed_width;
		d->width_percent = TRUE;
		d->has_width = TRUE;
	} else if (d->cell->fixed_width) {
		d->width = d->cell->fixed_width;
		d->width_percent = FALSE;
		d->has_width = TRUE;
	}

	d->cspan = d->cell->cspan;
	d->rspan = d->cell->rspan;
}

GtkWidget *
cell_properties (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkHTMLEditCellProperties *data = data_new (cd);
	GtkWidget *rv;

	get_data (data);
	*set_data = data;
	rv        = cell_widget (data);
	set_ui (data);

	return rv;
}

static void
cell_apply_1 (HTMLTableCell *cell, GtkHTMLEditCellProperties *d)
{
	if (d->changed_halign)
		html_engine_table_cell_set_halign (d->cd->html->engine, cell, d->halign);

	if (d->changed_valign)
		html_engine_table_cell_set_valign (d->cd->html->engine, cell, d->valign);

	if (d->changed_width)
		html_engine_table_cell_set_width (d->cd->html->engine, cell,
						  d->has_width ? d->width : 0, d->has_width ? d->width_percent : FALSE);
	html_engine_set_cspan (d->cd->html->engine, d->cspan);
	html_engine_set_rspan (d->cd->html->engine, d->rspan);
}

static void
cell_apply_row (GtkHTMLEditCellProperties *d)
{
	HTMLTableCell *cell;
	HTMLEngine *e = d->cd->html->engine;

	if (html_engine_table_goto_row (e, HTML_TABLE (HTML_OBJECT (d->cell)->parent), d->cell->row)) {
		cell = html_engine_get_table_cell (e);

		while (cell && cell->row == d->cell->row) {
			if (HTML_OBJECT (cell)->parent == HTML_OBJECT (d->cell)->parent)
				cell_apply_1 (cell, d);
			html_engine_next_cell (e, FALSE);
			cell = html_engine_get_table_cell (e);
		}
	}
}

static void
cell_apply_col (GtkHTMLEditCellProperties *d)
{
	HTMLTableCell *cell;
	HTMLEngine *e = d->cd->html->engine;

	if (html_engine_table_goto_col (e, HTML_TABLE (HTML_OBJECT (d->cell)->parent), d->cell->col)) {
		cell = html_engine_get_table_cell (e);

		while (cell) {
			if (cell->col == d->cell->col && HTML_OBJECT (cell)->parent == HTML_OBJECT (d->cell)->parent)
				cell_apply_1 (cell, d);
			html_engine_next_cell (e, FALSE);
			cell = html_engine_get_table_cell (e);
		}
	}
}

static void
cell_apply_table (GtkHTMLEditCellProperties *d)
{
	HTMLTable *table;
	HTMLTableCell *cell;
	HTMLEngine *e = d->cd->html->engine;

	table = html_engine_get_table (e);
	if (table && html_engine_goto_table_0 (e, table)) {
		cell = html_engine_get_table_cell (e);

		while (cell) {
			if (HTML_OBJECT (cell)->parent == HTML_OBJECT (d->cell)->parent)
				cell_apply_1 (cell, d);
			html_engine_next_cell (e, FALSE);
			cell = html_engine_get_table_cell (e);
		}
	}
}

gboolean
cell_apply_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditCellProperties *d = (GtkHTMLEditCellProperties *) get_data;
	HTMLEngine *e = d->cd->html->engine;
	gint position;

	position = e->cursor->position;

	if (html_engine_get_table_cell (e) != d->cell) {
		if (!html_engine_goto_table (e, HTML_TABLE (HTML_OBJECT (d->cell)->parent), d->cell->row, d->cell->col)) {
			GtkWidget *dialog;

			dialog = gtk_message_dialog_new (GTK_WINDOW (d->cd->properties_dialog->dialog),
							 GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
							 _("The editted cell was removed from the document.\nCannot apply your changes."));
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
			html_cursor_jump_to_position (e->cursor, e, position);

			return FALSE;
		}
	}

	html_cursor_jump_to_position (e->cursor, e, position);

	return TRUE;
}

void
cell_close_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	g_free (get_data);
}
