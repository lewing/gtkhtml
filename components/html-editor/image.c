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
#include <unistd.h>
#include <string.h>
#include <glade/glade.h>

#include "gtkhtml.h"
#include "htmlcolorset.h"
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlengine-edit-images.h"
#include "htmlengine-save.h"
#include "htmlimage.h"
#include "htmlsettings.h"

#include "image.h"
#include "properties.h"
#include "utils.h"

struct _GtkHTMLEditImageProperties {
	GtkHTMLControlData *cd;

	GtkWidget *page;

	HTMLImage *image;
	gboolean  insert;

	GtkWidget *pentry;
	gchar *location;

	GtkWidget *option_template;
	gint template;

	GtkWidget *spin_width;
	GtkWidget *option_width_percent;
	gint width;
	gint width_percent;


	GtkWidget *spin_height;
	GtkWidget *option_height_percent;
	gint height;
	gint height_percent;

	GtkWidget *spin_padh;
	gint padh;

	GtkWidget *spin_padv;
	gint padv;

	GtkWidget *spin_border;
	gint border;

	GtkWidget *option_align;
	HTMLVAlignType align;

	GtkWidget *entry_url;
	gchar *url;

	GtkWidget *entry_alt;
	gchar *alt;

	gboolean   disable_change;
};
typedef struct _GtkHTMLEditImageProperties GtkHTMLEditImageProperties;

#define TEMPLATES 3
typedef struct {
	gchar *name;
	gint offset;

	gboolean can_set_align;
	gboolean can_set_border;
	gboolean can_set_padding;
	gboolean can_set_size;

	HTMLVAlignType align;
	gint border;
	gint padh;
	gint padv;
	gint width;
	gboolean width_percent;
	gint height;
	gboolean height_percent;

	gchar *image;
} ImageInsertTemplate;

static ImageInsertTemplate image_templates [TEMPLATES] = {
	{
		N_("Plain"), 1,
		TRUE, TRUE, TRUE, TRUE, HTML_VALIGN_TOP, 0, 0, 0, 0, FALSE, 0, FALSE,
		N_("@link_begin@<img@alt@@width@@height@@align@ border=@border@@padh@@padv@@src@>@link_end@")
	},
	{
		N_("Frame"), 1,
		FALSE, TRUE, FALSE, TRUE, HTML_VALIGN_TOP, 1, 0, 0, 0, FALSE, 0, FALSE,
		N_("<center><table bgcolor=\"#c0c0c0\" cellspacing=\"0\" cellpadding=@border@>"
		   "<tr><td>"
		   "<table bgcolor=\"#f2f2f2\" cellspacing=\"0\" cellpadding=\"8\" width=\"100%\">"
		   "<tr><td align=\"center\">"
		   "<img @src@@alt@@width@@height@align=top border=0>"
		   "</td></tr></table></td></tr></table></center>")
	},
	{
		N_("Caption"), 1,
		FALSE, TRUE, FALSE, TRUE, HTML_VALIGN_TOP, 1, 0, 0, 0, FALSE, 0, FALSE,
		N_("<center><table bgcolor=\"#c0c0c0\" cellspacing=\"0\" cellpadding=@border@>"
		   "<tr><td>"
		   "<table bgcolor=\"#f2f2f2\" cellspacing=\"0\" cellpadding=\"8\" width=\"100%\">"
		   "<tr><td align=\"center\">"
		   "<img @src@@alt@@width@@height@align=top border=0>"
		   "</td></tr>"
		   "<tr><td><b>[Place your comment here]</td></tr>"
		   "</table></td></tr></table></center>")
	},
};

