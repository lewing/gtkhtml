/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the KDE libraries

   Copyright (C) 1997 Martin Jones (mjones@kde.org)
   Copyright (C) 1997 Torben Weis (weis@kde.org)
   Copyright (C) 2000 Helix Code, Inc.
   
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

#include "htmlvspace.h"
#include "htmlclueflow.h"


HTMLVSpaceClass html_vspace_class;


static void
calc_size (HTMLObject *self,
	   HTMLPainter *painter)
{
	HTMLFontStyle font_style;

	if (self->parent != NULL
	    && HTML_OBJECT_TYPE (self->parent) == HTML_TYPE_CLUEFLOW)
		font_style = html_clueflow_get_default_font_style
			(HTML_CLUEFLOW (self->parent));
	else
		font_style = HTML_FONT_STYLE_SIZE_3;

	self->ascent = html_painter_calc_ascent (painter, font_style);
	self->descent = html_painter_calc_descent (painter, font_style);

	self->width = 2;
}


void
html_vspace_type_init (void)
{
	html_vspace_class_init (&html_vspace_class, HTML_TYPE_VSPACE, sizeof (HTMLVSpace));
}

void
html_vspace_class_init (HTMLVSpaceClass *klass,
			HTMLType type,
			guint object_size)
{
	HTMLObjectClass *object_class;

	object_class = HTML_OBJECT_CLASS (klass);

	html_object_class_init (object_class, type, object_size);

	object_class->calc_size = calc_size;
}

void
html_vspace_init (HTMLVSpace *vspace,
		  HTMLVSpaceClass *klass,
		  HTMLClearType clear)
{
	HTMLObject *object;

	object = HTML_OBJECT (vspace);

	html_object_init (object, HTML_OBJECT_CLASS (klass));
	
	object->width = 1;
	object->flags |= HTML_OBJECT_FLAG_NEWLINE;
	
	vspace->clear = clear;
}

HTMLObject *
html_vspace_new (HTMLClearType clear)
{
	HTMLVSpace *vspace;

	vspace = g_new (HTMLVSpace, 1);
	html_vspace_init (vspace, &html_vspace_class, clear);

	return HTML_OBJECT (vspace);
}
