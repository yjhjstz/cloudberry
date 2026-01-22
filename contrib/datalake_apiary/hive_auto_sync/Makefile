top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

MVN_EXISTS := $(shell command -v mvn 2> /dev/null)

with_llvm = no

all:
	mvn package

clean:
ifdef MVN_EXISTS
	mvn clean
endif

install: all
	$(MKDIR_P) $(pkglibdir)/java/
	$(INSTALL_DATA) 'apiary-metastore-events/kafka-metastore-events/kafka-metastore-listener/target/kafka-metastore-listener-7.3.8-all.jar' $(pkglibdir)/java/
	$(INSTALL_DATA) 'hivesync/target/hivesync-1.0.0.jar' $(pkglibdir)/java/


uninstall:
	rm -f './kafka-metastore-listener-7.3.8-all.jar'
	rm -f './hivesync-1.0.0-all.jar'

distclean: clean
