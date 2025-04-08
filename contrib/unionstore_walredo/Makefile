MODULE_big = unionstore_walredo
OBJS = \
	$(WIN32RES) \
	inmem_smgr.o \
	walredoproc.o

PGFILEDESC = "unionstore_walredo - helper process that runs in unionstore pageserver"

ifdef USE_PGXS
  PG_CONFIG = pg_config
  PGXS := $(shell $(PG_CONFIG) --pgxs)
  include $(PGXS)
else
  subdir = contrib/unionstore_walredo
  top_builddir = ../..

  include $(top_builddir)/src/Makefile.global

  ifeq ($(with_libseccomp),yes)
    OBJS += seccomp.o
    SHLIB_LINK += -lseccomp
  endif

  include $(top_srcdir)/contrib/contrib-global.mk
endif
