/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

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

#include <string.h>

#include "htmlobject.h"
#include "htmlclue.h"
#include "htmltext.h"
#include "htmltextmaster.h"
#include "htmlclueflow.h"
#include "htmlcluev.h"
#include "htmlpainter.h"
#include "htmlrule.h"
#include "htmlclue.h"
#include "htmlcursor.h"

#include "gtkhtmldebug.h"


HTMLObjectClass html_object_class;

#define HO_CLASS(x) HTML_OBJECT_CLASS (HTML_OBJECT (x)->klass)


/* HTMLObject virtual methods.  */

static void
destroy (HTMLObject *self)
{
	if (self->redraw_pending)
		self->free_pending = TRUE;
	else
		g_free (self);
}

static void
copy (HTMLObject *self,
      HTMLObject *dest)
{
	dest->klass = self->klass;
	dest->parent = NULL;
	dest->prev = NULL;
	dest->next = NULL;
	dest->x = 0;
	dest->y = 0;
	dest->ascent = self->ascent;
	dest->descent = self->descent;
	dest->width = self->width;
	dest->max_width = self->max_width;
	dest->percent = self->percent;
	dest->flags = self->flags;
	dest->redraw_pending = self->redraw_pending;
	dest->selected = self->selected;
	dest->free_pending = FALSE;
}

static void
draw (HTMLObject *o,
      HTMLPainter *p,
      gint x, gint y,
      gint width, gint height,
      gint tx, gint ty)
{
}

static HTMLFitType
fit_line (HTMLObject *o,
	  HTMLPainter *painter,
	  gboolean start_of_line,
	  gboolean first_run,
	  gint width_left)
{
	return HTML_FIT_COMPLETE;
}

static gboolean
calc_size (HTMLObject *o,
	   HTMLPainter *painter)
{
	return FALSE;
}

static gint
calc_min_width (HTMLObject *o,
		HTMLPainter *painter)
{
	html_object_calc_size (o, painter);
	return o->width;
}

static gint
calc_preferred_width (HTMLObject *o,
		      HTMLPainter *painter)
{
	html_object_calc_size (o, painter);
	return o->width;
}

static void
set_max_ascent (HTMLObject *o, HTMLPainter *painter, gint a)
{
}
	
static void
set_max_descent (HTMLObject *o, HTMLPainter *painter, gint d)
{
}
	
static void
set_max_width (HTMLObject *o, HTMLPainter *painter, gint max_width)
{
	o->max_width = max_width;
}

static void
reset (HTMLObject *o)
{
}

static const gchar *
get_url (HTMLObject *o)
{
	return NULL;
}

static const gchar *
get_target (HTMLObject *o)
{
	return NULL;
}

static HTMLAnchor *
find_anchor (HTMLObject *o,
	     const gchar *name,
	     gint *x, gint *y)
{
	return NULL;
}

static void
set_bg_color (HTMLObject *o,
	      GdkColor *color)
{
}

static HTMLObject*
check_point (HTMLObject *self,
	     HTMLPainter *painter,
	     gint x, gint y,
	     guint *offset_return,
	     gboolean for_cursor)
{
	if (x >= self->x
	    && x < self->x + self->width
	    && y >= self->y - self->ascent
	    && y < self->y + self->descent) {
		if (offset_return != NULL)
			*offset_return = 0;
		return self;
	}
    
	return NULL;
}

