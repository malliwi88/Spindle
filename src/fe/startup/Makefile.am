bin_PROGRAMS = spindle

AM_CPPFLAGS = -I$(top_srcdir)/../logging

spindle_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/../include -I$(top_srcdir)/comlib -I$(top_srcdir)/../server/cache -I$(top_srcdir)/../server/comlib -I$(top_srcdir)/../utils -I$(top_srcdir)/../cobo -DBINDIR=\"$(pkglibexecdir)\" -DLIBEXECDIR=\"$(pkglibexecdir)\"
spindle_SOURCES = spindle_fe_main.cc spindle_fe_serial.cc spindle_fe.cc parse_launcher.cc parse_launcher_args.cc parseargs.cc parse_preload.cc $(top_srcdir)/../utils/pathfn.c $(top_srcdir)/../utils/keyfile.c $(top_srcdir)/../utils/parseloc.c

spindle_LDADD = $(top_builddir)/logging/libspindlelogc.la $(top_builddir)/openmpi_intercept/libparseompi.la $(top_builddir)/hostbin/libhostbin.la

if LMON
spindle_LDADD += $(top_builddir)/launchmon/libfelmon.la
endif
if COBO
spindle_LDADD += $(top_builddir)/comlib/libfe_cobo.la $(top_builddir)/cobo/libldcs_cobo.la
endif
if MSOCKET
spindle_LDADD += $(top_builddir)/comlib/libfe_msocket.la
endif
spindle_LDADD += $(MUNGE_LIBS) $(GCRYPT_LIBS)
