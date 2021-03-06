/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.
 *
 *  Copyright (C) 2000 Helix Code, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
*/

#ifndef _HTMLENGINE_CURSOR_H
#define _HTMLENGINE_CURSOR_H

#include "htmltypes.h"

void html_engine_set_cursor_blink_timeout (gint timeout);

void  html_engine_hide_cursor            (HTMLEngine *engine);
void  html_engine_show_cursor            (HTMLEngine *engine);
void  html_engine_draw_cell_cursor       (HTMLEngine *engine);
void  html_engine_draw_table_cursor      (HTMLEngine *engine);
void  html_engine_draw_cursor_in_area    (HTMLEngine *engine,
					  gint        x,
					  gint        y,
					  gint        width,
					  gint        height);
void  html_engine_setup_blinking_cursor  (HTMLEngine *engine);
void  html_engine_stop_blinking_cursor   (HTMLEngine *engine);
void  html_engine_reset_blinking_cursor  (HTMLEngine *engine);

#endif /* _HTMLENGINE_CURSOR_H */