gboolean
ensure_image (GtkHTMLEditImageProperties *d)
{
	HTMLEngine *e = d->cd->html->engine;
	guint position = e->cursor->position;

	if (e->cursor->object != HTML_OBJECT (d->image))
		if (!html_cursor_jump_to (e->cursor, e, HTML_OBJECT (d->image), 1)) {
			GtkWidget *dialog;
			printf ("d: %p\n", d->cd->properties_dialog);
			dialog = gtk_message_dialog_new (GTK_WINDOW (d->cd->properties_dialog->dialog),
							 GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
							 _("The editted image was removed from the document.\nCannot apply your changes."));
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
			html_cursor_jump_to_position (e->cursor, e, position);
			return FALSE;
		}

	return TRUE;
}

static GtkHTMLEditImageProperties *
data_new (GtkHTMLControlData *cd)
{
	GtkHTMLEditImageProperties *data = g_new0 (GtkHTMLEditImageProperties, 1);

	/* fill data */
	data->cd             = cd;
	data->disable_change = TRUE;
	data->image          = NULL;

	/* default values */
	data->align          = HTML_VALIGN_TOP;
	data->width_percent  = 2;
	data->height_percent = 2;

	return data;
}

static gchar *
substitute_string (gchar *str, const gchar *var_name, const gchar *value)
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

static gchar *
get_location (GtkHTMLEditImageProperties *d)
{
	gchar *file;
	gchar *url;

	file = gnome_pixmap_entry_get_filename (GNOME_PIXMAP_ENTRY (d->pentry));
	if (file) {
		url = g_strconcat ("file://", file, NULL);
	} else {
		GtkWidget *entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (d->pentry));

		url = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));
	}

	if (!url)
		url = g_strdup ("");
	g_free (file);

	return url;
}

static void
pentry_changed (GtkWidget *entry, GtkHTMLEditImageProperties *d)
{
	const gchar *text;

	text = gtk_entry_get_text (GTK_ENTRY (entry));
	if (!text || !d->location || strcmp (text, d->location)) {
		g_free (d->location);
		d->location = g_strdup (text);
		if (!d->width_percent)
			d->width = 0;
		if (!d->height_percent)
			d->height = 0;
	}
}

static void
url_changed (GtkWidget *entry, GtkHTMLEditImageProperties *d)
{
	char *url, *target;

	url = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));
	target = NULL;

	if (url) {
		target = strchr (url, '#');
		if (target) {
			*target = 0;
			target ++;
		}
	}

	html_object_set_link (HTML_OBJECT (d->image),
			      url && *url
			      ? html_colorset_get_color (d->cd->html->engine->settings->color_set, HTMLLinkColor)
			      : html_colorset_get_color (d->cd->html->engine->settings->color_set, HTMLTextColor),
			      url, target);
	g_free (url);
}

static void
alt_changed (GtkWidget *entry, GtkHTMLEditImageProperties *d)
{
	if (ensure_image (d))
		html_image_set_alt (d->image, (char *) gtk_entry_get_text (GTK_ENTRY (entry)));
}

static void
changed_align (GtkWidget *w, GtkHTMLEditImageProperties *d)
{
	if (ensure_image (d))
		html_image_set_valign (d->image, g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w))));
}

static void
changed_size (GtkWidget *widget, GtkHTMLEditImageProperties *d)
{
	GtkWidget *menu_width_p, *menu_height_p;
	gint width, height, width_percent, height_percent;

	width = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_width));
	height = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_height));
	menu_width_p = gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_width_percent));
	menu_height_p = gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_height_percent));
	width_percent = g_list_index (GTK_MENU_SHELL (menu_width_p)->children,
				      gtk_menu_get_active (GTK_MENU (menu_width_p)));
	height_percent = g_list_index (GTK_MENU_SHELL (menu_height_p)->children,
				       gtk_menu_get_active (GTK_MENU (menu_height_p)));
	gtk_widget_set_sensitive (d->spin_width, width_percent != 2);
	gtk_widget_set_sensitive (d->spin_height, height_percent != 2);

	html_image_set_size (d->image,
			     width_percent == 2 ? 0 : width,
			     height_percent == 2 ? 0 : height,
			     width_percent == 1, height_percent == 1);
}

