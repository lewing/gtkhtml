/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

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

#include <glib.h>

#include "htmlobject.h"
#include "htmlclueflow.h"
#include "htmlcursor.h"
#include "htmlhspace.h"
#include "htmltext.h"
#include "htmltextslave.h"

#include "htmlengine-edit.h"


static void
queue_draw_for_cursor (HTMLEngine *e)
{
	HTMLObject *current;

	current = e->cursor->object;
	if (current == NULL)
		return;

	if (html_object_is_text (current))
		html_text_queue_draw (HTML_TEXT (current),
				      e, e->cursor->offset, 1);
	else
		html_engine_queue_draw (e, current);
}

guint
html_engine_move_cursor (HTMLEngine *e,
			 HTMLEngineCursorMovement movement,
			 guint count)
{
	gboolean (* movement_func) (HTMLCursor *, HTMLEngine *);
	guint c;

	g_return_val_if_fail (e != NULL, 0);

	if (count == 0)
		return 0;

	switch (movement) {
	case HTML_ENGINE_CURSOR_RIGHT:
		movement_func = html_cursor_forward;
		break;

	case HTML_ENGINE_CURSOR_LEFT:
		movement_func = html_cursor_backward;
		break;

	case HTML_ENGINE_CURSOR_UP:
		movement_func = html_cursor_up;
		break;

	case HTML_ENGINE_CURSOR_DOWN:
		movement_func = html_cursor_down;
		break;

	default:
		g_warning ("Unsupported movement %d\n", (gint) movement);
		return 0;
	}

	queue_draw_for_cursor (e);

	for (c = 0; c < count; c++) {
		if (! (* movement_func) (e->cursor, e))
			break;
	}

	queue_draw_for_cursor (e);

	return c;
}


/* Paragraph insertion.  */

/* FIXME this sucks.  */
static HTMLObject *
get_flow (HTMLObject *object)
{
	HTMLObject *p;

	for (p = object->parent; p != NULL; p = p->parent) {
		if (HTML_OBJECT_TYPE (p) == HTML_TYPE_CLUEFLOW)
			break;
	}

	return p;
}

void
html_engine_insert_para (HTMLEngine *e,
			 gboolean vspace)
{
	HTMLObject *flow;
	HTMLObject *next_flow;
	HTMLObject *current;
	guint offset;

	g_return_if_fail (e != NULL);
	g_return_if_fail (HTML_IS_ENGINE (e));
	g_return_if_fail (e->cursor != NULL);
	g_return_if_fail (e->cursor->object != NULL);

	current = e->cursor->object;
	offset = e->cursor->offset;

	flow = get_flow (current);
	if (flow == NULL) {
		g_warning ("%s: Object %p (%s) is not contained in an HTMLClueFlow\n",
			   __FUNCTION__, current, html_type_name (HTML_OBJECT_TYPE (current)));
		return;
	}

	/* Remove text slaves, if any.  */

	if (html_object_is_text (current) && current->next != NULL) {
		HTMLObject *p;
		HTMLObject *pnext;

		for (p = current->next;
		     p != NULL && HTML_OBJECT_TYPE (p) == HTML_TYPE_TEXTSLAVE;
		     p = pnext) {
			pnext = p->next;
			html_object_destroy (p);
		}

		current->next = p;
		if (p != NULL)
			p->prev = current;
	}

	if (offset > 0) {
		if (current->next != NULL) {
			next_flow = HTML_OBJECT (html_clueflow_split (HTML_CLUEFLOW (flow),
								      current->next));
		} else {
			/* FIXME we need a `html_clueflow_like_another_one()'.  */
			next_flow = html_clueflow_new (HTML_CLUEFLOW (flow)->font,
						       HTML_CLUEFLOW (flow)->style,
						       HTML_CLUEFLOW (flow)->level);
		}
	} else {
		next_flow = HTML_OBJECT (html_clueflow_split (HTML_CLUEFLOW (flow), current));
		if (current->prev == NULL) {
			HTMLObject *hspace;

			/* We don't want the clue to be empty: so we add an
                           hidden hspace with an appropriate font.  FIXME FIXME
                           FIXME: Currently, "appropriate font" makes sense
                           when we are on a text object only.  So, in the other
                           cases, we don't do anything, which is broken.  */

			if (html_object_is_text (current)) {
				hspace = html_hspace_new (HTML_TEXT (current)->font, TRUE);
				html_clue_append (HTML_CLUE (flow), hspace);
			}
		}
	}

	html_clue_append_after (HTML_CLUE (flow->parent), next_flow, flow);

	if (offset > 0 && html_object_is_text (current)) {
		HTMLObject *text_next;

		text_next = HTML_OBJECT (html_text_split (HTML_TEXT (current), offset));
		html_clue_prepend (HTML_CLUE (next_flow), text_next);

		e->cursor->object = text_next;
		e->cursor->offset = 0;
		e->cursor->have_target_x = FALSE;
	}

	if (flow->parent == NULL) {
		html_object_relayout (flow, e, NULL);
		html_engine_queue_draw (e, flow);
	} else {
		html_object_relayout (flow->parent, e, flow);
		html_engine_queue_draw (e, flow->parent);
	}
}