static gboolean
relayout (HTMLObject *self,
	  HTMLEngine *engine,
	  HTMLObject *child)
{
	/* FIXME int types of this stuff might change in `htmlobject.h',
           remember to sync.  */
	guint prev_width;
	guint prev_ascent, prev_descent;
	gboolean changed;

	if (html_engine_frozen (engine))
		return FALSE;

	prev_width = self->width;
	prev_ascent = self->ascent;
	prev_descent = self->descent;

	html_object_reset (self);

	changed = html_object_calc_size (self, engine->painter);

	if (prev_width == self->width
	    && prev_ascent == self->ascent
	    && prev_descent == self->descent) {
		gtk_html_debug_log (engine->widget,
				    "relayout: %s %p did not change.\n",
				    html_type_name (HTML_OBJECT_TYPE (self)),
				    self);
		if (changed)
			html_engine_queue_draw (engine, self);

		return FALSE;
	}
	
	gtk_html_debug_log (engine->widget, "relayout: %s %p changed.\n",
			    html_type_name (HTML_OBJECT_TYPE (self)), self);

	if (self->parent == NULL) {
		/* FIXME resize the widget, e.g. scrollbars and such.  */
		html_engine_queue_draw (engine, self);

		/* FIXME extreme ugliness.  */
		self->x = 0;
		self->y = self->ascent;
	} else {
		/* Relayout our parent starting from us.  */
		if (! html_object_relayout (self->parent, engine, self))
			html_engine_queue_draw (engine, self);
	}

	/* If the object has shrunk, we have to clean the areas around
	   it so that we don't leave garbage on the screen.  FIXME:
	   this wastes some time if there is an object on the right of
	   or under this one.  */

	if (prev_ascent + prev_descent > self->ascent + self->descent)
		html_engine_queue_clear (engine,
					 self->x,
					 self->y + self->descent,
					 self->width,
					 (prev_ascent + prev_descent
					  - (self->ascent + self->descent)));

	if (prev_width > self->width)
		html_engine_queue_clear (engine,
					 self->x + self->width,
					 self->y - self->ascent,
					 prev_width - self->width,
					 self->ascent + self->descent);

	return TRUE;
}

static gboolean
accepts_cursor (HTMLObject *self)
{
	return FALSE;
}

static void
get_cursor (HTMLObject *self,
	    HTMLPainter *painter,
	    guint offset,
	    gint *x1, gint *y1,
	    gint *x2, gint *y2)
{
	html_object_get_cursor_base (self, painter, offset, x2, y2);

	*x1 = *x2;
	*y1 = *y2 - self->ascent;
	*y2 += self->descent - 1;
}

static void
get_cursor_base (HTMLObject *self,
		 HTMLPainter *painter,
		 guint offset,
		 gint *x, gint *y)
{
	html_object_calc_abs_position (self, x, y);

	if (offset > 0)
		*x += self->width;
}

static gboolean
select_range (HTMLObject *self,
	      HTMLEngine *engine,
	      guint start,
	      gint length,
	      gboolean queue_draw)
{
	gboolean selected;
	gboolean changed;

	if (length != 0)
		selected = TRUE;
	else
		selected = FALSE;

	if ((! selected && self->selected) || (selected && ! self->selected))
		changed = TRUE;
	else
		changed = FALSE;

	self->selected = selected;

	if (queue_draw && changed)
		html_engine_queue_draw (engine, self);

	return changed;
}

static HTMLObject *
get_selection (HTMLObject *self)
{
	if (! self->selected)
		return NULL;
	
	return html_object_dup (self);
}

static void
forall (HTMLObject *self,
	HTMLObjectForallFunc func,
	gpointer data)
{
	(* func) (self, data);
}

static gboolean
is_container (HTMLObject *self)
{
	return FALSE;
}

static gboolean
save (HTMLObject *self,
      HTMLEngineSaveState *state)
{
	return TRUE;
}

static gint
check_page_split (HTMLObject *self,
		  gint y)
{
	return y;
}


/* Class initialization.  */

void
html_object_type_init (void)
{
	html_object_class_init (&html_object_class, HTML_TYPE_OBJECT, sizeof (HTMLObject));
}