static void
test_url_clicked (GtkWidget *w, GtkHTMLEditImageProperties *d)
{
	const char *url = gtk_entry_get_text (GTK_ENTRY (d->entry_url));

	if (url)
		gnome_url_show (url, NULL);
}

static void
fill_templates (GtkHTMLEditImageProperties *d)
{
	GtkWidget *menu;
	gint i;

	menu = gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_template));

	for (i = 0; i < TEMPLATES; i ++)
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_menu_item_new_with_label (_(image_templates [i].name)));
	gtk_menu_set_active (GTK_MENU (menu), 0);
	gtk_container_remove (GTK_CONTAINER (menu), gtk_menu_get_active (GTK_MENU (menu)));
}

static void
set_ui (GtkHTMLEditImageProperties *d)
{
	d->disable_change = TRUE;

	/* gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_template), d->template); */
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_align), d->align);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_width_percent), d->width_percent);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_height_percent), d->height_percent);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_width), d->width);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_height), d->height);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_padh), d->padh);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_padv), d->padv);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_border), d->border);
	gtk_entry_set_text (GTK_ENTRY (d->entry_url), d->url ? d->url : "");
	gtk_entry_set_text (GTK_ENTRY (d->entry_alt), d->alt ? d->alt : "");
	gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (d->pentry))),
			    d->location ? d->location : "");

	gtk_widget_set_sensitive (d->spin_width, d->width_percent != 2);
	gtk_widget_set_sensitive (d->spin_height, d->height_percent != 2);

	d->disable_change = FALSE;
}

static void
changed_template (GtkWidget *w, GtkHTMLEditImageProperties *d)
{
	gint oldtemplate = d->template;
	d->template = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w)));

	if (d->template == oldtemplate)
		return;

	d->border = image_templates [d->template].border;
	d->align = image_templates [d->template].align;
	d->padh = image_templates [d->template].padh;
	d->padv = image_templates [d->template].padv;
	d->padh = image_templates [d->template].padh;
	d->width = image_templates [d->template].width;
	d->width_percent = image_templates [d->template].width_percent;
	d->height = image_templates [d->template].height;
	d->height_percent = image_templates [d->template].height_percent;

	gtk_widget_set_sensitive (d->spin_width, image_templates [d->template].can_set_size);
	gtk_widget_set_sensitive (d->option_width_percent, image_templates [d->template].can_set_size);
	gtk_widget_set_sensitive (d->spin_height, image_templates [d->template].can_set_size);
	gtk_widget_set_sensitive (d->option_height_percent, image_templates [d->template].can_set_size);

	gtk_widget_set_sensitive (d->spin_padh, image_templates [d->template].can_set_padding);
	gtk_widget_set_sensitive (d->spin_padv, image_templates [d->template].can_set_padding);

	gtk_widget_set_sensitive (d->spin_border, image_templates [d->template].can_set_border);

	gtk_widget_set_sensitive (d->option_align, image_templates [d->template].can_set_align);

	set_ui (d);
}

static void
changed_border (GtkWidget *check, GtkHTMLEditImageProperties *d)
{
	if (ensure_image (d))
		html_image_set_border (d->image, gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_border)));
}

static void
changed_padding (GtkWidget *check, GtkHTMLEditImageProperties *d)
{
	if (ensure_image (d))
		html_image_set_spacing  (d->image,
					 gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_padh)),
					 gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_padv)));
}

static void
set_size_all (HTMLObject *o, HTMLEngine *e, GtkHTMLEditImageProperties *d)
{
	if (d->location && HTML_IS_IMAGE (o) && HTML_IMAGE (o)->image_ptr && HTML_IMAGE (o)->image_ptr->url) {
		gchar *location = get_location (d);

		if (!strcmp (HTML_IMAGE (o)->image_ptr->url, location)) {
			HTMLImage *i = HTML_IMAGE (o);

			d->disable_change = TRUE;
			if ((d->width == 0 || d->width_percent == 2) && d->width_percent != 1) {
				d->width = html_image_get_actual_width (i, NULL);
				gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_width), d->width);
			}
			if ((d->height == 0 || d->height_percent == 2) && d->height_percent != 1) {
				d->height = html_image_get_actual_height (i, NULL);
				gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_height), d->height);
			}
			d->disable_change = FALSE;
		}
		g_free (location);
	}
}

