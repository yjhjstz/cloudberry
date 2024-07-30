top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

all:
	mvn package

clean:
	mvn clean

install: all
	$(MKDIR_P) $(pkglibdir)/java/
	$(INSTALL_DATA) 'apiary-metastore-events/kafka-metastore-events/kafka-metastore-listener/target/kafka-metastore-listener-7.3.8-all.jar' $(pkglibdir)/java/
	$(INSTALL_DATA) 'hivesync/target/hivesync-1.0.0.jar' $(pkglibdir)/java/


uninstall:
	rm -f './kafka-metastore-listener-7.3.8-all.jar'
	rm -f './hivesync-1.0.0-all.jar'

distclean:
	mvn clean