void
html_object_class_init (HTMLObjectClass *klass,
			HTMLType type,
			guint object_size)
{
	g_return_if_fail (klass != NULL);

	/* Set type.  */
	klass->type = type;
	klass->object_size = object_size;

	/* Install virtual methods.  */
	klass->destroy = destroy;
	klass->copy = copy;
	klass->draw = draw;
	klass->fit_line = fit_line;
	klass->calc_size = calc_size;
	klass->set_max_ascent = set_max_ascent;
	klass->set_max_descent = set_max_descent;
	klass->set_max_width = set_max_width;
	klass->reset = reset;
	klass->calc_min_width = calc_min_width;
	klass->calc_preferred_width = calc_preferred_width;
	klass->get_url = get_url;
	klass->get_target = get_target;
	klass->find_anchor = find_anchor;
	klass->set_bg_color = set_bg_color;
	klass->check_point = check_point;
	klass->relayout = relayout;
	klass->accepts_cursor = accepts_cursor;
	klass->get_cursor = get_cursor;
	klass->get_cursor_base = get_cursor_base;
	klass->select_range = select_range;
	klass->get_selection = get_selection;
	klass->forall = forall;
	klass->is_container = is_container;
	klass->save = save;
	klass->check_page_split = check_page_split;
}

void
html_object_init (HTMLObject *o,
		  HTMLObjectClass *klass)
{
	o->klass = klass;

	o->parent = NULL;
	o->prev = NULL;
	o->next = NULL;

	o->x = 0;
	o->y = 0;

	o->ascent = 0;
	o->descent = 0;

	o->width = 0;
	o->max_width = 0;
	o->percent = 0;

	o->flags = HTML_OBJECT_FLAG_FIXEDWIDTH; /* FIXME Why? */

	o->redraw_pending = FALSE;
	o->free_pending = FALSE;
	o->selected = FALSE;
}

HTMLObject *
html_object_new (HTMLObject *parent)
{
	HTMLObject *o;
	
	o = g_new0 (HTMLObject, 1);
	html_object_init (o, &html_object_class);

	return o;
}


/* Object duplication.  */

HTMLObject *
html_object_dup (HTMLObject *object)
{
	HTMLObject *new;

	g_return_val_if_fail (object != NULL, NULL);

	new = g_malloc (object->klass->object_size);
	html_object_copy (object, new);

	return new;
}


void
html_object_set_parent (HTMLObject *o,
			HTMLObject *parent)
{
	o->parent = parent;
}

void
html_object_calc_abs_position (HTMLObject *o,
			       gint *x_return, gint *y_return)
{
	HTMLObject *p;

	g_return_if_fail (o != NULL);

	*x_return = o->x;
	*y_return = o->y;

	for (p = o->parent; p != NULL; p = p->parent) {
		*x_return += p->x;
		*y_return += p->y - p->ascent;
	}
}


/* Virtual methods.  */

void
html_object_destroy (HTMLObject *self)
{
	(* HO_CLASS (self)->destroy) (self);
}

void
html_object_copy (HTMLObject *self,
		  HTMLObject *dest)
{
	(* HO_CLASS (self)->copy) (self, dest);
}

void
html_object_draw (HTMLObject *o,
		  HTMLPainter *p,
		  gint x, gint y,
		  gint width, gint height,
		  gint tx, gint ty)
{
	(* HO_CLASS (o)->draw) (o, p, x, y, width, height, tx, ty);
}

HTMLFitType
html_object_fit_line (HTMLObject *o,
		      HTMLPainter *painter,
		      gboolean start_of_line, 
		      gboolean first_run,
		      gint width_left)
{
	return (* HO_CLASS (o)->fit_line) (o, painter, start_of_line,
					   first_run, width_left);
}

gboolean
html_object_calc_size (HTMLObject *o,
		       HTMLPainter *painter)
{
	return (* HO_CLASS (o)->calc_size) (o, painter);
}

void
html_object_set_max_ascent (HTMLObject *o, HTMLPainter *painter, gint a)
{
	(* HO_CLASS (o)->set_max_ascent) (o, painter, a);
}

void
html_object_set_max_descent (HTMLObject *o, HTMLPainter *painter, gint d)
{
	(* HO_CLASS (o)->set_max_descent) (o, painter, d);
}

void
html_object_set_max_width (HTMLObject *o, HTMLPainter *painter, gint max_width)
{
	(* HO_CLASS (o)->set_max_width) (o, painter, max_width);
}

