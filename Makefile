CC = cl
AR = lib
LD = link
RC = rc
RM = del

WHOIS_EXE = whois.exe
WHOIS_RES = whois.res
WHOIS_OBJ = main.obj memory.obj array.obj xmldb.obj socks.obj whois.obj

LIBXML_LIB = libxml2.lib
LIBXML_OBJ = buf.obj c14n.obj catalog.obj chvalid.obj debugXML.obj dict.obj DOCBparser.obj encoding.obj entities.obj error.obj globals.obj \
	hash.obj HTMLparser.obj HTMLtree.obj legacy.obj list.obj parser.obj parserInternals.obj pattern.obj relaxng.obj SAX.obj SAX2.obj \
	schematron.obj threads.obj tree.obj uri.obj valid.obj xinclude.obj xlink.obj xmlIO.obj xmlmemory.obj xmlmodule.obj xmlreader.obj \
	xmlregexp.obj xmlsave.obj xmlschemas.obj xmlschemastypes.obj xmlstring.obj xmlunicode.obj xmlwriter.obj xpath.obj xpointer.obj \
	trio.obj trionan.obj triostr.obj

CFLAGS = /c /nologo /MD /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_MBCS" /D "LIBXML_STATIC" /I "config" /I "libxml2/include"
CFLAGS_LIBXML = $(CFLAGS) /D "_REENTRANT" /D "HAVE_WIN32_THREADS" /D "NOLIBTOOL"
CFLAGS_WHOIS = $(CFLAGS) /I "whois"

LDFLAGS = /nologo /release /version:1.0 /out:$(WHOIS_EXE)
ARFLAGS = /nologo /out:$(LIBXML_LIB)
RCFLAGS = /nologo /dAPSTUDIO_INVOKED

all:	$(WHOIS_OBJ) $(WHOIS_RES) $(LIBXML_LIB)
	$(LD) $(LDFLAGS) $**
	-@$(RM) *.obj

$(LIBXML_LIB): $(LIBXML_OBJ)
	$(AR) $(ARFLAGS) $**

{libxml2}.c.obj::
	$(CC) $(CFLAGS_LIBXML) $<

{whois}.c.obj::
	$(CC) $(CFLAGS_WHOIS) $<

{projects}.rc.res::
	$(RC) $(RCFLAGS) $<

clean:
	-$(RM) *.lib
	-$(RM) *.obj
	-$(RM) *.res
	-$(RM) *.exe
