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

#include <string.h>
#include "config.h"
#include "properties.h"
#include "dialog.h"
#include "link.h"
#include "htmlengine-edit-insert.h"

static void
set_entries (HTMLEngine *e, GtkWidget *el)
{
	gchar *text;
	gchar *found;

	text = html_engine_get_selection_string (e);

	/* use only text part to \n */
	found = strchr (text, '\n');
	if (found) *found=0;

	/* if it contains mailto: and '@'  we assume it is href */
	if (text == (found = strstr (text, "mailto:")) && strchr (text, '@') > found) {
		gtk_entry_set_text (GTK_ENTRY (el), text);
		goto end;
	}

	/* if it contains http: or ftp: we assume it's href */
	if (text == strstr (text, "http:") || text == strstr (text, "ftp:")) {
		gtk_entry_set_text (GTK_ENTRY (el), text);
		goto end;
	}

	/* mailto addition */
	if (strchr (text, '@')) {
		gchar *link;

		link = g_strconcat ("mailto:", text, NULL);
		gtk_entry_set_text (GTK_ENTRY (el), link);
		g_free (link);
		goto end;
	}

 end:
	g_free (text);
	return;
}

struct _GtkHTMLEditLinkProperties {
	GtkHTMLControlData *cd;
	GtkWidget *entry;
	gboolean url_changed;
};
typedef struct _GtkHTMLEditLinkProperties GtkHTMLEditLinkProperties;

static void
set_link (GtkWidget *w, GtkHTMLEditLinkProperties *data)
{
	data->url_changed = TRUE;
	gtk_html_edit_properties_dialog_change (data->cd->properties_dialog);
}

GtkWidget *
link_properties (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkWidget *vbox, *hbox;
	GtkHTMLEditLinkProperties *data = g_new (GtkHTMLEditLinkProperties, 1);

	*set_data = data;
	data->cd = cd;
	data->url_changed = FALSE;

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_border_width (GTK_CONTAINER (vbox), 3);
	hbox = gtk_hbox_new (FALSE, 3);

	data->entry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (data->entry), (cd->html->engine->active_selection)
			    ? html_engine_get_document_url (cd->html->engine)
			    : html_engine_get_url (cd->html->engine));
	gtk_signal_connect (GTK_OBJECT (data->entry), "changed", set_link, data);
	gtk_box_pack_start (GTK_BOX (hbox), gtk_label_new (_("URL")), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), data->entry, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	return vbox;
}

void
link_apply_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditLinkProperties *data = (GtkHTMLEditLinkProperties *) get_data;
	HTMLEngine *e = cd->html->engine;
	gchar *url;
	gchar *target = "";

	if (!data->url_changed)
		return;

	url = gtk_entry_get_text (GTK_ENTRY (data->entry));
	if (*url)
		html_engine_insert_link (e, url, target);
	else
		html_engine_remove_link (e);
}

void
link_close_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	g_free (get_data);
}