#define UPPER_FIX(x) gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (d->spin_ ## x))->upper = 100000.0

static GtkWidget *
image_widget (GtkHTMLEditImageProperties *d, gboolean insert)
{
	GladeXML *xml;
	GtkWidget *frame_template, *button;

	xml = glade_xml_new (GLADE_DATADIR "/gtkhtml-editor-properties.glade", "image_page", GETTEXT_PACKAGE);
	if (!xml)
		g_error (_("Could not load glade file."));

	d->page = glade_xml_get_widget (xml, "image_page");
	frame_template = glade_xml_get_widget (xml, "frame_image_template");

	d->option_align = glade_xml_get_widget (xml, "option_image_align");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_align)),
			  "selection-done", G_CALLBACK (changed_align), d);
	d->option_width_percent = glade_xml_get_widget (xml, "option_image_width_percent");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_width_percent)),
			  "selection-done", G_CALLBACK (changed_size), d);
	d->option_height_percent = glade_xml_get_widget (xml, "option_image_height_percent");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_height_percent)),
			  "selection-done", G_CALLBACK (changed_size), d);

	d->spin_border = glade_xml_get_widget (xml, "spin_image_border");
	UPPER_FIX (border);
	g_signal_connect (d->spin_border, "value_changed", G_CALLBACK (changed_border), d);
	d->spin_width = glade_xml_get_widget (xml, "spin_image_width");
	UPPER_FIX (width);
	g_signal_connect (d->spin_width, "value_changed", G_CALLBACK (changed_size), d);
	d->spin_height = glade_xml_get_widget (xml, "spin_image_height");
	UPPER_FIX (height);
	g_signal_connect (d->spin_height, "value_changed", G_CALLBACK (changed_size), d);
	d->spin_padh = glade_xml_get_widget (xml, "spin_image_padh");
	UPPER_FIX (padh);
	g_signal_connect (d->spin_padh, "value_changed", G_CALLBACK (changed_padding), d);
	d->spin_padv = glade_xml_get_widget (xml, "spin_image_padv");
	UPPER_FIX (padv);
	g_signal_connect (d->spin_padv, "value_changed", G_CALLBACK (changed_padding), d);

	/* d->option_template = glade_xml_get_widget (xml, "option_image_template");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_template)),
			  "selection-done", G_CALLBACK (changed_template), d);
	if (insert)
	   fill_templates (d); */

	d->entry_url = glade_xml_get_widget (xml, "entry_image_url");
	g_signal_connect (GTK_OBJECT (d->entry_url), "changed", G_CALLBACK (url_changed), d);

	d->entry_alt = glade_xml_get_widget (xml, "entry_image_alt");
	g_signal_connect (d->entry_alt, "changed", G_CALLBACK (alt_changed), d);

	d->pentry = glade_xml_get_widget (xml, "pentry_image_location");
	gnome_pixmap_entry_set_pixmap_subdir (GNOME_PIXMAP_ENTRY (d->pentry), g_get_home_dir ());
	g_signal_connect (GTK_OBJECT (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (d->pentry))),
			    "changed", G_CALLBACK (pentry_changed), d);

	gtk_widget_show_all (d->page);
	gnome_pixmap_entry_set_preview (GNOME_PIXMAP_ENTRY (d->pentry), FALSE);

	editor_check_stock ();
	button = gtk_button_new_from_stock (GTKHTML_STOCK_TEST_URL);
	g_signal_connect (button, "clicked", G_CALLBACK (test_url_clicked), d);
	gtk_widget_show (button);
	gtk_table_attach (GTK_TABLE (glade_xml_get_widget (xml, "image_table")), button, 2, 3, 0, 1, 0, 0, 0, 0);

	return d->page;
}

