/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library.
   
   Copyright (C) 1997 Martin Jones (mjones@kde.org)
   Copyright (C) 1997 Torben Weis (weis@kde.org)
   Copyright (C) 1999 Helix Code, Inc.

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

#ifndef _HTMLCLUEFLOW_H_
#define _HTMLCLUEFLOW_H_

#include "htmlobject.h"
#include "htmlclue.h"

#define HTML_CLUEFLOW(x) ((HTMLClueFlow *)(x))
#define HTML_CLUEFLOW_CLASS(x) ((HTMLClueFlow *)(x))

typedef struct _HTMLClueFlow HTMLClueFlow;
typedef struct _HTMLClueFlowClass HTMLClueFlowClass;
typedef enum _HTMLClueFlowStyle HTMLClueFlowStyle;

enum _HTMLClueFlowStyle {
	HTML_CLUEFLOW_STYLE_NORMAL,
	HTML_CLUEFLOW_STYLE_P,
	HTML_CLUEFLOW_STYLE_H1,
	HTML_CLUEFLOW_STYLE_H2,
	HTML_CLUEFLOW_STYLE_H3,
	HTML_CLUEFLOW_STYLE_H4,
	HTML_CLUEFLOW_STYLE_H5,
	HTML_CLUEFLOW_STYLE_H6,
	HTML_CLUEFLOW_STYLE_ADDRESS,
	HTML_CLUEFLOW_STYLE_ITEMDOTTED,
	HTML_CLUEFLOW_STYLE_ITEMROMAN,
	HTML_CLUEFLOW_STYLE_ITEMDIGIT
};

struct _HTMLClueFlow {
	HTMLClue clue;

	/* Font for the itemized lists.  */
	HTMLFont *font;

	/* Paragraph style.  */
	HTMLClueFlowStyle style;

	/* Level for itemized lists and indented paragraphs.  */
	guint8 level;
};

struct _HTMLClueFlowClass {
	HTMLClueClass clue_class;
};


extern HTMLClueFlowClass html_clueflow_class;


void html_clueflow_type_init (void);
void html_clueflow_class_init (HTMLClueFlowClass *klass, HTMLType type);
void html_clueflow_init (HTMLClueFlow *flow, HTMLClueFlowClass *klass,
			 HTMLFont *font, HTMLClueFlowStyle style, guint8 level);
HTMLObject *html_clueflow_new (HTMLFont *font, HTMLClueFlowStyle style, guint8 level);

HTMLClueFlow *html_clueflow_split (HTMLClueFlow *clue, HTMLObject *child);

#endif /* _HTMLCLUEFLOW_H_ */
