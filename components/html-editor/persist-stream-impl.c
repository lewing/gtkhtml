/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

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

    Author: Ettore Perazzoli <ettore@helixcode.com>
*/

/* This file implements the Bonobo::PersistStream interface for the HTML editor
   control.  */

#include <config.h>

#include <gnome.h>
#include <bonobo.h>

#include "gtkhtml.h"

#include "persist-stream-impl.h"


/* Loading.  */

#define READ_CHUNK_SIZE 4096

static void
load (BonoboPersistStream *ps,
      Bonobo_Stream stream,
      Bonobo_Persist_ContentType type,
      gpointer data,
      CORBA_Environment *ev)
{
	GtkHTML *html;
	CORBA_long bytes_read;
	Bonobo_Stream_iobuf *buffer;
	GtkHTMLStream *handle;

	if (strcmp (type, "text/html") != 0)
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	html = GTK_HTML (data);

	handle = gtk_html_begin (html);

	do {
		bytes_read = Bonobo_Stream_read (stream, READ_CHUNK_SIZE,
						 &buffer, ev);
		if (ev->_major != CORBA_NO_EXCEPTION) {
			bytes_read = -1;
			break;
		}

		gtk_html_write (html, handle, buffer->_buffer, buffer->_length);

		CORBA_free (buffer);
	} while (bytes_read > 0);

	if (bytes_read < 0)
		gtk_html_end (html, handle, GTK_HTML_STREAM_ERROR);
	else
		gtk_html_end (html, handle, GTK_HTML_STREAM_OK);
}


/* Saving.  */

struct _SaveState {
	Bonobo_Stream stream;
	CORBA_Environment *ev;
};
typedef struct _SaveState SaveState;

static gboolean
save_receiver (const HTMLEngine *engine,
	       const gchar *data,
	       guint length,
	       gpointer user_data)
{
	Bonobo_Stream_iobuf buffer;
	CORBA_long bytes_written;
	SaveState *state;

	state = (SaveState *) user_data;
	if (state->ev->_major != CORBA_NO_EXCEPTION)
		return FALSE;

	buffer._maximum = length;
	buffer._length = length;
	buffer._buffer = (CORBA_char *) data; /* Should be safe.  */

	bytes_written = Bonobo_Stream_write (state->stream, &buffer, state->ev);

	if (bytes_written != length || state->ev->_major != CORBA_NO_EXCEPTION)
		return FALSE;

	return TRUE;
}

static void
save (BonoboPersistStream *ps,
      Bonobo_Stream stream,
      Bonobo_Persist_ContentType type,
      gpointer data,
      CORBA_Environment *ev)
{
	GtkHTML *html;
	SaveState save_state;

	if (strcmp (type, "text/html") != 0) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	html = GTK_HTML (data);

	save_state.ev = ev;
	save_state.stream = CORBA_Object_duplicate (stream, ev);
	if (ev->_major == CORBA_NO_EXCEPTION)
		gtk_html_save (html, save_receiver, &save_state);
	CORBA_Object_release (save_state.stream, ev);
}

static Bonobo_Persist_ContentTypeList *
get_content_types (BonoboPersistStream *ps, gpointer data,
		   CORBA_Environment *ev)
{
	return bonobo_persist_generate_content_types (1, "text/html");
}


BonoboPersistStream *
persist_stream_impl_new (GtkHTML *html)
{
	return bonobo_persist_stream_new (load, save, NULL, get_content_types,
					  html);
}