GtkWidget *
image_insertion (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkWidget *w;
	GtkHTMLEditImageProperties *d;

	*set_data = d = data_new (cd);
	w = image_widget (d, TRUE);

	set_ui (d);

	gtk_widget_show (w);

	return w;
}

static void
get_data (GtkHTMLEditImageProperties *d, HTMLImage *image)
{
	HTMLImagePointer *ip = image->image_ptr;
	gint off = 0;

	d->image = image;
	if (!HTML_OBJECT (image)->parent || !html_object_get_data (HTML_OBJECT (image)->parent, "template_image")) {
		if (!strncmp (ip->url, "file://", 7))
			off = 7;
		else if (!strncmp (ip->url, "file:", 5))
			off = 5;
		d->location = g_strdup (ip->url + off);
	}

	if (image->percent_width) {
		d->width_percent = 1;
		d->width = image->specified_width;
	} else if (image->specified_width > 0) {
		d->width_percent = 0;
		d->width = image->specified_width;
	} else
		d->width_percent = 2;
	if (image->percent_height) {
		d->height_percent = 1;
		d->height = image->specified_height;
	} else if (image->specified_height > 0) {
		d->height_percent = 0;
		d->height = image->specified_height;
	} else
		d->height_percent = 2;

	if ((d->width == 0 || d->width_percent == 2) && d->width_percent != 1)
		d->width = html_image_get_actual_width (image, NULL);

	if ((d->height == 0 || d->height_percent == 2) && d->height_percent != 1)
		d->height = html_image_get_actual_height (image, NULL);

	d->align  = image->valign;
	d->padh   = image->hspace;
	d->padv   = image->vspace;
	d->border = image->border;
	d->url    = image->url ? g_strconcat (image->url, image->target ? "#" : "", image->target, NULL) : g_strdup ("");
	d->alt    = g_strdup (image->alt);
}

GtkWidget *
image_properties (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkWidget *w;
	GtkHTMLEditImageProperties *d;
	HTMLImage *image = HTML_IMAGE (cd->html->engine->cursor->object);

	g_assert (HTML_OBJECT_TYPE (cd->html->engine->cursor->object) == HTML_TYPE_IMAGE);

	*set_data = d = data_new (cd);
	w = image_widget (d, FALSE);

	get_data (d, image);
	set_ui (d);
	gtk_widget_show (w);

	return w;
}

static gboolean
insert_or_apply (GtkHTMLControlData *cd, gpointer get_data, gboolean insert)
{	
	GtkHTMLEditImageProperties *d = (GtkHTMLEditImageProperties *) get_data;
	HTMLImage *image = HTML_IMAGE (d->image);
	HTMLEngine *e = d->cd->html->engine;
	gchar *location, *url, *target;
	gint position;

	position = e->cursor->position;

	g_assert (HTML_OBJECT_TYPE (d->image) == HTML_TYPE_IMAGE);

	if (!ensure_image (d))
		return FALSE;

	if (HTML_OBJECT (image)->parent && html_object_get_data (HTML_OBJECT (image)->parent, "template_image"))
		html_object_set_data_full (HTML_OBJECT (image)->parent, "template_image", NULL, NULL);

	location = get_location (d);
	html_image_edit_set_url (image, location);
	g_free (location);

	html_cursor_jump_to_position (e->cursor, e, position);

	return TRUE;
}

gboolean
image_apply_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	return insert_or_apply (cd, get_data, FALSE);
}

gboolean
image_insert_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	insert_or_apply (cd, get_data, TRUE);
	return TRUE;
}

void
image_close_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditImageProperties *d = (GtkHTMLEditImageProperties *) get_data;

	g_free (d->url);
	g_free (d->alt);
	g_free (d->location);
	g_free (d);
}
