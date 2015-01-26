QMAKE=qmake # Set this for a non-default QMake version
PREFIX=/usr/bin/ # Set this for different install dir
BUILDDIR=bin/ # Set this for different build dir
TESTDIR=testbin/ # Set this for different testing dir
DOXY=doxygen # Set this for non-default doxygen version

TARGET=ShaderSandbox
TESTTARGET=ShaderSandboxTest
SOURCES=$(wildcard src/*.c)
TESTS=$(filter-out src/main.c, $(wildcard test/*.c src/*c))

# Makes everything
all: testing app

# makes the tests
testing:
	mkdir -p $(TESTDIR) $(BUILDDIR)
	cd test && ${QMAKE} $(TESTARGET).pro -o ../$(TESTDIR)Makefile
	cd $(TESTDIR) && make && rm *.o *[ch]pp

# Makes the app
app:
	mkdir -p $(TESTDIR) $(BUILDDIR)
	cd src && ${QMAKE} $(TARGET).pro -o ../$(BUILDDIR)Makefile
	cd $(BUILDDIR) && make && rm *.o *[ch]pp

# Makes the documentation
doc:
	$(DOXY) configfile

# Removes everything previously built
clean:
	rm -r $(TESTDIR) $(BUILDDIR)

# Installs the app
install:
	install -d $(PREFIX)$(TARGET)
	install $(BUILDDIR)$(TARGET) $(PREFIX)$(TARGET)

# Uninstalls the app
uninstall:
	rm -rf $(PREFIX)$(TARGET)
