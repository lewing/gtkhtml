END=

INCLUDES = -I$(top_srcdir)/src $(GTKHTML_CFLAGS)

CFLAGS += $(GAIL_CFLAGS)

lib_LTLIBRARIES = libgtkhtml-a11y.la

libgtkhtml_a11y_la_LDFLAGS = -release $(GTKHTML_RELEASE) -version-info $(GTKHTML_CURRENT):$(GTKHTML_REVISION):$(GTKHTML_AGE)
libgtkhtml_a11y_la_LIBADD = $(GTKHTML_LIBS) $(GAIL_LIBS)

libgtkhtml_a11y_la_SOURCES =				\
	cell.h						\
	cell.c						\
	factory.h					\
	factory.c					\
	html.h						\
	html.c						\
	image.h						\
	image.c						\
	object.h					\
	object.c					\
	paragraph.h					\
	paragraph.c					\
	table.h						\
	table.c						\
	text.h						\
	text.c						\
	utils.h						\
	utils.c						\
	$(END)
