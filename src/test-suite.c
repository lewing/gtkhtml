#include <string.h>
#include <stdio.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkwindow.h>
#include "gtkhtml.h"
#include "htmlengine.h"
#include "htmlengine-edit-movement.h"

typedef struct {
	char *name;
	int (*test_function) (GtkHTML *html);
} Test;

static int test_cursor_beol (GtkHTML *html);

static Test tests[] = {
	{ "cursor begin/end of line", test_cursor_beol },
	{ NULL, NULL }
};

static void load_editable (GtkHTML *html, char *s)
{
	gtk_html_set_editable (html, FALSE);
	gtk_html_load_from_string (html, s, -1);
	gtk_html_set_editable (html, TRUE);
}

static int test_cursor_beol (GtkHTML *html)
{
	load_editable (html, "<pre>simple line\nsecond line\n");

	/* first test it on 1st line */
	printf ("test_cursor_beol: testing 1st line eol\n");
	if (!html_engine_end_of_line (html->engine)
	    || html->engine->cursor->offset != html->engine->cursor->position
	    || html->engine->cursor->offset != 11)
		return FALSE;

	printf ("test_cursor_beol: testing 1st line bol\n");
	if (!html_engine_beginning_of_line (html->engine)
	    || html->engine->cursor->offset != html->engine->cursor->position
	    || html->engine->cursor->offset != 0)
		return FALSE;

	/* move to 2nd and test again */
	printf ("test_cursor_beol: testing 2nd line\n");
	if (!html_engine_move_cursor (html->engine, HTML_ENGINE_CURSOR_DOWN, 1))
		return FALSE;

	printf ("test_cursor_beol: testing 2nd line eol\n");
	if (!html_engine_end_of_line (html->engine)
	    || html->engine->cursor->offset != 11
	    || html->engine->cursor->position != 23)
		return FALSE;

	printf ("test_cursor_beol: testing 2nd line bol\n");
	if (!html_engine_beginning_of_line (html->engine)
	    || html->engine->cursor->offset != 0
	    || html->engine->cursor->position != 12)
		return FALSE;

	printf ("test_cursor_beol: passed\n");

	return TRUE;
}

int main (int argc, char *argv[])
{
	GtkWidget *win, *html_widget;
	GtkHTML *html;
	int i = 0, n_all, n_successful;

	gtk_init (&argc, &argv);

	html_widget = gtk_html_new ();
	html = GTK_HTML (html_widget);
	gtk_html_load_empty (html);
	gtk_html_set_editable (html, TRUE);
	win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_add (GTK_CONTAINER (win), html_widget);
	gtk_widget_show_all (win);
	gtk_widget_show_now (win);

	n_all = n_successful = 0;

	fprintf (stderr, "GtkHTML test suite\n");
	fprintf (stderr, "--------------------------------------------------------------------------------\n");
	for (i = 0; tests [i].name; i ++) {
		int j, result;

		fprintf (stderr, "running test '%s'", tests [i].name);
		for (j = strlen (tests [i].name); j < 58; j ++)
			fputc ('.', stderr);
		result = (*tests [i].test_function) (html);
		fprintf (stderr, " %s\n", result ? "OK" : "Failed");

		n_all ++;
		if (result)
			n_successful ++;
	}

	fprintf (stderr, "--------------------------------------------------------------------------------\n");
	fprintf (stderr, "%d tests failed %d tests passed (of %d tests)\n", n_all - n_successful, n_successful, n_all);

	return n_all != n_successful;
}
