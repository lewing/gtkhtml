/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
   Copyright (C) 2000 Helix Code, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Radek Doulik <rodo@helixcode.com>

*/

#ifndef _GTK_HTML_ENUMS_H
#define _GTK_HTML_ENUMS_H

typedef enum {
	GTK_HTML_COMMAND_UNDO,
	GTK_HTML_COMMAND_REDO,
	GTK_HTML_COMMAND_COPY,
	GTK_HTML_COMMAND_CUT,
	GTK_HTML_COMMAND_PASTE,

	GTK_HTML_COMMAND_CUT_LINE,

	GTK_HTML_COMMAND_INSERT_PARAGRAPH,
	GTK_HTML_COMMAND_INSERT_RULE,
	GTK_HTML_COMMAND_INSERT_RULE_PARAM,
	GTK_HTML_COMMAND_INSERT_IMAGE_PARAM,

	GTK_HTML_COMMAND_MAKE_LINK,
	GTK_HTML_COMMAND_REMOVE_LINK,

	GTK_HTML_COMMAND_DELETE,
	GTK_HTML_COMMAND_DELETE_BACK,
	GTK_HTML_COMMAND_DELETE_BACK_OR_INDENT_DEC,

	GTK_HTML_COMMAND_SET_MARK,
	GTK_HTML_COMMAND_DISABLE_SELECTION,

	GTK_HTML_COMMAND_BOLD_ON,
	GTK_HTML_COMMAND_BOLD_OFF,
	GTK_HTML_COMMAND_BOLD_TOGGLE,

	GTK_HTML_COMMAND_ITALIC_ON,
	GTK_HTML_COMMAND_ITALIC_OFF,
	GTK_HTML_COMMAND_ITALIC_TOGGLE,

	GTK_HTML_COMMAND_UNDERLINE_ON,
	GTK_HTML_COMMAND_UNDERLINE_OFF,
	GTK_HTML_COMMAND_UNDERLINE_TOGGLE,

	GTK_HTML_COMMAND_STRIKEOUT_ON,
	GTK_HTML_COMMAND_STRIKEOUT_OFF,
	GTK_HTML_COMMAND_STRIKEOUT_TOGGLE,

	GTK_HTML_COMMAND_SIZE_MINUS_2,
	GTK_HTML_COMMAND_SIZE_MINUS_1,
	GTK_HTML_COMMAND_SIZE_PLUS_0,
	GTK_HTML_COMMAND_SIZE_PLUS_1,
	GTK_HTML_COMMAND_SIZE_PLUS_2,
	GTK_HTML_COMMAND_SIZE_PLUS_3,
	GTK_HTML_COMMAND_SIZE_PLUS_4,

	GTK_HTML_COMMAND_SIZE_INCREASE,
	GTK_HTML_COMMAND_SIZE_DECREASE,

	GTK_HTML_COMMAND_ALIGN_LEFT,
	GTK_HTML_COMMAND_ALIGN_CENTER,
	GTK_HTML_COMMAND_ALIGN_RIGHT,

	GTK_HTML_COMMAND_INDENT_ZERO,
	GTK_HTML_COMMAND_INDENT_INC,
	GTK_HTML_COMMAND_INDENT_DEC,
	GTK_HTML_COMMAND_INDENT_PARAGRAPH,

	GTK_HTML_COMMAND_BREAK_AND_FILL_LINE,
	GTK_HTML_COMMAND_SPACE_AND_FILL_LINE,

	GTK_HTML_COMMAND_PARAGRAPH_STYLE_NORMAL,
	GTK_HTML_COMMAND_PARAGRAPH_STYLE_H1,
	GTK_HTML_COMMAND_PARAGRAPH_STYLE_H2,
	GTK_HTML_COMMAND_PARAGRAPH_STYLE_H3,
	GTK_HTML_COMMAND_PARAGRAPH_STYLE_H4,
	GTK_HTML_COMMAND_PARAGRAPH_STYLE_H5,
	GTK_HTML_COMMAND_PARAGRAPH_STYLE_H6,
	GTK_HTML_COMMAND_PARAGRAPH_STYLE_ADDRESS,
	GTK_HTML_COMMAND_PARAGRAPH_STYLE_PRE,
	GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMDOTTED,
	GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMROMAN,
	GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMDIGIT,

	GTK_HTML_COMMAND_MODIFY_SELECTION_UP,
	GTK_HTML_COMMAND_MODIFY_SELECTION_DOWN,
	GTK_HTML_COMMAND_MODIFY_SELECTION_LEFT,
	GTK_HTML_COMMAND_MODIFY_SELECTION_RIGHT,
	GTK_HTML_COMMAND_MODIFY_SELECTION_PAGEUP,
	GTK_HTML_COMMAND_MODIFY_SELECTION_PAGEDOWN,
	GTK_HTML_COMMAND_MODIFY_SELECTION_BOL,
	GTK_HTML_COMMAND_MODIFY_SELECTION_EOL,
	GTK_HTML_COMMAND_MODIFY_SELECTION_BOD,
	GTK_HTML_COMMAND_MODIFY_SELECTION_EOD,
	GTK_HTML_COMMAND_MODIFY_SELECTION_PREV_WORD,
	GTK_HTML_COMMAND_MODIFY_SELECTION_NEXT_WORD,

	GTK_HTML_COMMAND_CAPITALIZE_WORD,
	GTK_HTML_COMMAND_UPCASE_WORD,
	GTK_HTML_COMMAND_DOWNCASE_WORD,

	GTK_HTML_COMMAND_SPELL_SUGGEST,
	GTK_HTML_COMMAND_SPELL_PERSONAL_DICTIONARY_ADD,
	GTK_HTML_COMMAND_SPELL_SESSION_DICTIONARY_ADD,

	GTK_HTML_COMMAND_SEARCH,
	GTK_HTML_COMMAND_SEARCH_INCREMENTAL_FORWARD,
	GTK_HTML_COMMAND_SEARCH_INCREMENTAL_BACKWARD,
	GTK_HTML_COMMAND_SEARCH_REGEX,

	GTK_HTML_COMMAND_FOCUS_FORWARD,
	GTK_HTML_COMMAND_FOCUS_BACKWARD,

	GTK_HTML_COMMAND_POPUP_MENU,
	GTK_HTML_COMMAND_PROPERTIES_DIALOG,

	GTK_HTML_COMMAND_CURSOR_FORWARD,
	GTK_HTML_COMMAND_CURSOR_BACKWARD,

	GTK_HTML_COMMAND_INSERT_TABLE_1_1,

	GTK_HTML_COMMAND_TABLE_INSERT_COL_AFTER,
	GTK_HTML_COMMAND_TABLE_INSERT_COL_BEFORE,
	GTK_HTML_COMMAND_TABLE_INSERT_ROW_AFTER,
	GTK_HTML_COMMAND_TABLE_INSERT_ROW_BEFORE,
	GTK_HTML_COMMAND_TABLE_DELETE_COL,
	GTK_HTML_COMMAND_TABLE_DELETE_ROW,

	GTK_HTML_COMMAND_TABLE_CELL_INC_CSPAN,
	GTK_HTML_COMMAND_TABLE_CELL_DEC_CSPAN,
	GTK_HTML_COMMAND_TABLE_CELL_INC_RSPAN,
	GTK_HTML_COMMAND_TABLE_CELL_DEC_RSPAN,

	GTK_HTML_COMMAND_TABLE_CELL_JOIN_LEFT,
	GTK_HTML_COMMAND_TABLE_CELL_JOIN_RIGHT,
	GTK_HTML_COMMAND_TABLE_CELL_JOIN_UP,
	GTK_HTML_COMMAND_TABLE_CELL_JOIN_DOWN,

	GTK_HTML_COMMAND_TABLE_BORDER_WIDTH_INC,
	GTK_HTML_COMMAND_TABLE_BORDER_WIDTH_DEC,
	GTK_HTML_COMMAND_TABLE_BORDER_WIDTH_ZERO,

	GTK_HTML_COMMAND_TEXT_SET_DEFAULT_COLOR,

	GTK_HTML_COMMAND_SELECT_WORD,
	GTK_HTML_COMMAND_SELECT_LINE,
	GTK_HTML_COMMAND_SELECT_PARAGRAPH,
	GTK_HTML_COMMAND_SELECT_PARAGRAPH_EXTENDED,
	GTK_HTML_COMMAND_SELECT_ALL,

	GTK_HTML_COMMAND_CURSOR_POSITION_SAVE,
	GTK_HTML_COMMAND_CURSOR_POSITION_RESTORE,

	GTK_HTML_COMMAND_CURSOR_BOD,
	GTK_HTML_COMMAND_CURSOR_EOD,

	GTK_HTML_COMMAND_BLOCK_REDRAW,
	GTK_HTML_COMMAND_UNBLOCK_REDRAW,
} GtkHTMLCommandType;

