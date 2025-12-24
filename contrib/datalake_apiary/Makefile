top_builddir = ../..
include $(top_builddir)/src/Makefile.global

# Avoid building LLVM Bitcode for datalake_apiary module.
with_llvm = no

all:
	$(MAKE) -C hive_auto_sync all
	$(MAKE) -C hive_sync_bgworker all

install:
	$(MAKE) -C hive_auto_sync $@
	$(MAKE) -C hive_sync_bgworker $@

clean:
	$(MAKE) -C hive_auto_sync $@
	$(MAKE) -C hive_sync_bgworker $@

uninstall:
	$(MAKE) -C hive_auto_sync $@
	$(MAKE) -C hive_sync_bgworker $@
