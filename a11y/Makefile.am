END=

INCLUDES = -I$(top_srcdir)/src $(GTKHTML_CFLAGS)

lib_LTLIBRARIES = libgtkhtml-a11y.la

libgtkhtml_a11y_la_LDFLAGS = -release $(GTKHTML_RELEASE) -version-info $(GTKHTML_CURRENT):$(GTKHTML_REVISION):$(GTKHTML_AGE)
libgtkhtml_a11y_la_LIBADD = $(GTKHTML_LIBS)

libgtkhtml_a11y_la_SOURCES =				\
	factory.h					\
	factory.c					\
	html.h						\
	html.c						\
	object.h					\
	object.c					\
	paragraph.h					\
	paragraph.c					\
	utils.h						\
	utils.c						\
	$(END)