typedef enum {
	GTK_HTML_CURSOR_SKIP_ONE,
	GTK_HTML_CURSOR_SKIP_WORD,
	GTK_HTML_CURSOR_SKIP_PAGE,
	GTK_HTML_CURSOR_SKIP_ALL
} GtkHTMLCursorSkipType;

typedef enum {
	GTK_HTML_EDITOR_EVENT_COMMAND,
	GTK_HTML_EDITOR_EVENT_IMAGE_URL
} GtkHTMLEditorEventType;

typedef enum {
	GTK_HTML_ETCH_IN,
	GTK_HTML_ETCH_OUT,
	GTK_HTML_ETCH_NONE
} GtkHTMLEtchStyle; 

typedef enum {
	GTK_HTML_FONT_STYLE_DEFAULT = 0,
	GTK_HTML_FONT_STYLE_SIZE_1 = 1,
	GTK_HTML_FONT_STYLE_SIZE_2 = 2,
	GTK_HTML_FONT_STYLE_SIZE_3 = 3,
	GTK_HTML_FONT_STYLE_SIZE_4 = 4,
	GTK_HTML_FONT_STYLE_SIZE_5 = 5,
	GTK_HTML_FONT_STYLE_SIZE_6 = 6,
	GTK_HTML_FONT_STYLE_SIZE_7 = 7,
	GTK_HTML_FONT_STYLE_SIZE_MASK = 0x7,
	GTK_HTML_FONT_STYLE_BOLD        = 1 << 3,
	GTK_HTML_FONT_STYLE_ITALIC      = 1 << 4,
	GTK_HTML_FONT_STYLE_UNDERLINE   = 1 << 5,
	GTK_HTML_FONT_STYLE_STRIKEOUT   = 1 << 6,
	GTK_HTML_FONT_STYLE_FIXED       = 1 << 7,
	GTK_HTML_FONT_STYLE_SUBSCRIPT   = 1 << 8,
	GTK_HTML_FONT_STYLE_SUPERSCRIPT = 1 << 9
} GtkHTMLFontStyle;