/* FIXME This should actually do a lot more.  */
void
html_engine_insert (HTMLEngine *e,
		    const gchar *text,
		    guint len)
{
	HTMLObject *current_object;

	g_return_if_fail (e != NULL);
	g_return_if_fail (HTML_IS_ENGINE (e));
	g_return_if_fail (text != NULL);

	if (len == 0)
		return;

	current_object = e->cursor->object;

	if (! html_object_is_text (current_object)) {
		g_warning ("Cannot insert text in object of type `%s'",
			   html_type_name (HTML_OBJECT_TYPE (current_object)));
		return;
	}

	html_text_insert_text (HTML_TEXT (current_object), e,
			       e->cursor->offset, text, len);
}

/* FIXME This should actually do a lot more.  */
void
html_engine_delete (HTMLEngine *e,
		    guint count)
{
	HTMLObject *current_object;
	HTMLObject *parent;
	HTMLObject *next;
	HTMLType type;

	g_return_if_fail (e != NULL);
	g_return_if_fail (HTML_IS_ENGINE (e));

	current_object = e->cursor->object;

	while (count > 0) {

		/* Check the parent.  */

		parent = current_object->parent;
		if (parent == NULL) {
			g_warning ("Cannot delete in object %p that has no parent.",
				   current_object);
			break;
		}
		if (HTML_OBJECT_TYPE (parent) != HTML_TYPE_CLUEFLOW) {
			g_warning ("Cannot delete in object %p whose parent is a %s.",
				   current_object,
				   html_type_name (HTML_OBJECT_TYPE (current_object)));
			break;
		}

		if (html_object_is_text (current_object)) {
			count -= html_text_remove_text (HTML_TEXT (current_object), e,
							e->cursor->offset, count);
			continue;
		}

		/* Not a text object: it must be deleted completely.  */

		e->cursor->offset = 0;

		next = current_object->next;
		type = HTML_OBJECT_TYPE (current_object);

		html_clue_remove (HTML_CLUE (current_object->parent),
				  current_object);
		html_object_destroy (current_object);

		/* FIXME optimize?  */
		html_object_relayout (parent, e, next);

		if (type == HTML_TYPE_HSPACE && next == NULL) {
			HTMLObject *next_para;

			/* This is the trailing hspace for the paragraph: we
                           have to merge this with the following paragraph.  */

			next_para = parent->next;
			if (next_para == NULL
			    || HTML_OBJECT_TYPE (next_para) != HTML_TYPE_CLUEFLOW)
				break;
			if (next_para->parent == NULL)
				break;

			next = HTML_CLUE (next_para)->head;
			html_clue_append (HTML_CLUE (parent), HTML_CLUE (next_para)->head);

			html_clue_remove (HTML_CLUE (next_para->parent), next_para);
			HTML_CLUE (next_para)->head = NULL;
			html_object_destroy (next_para);
		}

		count--;

		current_object = next;
	}

	e->cursor->object = current_object;
}
