/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library

    Copyright (C) 2000 Jonas Borgstr�m <jonas_b@bitsmart.com>

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
#ifndef _HTMLRADIO_H_
#define _HTMLRADIO_H_

#include "htmlelement.h"

#define HTML_RADIO(x) ((HTMLRadio *) (x))
#define HTML_RADIO_CLASS(x) ((HTMLRadioClass *) (x))

typedef struct _HTMLRadio HTMLRadio;
typedef struct _HTMLRadioClass HTMLRadioClass;

struct _HTMLRadio {
	HTMLElement element;
};

struct _HTMLRadioClass {
	HTMLElementClass element_class;
};


extern HTMLRadioClass html_radio_class;


void html_radio_type_init (void);
void html_radio_class_init (HTMLRadioClass *klass, HTMLType type);
void html_radio_init (HTMLRadio *radio, HTMLRadioClass *klass, GtkWidget *parent, gchar *name, gchar *value, gboolean checked);
HTMLObject *html_radio_new (GtkWidget *parent, gchar *name, gchar *value, gboolean checked);

#endif /* _HTMLRADIO_H_ */