typedef enum {
	GTK_HTML_PARAGRAPH_ALIGNMENT_LEFT,
	GTK_HTML_PARAGRAPH_ALIGNMENT_RIGHT,
	GTK_HTML_PARAGRAPH_ALIGNMENT_CENTER
} GtkHTMLParagraphAlignment;

typedef enum {
	GTK_HTML_PARAGRAPH_STYLE_NORMAL,
	GTK_HTML_PARAGRAPH_STYLE_H1,
	GTK_HTML_PARAGRAPH_STYLE_H2,
	GTK_HTML_PARAGRAPH_STYLE_H3,
	GTK_HTML_PARAGRAPH_STYLE_H4,
	GTK_HTML_PARAGRAPH_STYLE_H5,
	GTK_HTML_PARAGRAPH_STYLE_H6,
	GTK_HTML_PARAGRAPH_STYLE_ADDRESS,
	GTK_HTML_PARAGRAPH_STYLE_PRE,
	GTK_HTML_PARAGRAPH_STYLE_ITEMDOTTED,
	GTK_HTML_PARAGRAPH_STYLE_ITEMROMAN,
	GTK_HTML_PARAGRAPH_STYLE_ITEMDIGIT
} GtkHTMLParagraphStyle;

typedef enum {
	GTK_HTML_STREAM_OK,
	GTK_HTML_STREAM_ERROR
} GtkHTMLStreamStatus;

#endif
