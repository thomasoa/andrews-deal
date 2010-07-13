AUTOMAKE_OPTIONS = foreign

CFLAGS=-Wall -I/usr/local/include -DVERSION="\"$(VERSION)\"" -DPROG="\"$(PACKAGE)\""
LDFLAGS=-L/usr/local/lib -ltcl

bin_PROGRAMS = deal
deal_SOURCES = additive.c counttable.c deal.c dealtypes.c dist.c formats.c getopt.c hand.c holdings.c keywords.c maindeal.c random.c stat.c stringbox.c tcl_dds.c tcl_deal.c vector.c dds.cpp ddsLookup.cpp
EXTRA_deal_SOURCES = makecounttable.c

counttable.c: makecounttable
	./makecounttable > counttable.c
