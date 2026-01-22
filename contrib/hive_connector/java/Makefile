top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

MVN_EXISTS := $(shell command -v mvn 2> /dev/null)

all:
	mvn package

clean:
ifdef MVN_EXISTS
	mvn clean
endif

install: all
	$(MKDIR_P) $(pkglibdir)/java/
	$(INSTALL_DATA) 'target/libhms-1.0.0-jar-with-dependencies.jar' $(pkglibdir)/java/

uninstall:
	rm -f $(pkglibdir)/java/libhms-1.0.0-jar-with-dependencies.jar

distclean: clean
