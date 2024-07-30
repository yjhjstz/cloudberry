# gpcontrib/datalake_proxy/Makefile

MODULE_big = hive_auto_sync
OBJS = \
	$(WIN32RES) \
	hive_auto_sync.o

EXTENSION = hive_auto_sync

PGFILEDESC = "hive_auto_sync - Hive Auto Sync"

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/datalake_apiary/hive_sync_bgworker
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif