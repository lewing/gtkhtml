/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
	      (C) 1999 Anders Carlson (andersca@gnu.org)

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
#include "htmlfont.h"

/* FIXME: Fix a better one */
gint defaultFontSizes [7] = {8, 10, 12, 14, 18, 24, 24};

static GdkFont *create_gdk_font (gchar *family, gint size, gboolean bold, gboolean italic);

HTMLFont *
html_font_new (gchar *family, gint size, gint *fontSizes, gboolean bold, gboolean italic, gboolean underline)
{
	gchar *xlfd;
	HTMLFont *f;

	g_return_val_if_fail (family != NULL, NULL);

	f = g_new0 (HTMLFont, 1);
	f->family = g_strdup (family);
	f->size = size;
	f->bold = bold;
	f->italic = italic;
	f->underline = underline;
	f->pointSize = fontSizes [size];

	f->gdk_font = create_gdk_font (family, fontSizes[size], bold, italic);

	f->textColor = NULL;

	return f;
}

static GdkFont *
create_gdk_font (gchar *family, gint size, gboolean bold, gboolean italic)
{
	gboolean loaded = FALSE;
	gchar *boldstr;
	gchar *italicstr;
	gchar *fontname;
	gint realsize;
	GdkFont *font;

	/* FIXME: a better way to find out the size */
	realsize = size - 2;

	if (bold)
		boldstr = "bold";
	else
		boldstr = "medium";

	if (italic)
		italicstr = "i";
	else
		italicstr = "r";
	
	fontname = g_strdup_printf ("-*-%s-%s-%s-normal-*-%d-*-*-*-*-*-*-*",
				    family, boldstr, italicstr, realsize);

	g_print ("trying: %s\n", fontname);
	font = gdk_font_load (fontname);
	if (font)
		return font;
	else {
		g_free (fontname);
		g_warning ("font not found, using helvetica");
		fontname = g_strdup_printf ("-*-helvetica-medium-r-normal-*-*-%d-*-*-*-*-*-*",
					    realsize);
		font = gdk_font_load (fontname);
	}
	return font;
}

void
html_font_destroy (HTMLFont *html_font)
{
	g_return_if_fail (html_font != NULL);

	gdk_font_unref (html_font->gdk_font);
	g_free (html_font->family);

	if (html_font->textColor)
		gdk_color_free (html_font->textColor);

	g_free (html_font);
}

void
html_font_set_color (HTMLFont *html_font,
		     const GdkColor *color)
{
	if (html_font->textColor != NULL)
		gdk_color_free (html_font->textColor);

	/* Evil but safe cast: the prototype for `GdkColor' is wrong.  */
	html_font->textColor = gdk_color_copy ((GdkColor *) color);
}


gint
html_font_calc_ascent (HTMLFont *f)
{
	g_return_val_if_fail (f != NULL, 0);
	
	return f->gdk_font->ascent;
}

gint
html_font_calc_descent (HTMLFont *f)
{
	g_return_val_if_fail (f != NULL, 0);
	
	return f->gdk_font->descent;
}

gint
html_font_calc_width (HTMLFont *f, gchar *text, gint len)
{
	gint width;

	g_return_val_if_fail (f != NULL, 0);
	g_return_val_if_fail (text != NULL, 0);
	
	if (len == -1)
		width = gdk_string_width (f->gdk_font, text);
	else
		width = gdk_text_width (f->gdk_font, text, len);

	return width;
}
