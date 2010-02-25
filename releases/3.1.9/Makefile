# 
#
# This file is copyrighted under the GNU Public License.
# See http://www.gnu.org/copyleft/gpl.html for further
# details.
#
# Makefile for "deal", a bridge hand generator.
#

TCL_DIR=/usr

include Make.mac-osx
#include Make.ubuntu

CC=gcc


# Since strdup is not implemented on some Ultrix systems
# (and is not required by ANSI C) you might want to
# uncomment the following line:
#
# EXTRAS_OBJS= util.o
#
EXTRA_OBJS= 

# On system which don't have "random" and "srandom",
# you might have to use the horrible "rand" for random
# number generation
#
#EXTRA_CFLAGS = -Drandom=rand -Dsrandom=srand
#
#EXTRA_CFLAGS = 
#EXTRA_CFLAGS = -DUSE_RAND48

COMPILE.c= $(CC) $(CFLAGS) -c

CFLAGS= $(DEBUG_FLAGS) -I$(TCL_INCL) $(EXTRA_CFLAGS)


OBJS=random.o additive.o hand.o deal.o formats.o tcl_deal.o maindeal.o \
	stat.o counttable.o vector.o dist.o stringbox.o dealtypes.o \
	keywords.o holdings.o tcl_dds.o dds.o ddsLookup.o \
	$(EXTRA_OBJS) 