void
html_object_reset (HTMLObject *o)
{
	(* HO_CLASS (o)->reset) (o);
}

gint
html_object_calc_min_width (HTMLObject *o,
			    HTMLPainter *painter)
{
	return (* HO_CLASS (o)->calc_min_width) (o, painter);
}

gint
html_object_calc_preferred_width (HTMLObject *o,
				  HTMLPainter *painter)
{
	return (* HO_CLASS (o)->calc_preferred_width) (o, painter);
}

const gchar *
html_object_get_url (HTMLObject *o)
{
	return (* HO_CLASS (o)->get_url) (o);
}

const gchar *
html_object_get_target (HTMLObject *o)
{
	return (* HO_CLASS (o)->get_target) (o);
}

HTMLAnchor *
html_object_find_anchor (HTMLObject *o,
			 const gchar *name,
			 gint *x, gint *y)
{
	return (* HO_CLASS (o)->find_anchor) (o, name, x, y);
}

void
html_object_set_bg_color (HTMLObject *o, GdkColor *color)
{
	(* HO_CLASS (o)->set_bg_color) (o, color);
}

HTMLObject *
html_object_check_point (HTMLObject *self,
			 HTMLPainter *painter,
			 gint x, gint y,
			 guint *offset_return,
			 gboolean for_cursor)
{
	return (* HO_CLASS (self)->check_point) (self, painter, x, y, offset_return, for_cursor);
}

gboolean
html_object_relayout (HTMLObject *self,
		      HTMLEngine *engine,
		      HTMLObject *child)
{
	return (* HO_CLASS (self)->relayout) (self, engine, child);
}

gboolean
html_object_accepts_cursor (HTMLObject *self)
{
	return (* HO_CLASS (self)->accepts_cursor) (self);
}

/* Warning: `calc_size()' must have been called on `self' before this so that
   this works correctly.  */
void
html_object_get_cursor (HTMLObject *self,
			HTMLPainter *painter,
			guint offset,
			gint *x1, gint *y1,
			gint *x2, gint *y2)
{
	(* HO_CLASS (self)->get_cursor) (self, painter, offset, x1, y1, x2, y2);
}

/* Warning: `calc_size()' must have been called on `self' before this so that
   this works correctly.  */
void
html_object_get_cursor_base (HTMLObject *self,
			     HTMLPainter *painter,
			     guint offset,
			     gint *x, gint *y)
{
	(* HO_CLASS (self)->get_cursor_base) (self, painter, offset, x, y);
}


gboolean
html_object_select_range (HTMLObject *self,
			  HTMLEngine *engine,
			  guint start,
			  gint length,
			  gboolean queue_draw)
{
	return (* HO_CLASS (self)->select_range) (self, engine, start, length, queue_draw);
}

HTMLObject *
html_object_get_selection (HTMLObject *self)
{
	return (* HO_CLASS (self)->get_selection) (self);
}


void
html_object_forall (HTMLObject *self,
		    HTMLObjectForallFunc func,
		    gpointer data)
{
	(* HO_CLASS (self)->forall) (self, func, data);
}

/* Ugly.  We should have an `is_a' implementation.  */
gboolean
html_object_is_container (HTMLObject *self)
{
	return (* HO_CLASS (self)->is_container) (self);
}


/* Ugly.  We should have an `is_a' implementation.  */
gboolean
html_object_is_text (HTMLObject *object)
{
	HTMLType type;

	type = HTML_OBJECT_TYPE (object);

	return (type == HTML_TYPE_TEXT
		|| type == HTML_TYPE_TEXTMASTER
		|| type == HTML_TYPE_LINKTEXT
		|| type == HTML_TYPE_LINKTEXTMASTER);
}


gboolean
html_object_save (HTMLObject *self,
		  HTMLEngineSaveState *state)
{
	return (* HO_CLASS (self)->save) (self, state);
}


gint
html_object_check_page_split  (HTMLObject *self,
			       gint y)
{
	g_return_val_if_fail (self != NULL, 0);

	return (* HO_CLASS (self)->check_page_split) (self, y);
}

