/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the KDE libraries

   Copyright (C) 1997 Martin Jones (mjones@kde.org)
   Copyright (C) 1997 Torben Weis (weis@kde.org)
   Copyright (C) 1999, 2000 Helix Code, Inc.
   
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
#ifndef _HTMLIMAGE_H_
#define _HTMLIMAGE_H_

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include "htmlobject.h"
#include "htmlengine.h"


typedef struct _HTMLImage HTMLImage;
typedef struct _HTMLImageClass HTMLImageClass;
typedef struct _HTMLImagePointer HTMLImagePointer;
typedef struct _HTMLImageFactory HTMLImageFactory;

struct _HTMLImagePointer {
	gchar *url;
	GdkPixbufLoader *loader;
	GdkPixbuf *pixbuf;
	GSList *interests; /* A list of HTMLImage's, or a NULL pointer for the background pixmap */
	HTMLImageFactory *factory;
};

#define HTML_IMAGE(x) ((HTMLImage *)(x))

struct _HTMLImage {
	HTMLObject object;
        HTMLImagePointer *image_ptr;

	gint border;

	gint specified_width;
	gint specified_height;

	gchar *url;
	gchar *target;
};

struct _HTMLImageClass {
	HTMLObjectClass parent_class;
};


extern HTMLImageClass image_class;


void        html_image_type_init   (void);
void        html_image_class_init  (HTMLImageClass   *klass,
				    HTMLType          type,
				    guint             object_size);
void        html_image_init        (HTMLImage        *image,
				    HTMLImageClass   *klass,
				    HTMLImageFactory *imf,
				    gchar            *filename,
				    const gchar      *url,
				    const gchar      *target,
				    gint              width,
				    gint              height,
				    gint              percent,
				    gint              border);
HTMLObject *html_image_new         (HTMLImageFactory *imf,
				    gchar            *filename,
				    const gchar      *url,
				    const gchar      *target,
				    gint              width,
				    gint              height,
				    gint              percent,
				    gint              border);

/* FIXME move to htmlimagefactory.c */
HTMLImageFactory *html_image_factory_new      (HTMLEngine       *e);
void              html_image_factory_free     (HTMLImageFactory *factory);
void              html_image_factory_cleanup  (HTMLImageFactory *factory); /* Does gc etc. - removes unused image entries */

HTMLImagePointer *html_image_factory_register    (HTMLImageFactory *factory,
						  HTMLImage        *i,
						  const char       *filename);
void              html_image_factory_unregister  (HTMLImageFactory *factory,
						  HTMLImagePointer *pointer,
						  HTMLImage        *i);

#endif /* _HTMLIMAGE_H_ */
