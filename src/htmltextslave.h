/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

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
#ifndef _HTMLTEXTSLAVE_H_
#define _HTMLTEXTSLAVE_H_

#include "htmlobject.h"
#include "htmltext.h"

#define HTML_TEXT_SLAVE(x) ((HTMLTextSlave *) (x))
#define HTML_TEXT_SLAVE_CLASS(x) ((HTMLTextSlaveClass *) (x))
#define HTML_IS_TEXT_SLAVE(x) (HTML_CHECK_TYPE ((x), HTML_TYPE_TEXTSLAVE))

struct _HTMLTextSlave {
	HTMLObject object;

	HTMLText *owner;
	guint posStart;
	guint posLen;
	gchar *charStart;

	HTMLTextPangoInfo *pi;
	GList *glyphs; /* deprecated */
	GSList *glyph_items;
};

struct _HTMLTextSlaveClass {
	HTMLObjectClass object_class;
};

struct _HTMLTextSlaveGlyphItem {
	PangoGlyphItem glyph_item;
	PangoGlyphUnit *widths;

	HTMLTextSlaveGlyphItemType type;
};


void        html_text_slave_type_init             (void);
void        html_text_slave_class_init            (HTMLTextSlaveClass *klass,
						   HTMLType            type,
						   guint               object_size);
void        html_text_slave_init                  (HTMLTextSlave      *slave,
						   HTMLTextSlaveClass *klass,
						   HTMLText           *owner,
						   guint               posStart,
						   guint               posLen);
HTMLObject *html_text_slave_new                   (HTMLText           *owner,
						   guint               posStart,
						   guint               posLen);
gint        html_text_slave_get_line_offset       (HTMLTextSlave      *slave,
						   gint                offset,
						   HTMLPainter        *p);
char       *html_text_slave_get_text              (HTMLTextSlave      *slave);
gint        html_text_slave_nb_width              (HTMLTextSlave      *slave,
						   HTMLPainter        *painter,
						   gint                words);
gchar      *html_text_slave_remove_leading_space  (HTMLTextSlave      *slave,
						   HTMLPainter        *painter,
						   gboolean            lineBegin);
gint        html_text_slave_get_nb_width          (HTMLTextSlave      *slave,
						   HTMLPainter        *painter,
						   gboolean            lineBegin);
GList      *html_get_glyphs_non_tab               (GList              *glyphs,
						   PangoItem          *item,
						   gint                ii,
						   const gchar        *text,
						   gint                bytes,
						   gint                len);
GSList     *html_text_slave_get_glyph_items       (HTMLTextSlave      *slave,
						   HTMLPainter        *painter);
gboolean    html_text_slave_cursor_right          (HTMLTextSlave      *slave,
						   HTMLCursor         *cursor);
gboolean    html_text_slave_cursor_left           (HTMLTextSlave      *slave,
						   HTMLCursor         *cursor);
gboolean    html_text_slave_cursor_head           (HTMLTextSlave      *slave,
						   HTMLCursor         *cursor);
gboolean    html_text_slave_cursor_tail           (HTMLTextSlave      *slave,
						   HTMLCursor         *cursor);
void        html_text_slave_get_cursor_base       (HTMLTextSlave      *slave,
						   HTMLPainter        *painter,
						   guint               offset,
						   gint               *x,
						   gint               *y);
int         html_text_slave_get_left_edge_offset  (HTMLTextSlave      *slave);
int         html_text_slave_get_right_edge_offset (HTMLTextSlave      *slave);
#endif /* _HTMLTEXTSLAVE_H_ */