SRCS=additive.c hand.c deal.c formats.c tcl_deal.c dist.c vector.c stat.c counttable.c stringbox.c dealtypes.c holdings.c keywords.c maindeal.c random.c dds.cpp ddsLookup.cpp getopt.c
SRCKIT=additive.c hand.c deal.c formats.c tcl_deal.c dist.c vector.c stat.c makecounttable.c stringbox.c dealtypes.c holdings.c keywords.c maindeal.c random.c tcl_dds.c dds.cpp ddsLookup.cpp
HFILES=deck.h deal.h tcl_incl.h vector.h stat.h tcl_dist.h dist.h formats.h additive.h stringbox.h dealtypes.h holdings.h keywords.h ansidecl.h dds.h ddsInline.h ddsInterface.h Holding.h getopt.h ddsLookup.h
EXAMPLES= ex/*.tcl
TESTS=tests
HTML=html
BUILDFILES=Makefile Make.dep Make.mac-osx Make.ubuntu
OTHERFILES=CHANGES LICENSE GPL input format lib deal.tcl

SOURCEKIT=$(SRCKIT) $(HFILES) $(EXAMPLES) $(BUILDFILES) $(OTHERFILES) $(EXTRAS) $(TESTS) $(HTML)
BINKIT=$(EXAMPLES) $(OTHERFILES) $(TESTS) $(HTML)
UUKIT=$(EXAMPLES) $(OTHERFILES) deal
BINARY=./deal

deal: $(OBJS)
	g++ $(CFLAGS) $(OBJS) -o deal $(LDFLAGS)

universal:
	$(MAKE) clean
	$(MAKE) MAC_ARCH="$(MAC_ARCH_UNIVERSAL)"

deal.cgi:
	make clean
	make CFLAGS="$(CFLAGS) -DCGI"

vector.so: vector.c
	$(CC) -c $(CFLAGS) -PIC vector.c
	ld vector.o -o vector.so

dist.so: dist.c
	$(CC) -c $(CFLAGS) -PIC dist.c
	ld dist.o -o dist.so

stringbox.so: stringbox.c
	$(CC) -c $(CFLAGS) -PIC stringbox.c
	ld stringbox.o -o stringbox.so

$(SRCS): deal.h

counttable.c: makecounttable
	./makecounttable > counttable.c

KITNAME=deal
DEAL_VERSION=319
OLD_VERSION=318
RELEASE=$(KITNAME)$(DEAL_VERSION)
SRCDIR=$(RELEASE)
BINDIR=$(RELEASE)-bin
SRCZIP=$(SRCDIR).zip
EXEZIP=$(SRCDIR)win.zip
DOCZIP=$(SRCDIR)doc.zip
SRCTAR=$(SRCDIR).tar
SRCGZIP=$(SRCTAR).gz
BINZIP=$(KITNAME)exe.zip
DMG=$(BINDIR).dmg
DIFFZIP=deal$(DEAL_VERSION)diff.zip
OLDZIP=../deal/deal$(OLD_VERSION).zip
OLDDIR=$(KITNAME)$(OLD_VERSION)
DOCDEST=html
DOCTYPE=release

SMALLTESTCOUNT=10

allzip: zip dmg

CHANGES: docs/html/CHANGES.txt
	cp docs/html/CHANGES.txt CHANGES

zip: $(SRCZIP) 

dmg: $(DMG)

xzip: $(EXEZIP)

dzip: $(DOCZIP)

documentation:
	rm -rf $(DOCDEST)
	mkdir $(DOCDEST)
	cp -r docs/graphics $(DOCDEST)/graphics
	cp -r docs/html/*.css docs/html/ex $(DOCDEST)
	cd docs/html; for file in *.html *.js; do \
		php process.php $(DOCTYPE) $$file > ../../$(DOCDEST)/$$file ; done

sitedoc:
	$(MAKE) DOCDEST=site DOCTYPE=site documentation

newsite:
	$(MAKE) clean
	$(MAKE) universal
	$(MAKE) dmg
	cp $(DMG) site
	$(MAKE) zip
	cp $(SRCZIP) site
	$(MAKE) xzip
	cp $(EXEZIP) site
	

$(BINDIR): $(BINKIT) sitedoc documentation
	rm -rf $(BINDIR)
	mkdir $(BINDIR)
	/bin/ls -1d $(BINKIT) | xargs tar cf - | (cd $(BINDIR) ; tar xf -)
	find $(BINDIR) -name .svn -print | xargs /bin/rm -rf

$(SRCDIR): $(SOURCEKIT) docs/html docs/graphics documentation
	mv Make.dep Make.dep.saved
	touch Make.dep
	rm -rf $(SRCDIR)
	mkdir $(SRCDIR)
	/bin/ls -1d $(SOURCEKIT) | xargs tar cf - | (cd $(SRCDIR) ; tar xf -)
	mv Make.dep.saved Make.dep

$(SRCZIP): $(SRCDIR)
	zip -r $(SRCZIP) $(SRCDIR) -x \*~ -x *CVS/\* *.svn/\*

$(EXEZIP): $(SRCDIR)
	rm -f $(EXEZIP) $(SRCDIR)/deal
	test -f deal.exe 
	test -f tcl85.dll
	cp deal.exe $(SRCDIR)
	cp tcl85.dll $(SRCDIR)
	zip -r $(EXEZIP) $(SRCDIR)/ex $(SRCDIR)/input $(SRCDIR)/format $(SRCDIR)/docs $(SRCDIR)/CHANGES $(SRCDIR)/LICENSE $(SRCDIR)/GPL $(SRCDIR)/lib $(SRCDIR)/deal.tcl $(SRCDIR)/deal.exe $(SRCDIR)/tcl85.dll $(SRCDIR)/tests $(SRCDIR)/html -x \*~ -x *CVS/\* *.svn/\*

$(DMG): $(BINDIR) deal
	cp deal $(BINDIR)/deal
	/bin/rm -rf dmg $(DMG)
	mkdir dmg
	cp -r $(BINDIR) dmg/$(SRCDIR)
	hdiutil create -srcfolder dmg -volname "Deal $(DEAL_VERSION)" $(DMG)

$(DOCZIP): $(SRCDIR)
	rm -f $(DOCZIP)
	zip -r $(DOCZIP) $(SRCDIR)/CHANGES $(SRCDIR)/LICENSE $(SRCDIR)/GPL $(SRCDIR)/docs -x \*~ -x *CVS/\* *.svn/\*

gzip: $(SRCTAR).gz

$(SRCTAR).gz: $(SRCDIR)
	rm -f $(SRCTAR).gz
	tar cf $(SRCTAR) $(SRCDIR)
	gzip $(SRCTAR)

$(BINZIP): deal
	strip deal
	ls -1 $(UUKIT) | grep -v /RCS | xargs zip $(BINZIP)
	#zip $(BINZIP) $(UUKIT)

diffzip: $(DIFFZIP)

$(DIFFZIP): $(SRCZIP)
	rm -rf tempdir $(DIFFZIP)
	mkdir tempdir
	cd tempdir ; unzip ../$(SRCZIP) ; unzip ../$(OLDZIP)
	cd tempdir ; diff -r $(OLDDIR) $(SRCDIR) > ../diff.out || echo Done
	zip $(DIFFZIP) diff.out

tar:
	mv Make.dep Make.dep.saved
	touch Make.dep
	tar cvf deal.tar $(SOURCEKIT)
	mv Make.dep.saved Make.dep

test: ./deal
	$(BINARY) -I "line tests/input/sample.line" -i format/ddline 100 > test.out
	if cmp test.out tests/output/sample.ddline ; then echo PASS; else echo FAIL  ; fi

smalltest: ./deal
	$(BINARY) -I "line tests/input/sample.line" -i format/ddline $(SMALLTESTCOUNT) > test.out
	head -$(SMALLTESTCOUNT) tests/output/sample.ddline > correct.out
	diff test.out correct.out
	if cmp test.out correct.out ; then echo PASS; else echo FAIL  ; fi

great88: ./deal
	$(BINARY) -x tests/great88 | fgrep FAIL || echo "PASSED"

html: documentation
ftp: $(SRCZIP)
	cp $(SRCZIP) $(FTP)

depends:
	$(CC) $(CFLAGS) -M *.c *.cpp >Make.dep

clean:
	rm -rf deal $(OBJS) $(SRCDIR) $(SRCZIP) $(SRCGZIP) $(DOCZIP) $(DMG) $(EXEZIP) $(BINDIR) counttable.c makecounttable makecounttable.o html site

include Make.dep
