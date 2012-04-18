OUT = Qspreal

ifndef BUILD
#BUILD=arm_gnueabi
BUILD=emu
endif

include /usr/local/pocketbook/common.mk

CFLAGS+=-D_POCKETBOOK `freetype-config --cflags` -I../ 
CXXFLAGS+=-D_POCKETBOOK `freetype-config --cflags` -I../

LDFLAGS+= -L../pbtk/obj_$(BUILD)
LIBS+=-lpbtk -lpbtk_img 
#-lWildMidi

ifeq ($(BUILD),arm)
CXXFLAGS+=-I/usr/local/pocketbook/arm-linux/include\
  -I/usr/local/pocketbook/arm-linux/include/sigc++-2.0
LDFLAGS+=-L/usr/local/pocketbook/arm-linux/lib -lungif -lhtmlcxx -lpng -lsigc-2.0
endif

ifeq ($(BUILD),arm_gnueabi)
CXXFLAGS+=-I/usr/local/pocketbook_eabi/include -DHAS_NO_IV_GET_DEFAULT_FONT\
  -I/usr/local/pocketbook_eabi/include/sigc++-2.0 -DHAVE_SOUND=1 -D_NEW_DEV_
LDFLAGS+=-L/usr/local/pocketbook_eabi/lib -lungif -lhtmlcxx -lsigc-2.0
endif

ifeq ($(BUILD),emu)
CXXFLAGS+=-g `pkg-config --cflags sigc++-2.0` -DHTMLDEBUG  -DHAVE_SOUND=1 -DNETBOOK=1
LDFLAGS+=-g -lgif -lhtmlcxx -lsigc-2.0
endif

SOURCES_CPP = \
    src/helper.cpp\
    src/main.cpp\
    src/qspcallbacks.cpp\
    src/qspsound.cpp\
    src/qspthread.cpp \
    src/screens.cpp
#    src/pbwildmidi.cpp\
#    src/qspsnd.cpp\
#    src/pbsound.cpp\

SOURCES_CXX =

SOURCES_C = \
    src/qsp/actions.c\
    src/qsp/callbacks.c\
    src/qsp/codetools.c\
    src/qsp/coding.c\
    src/qsp/common.c\
    src/qsp/errors.c\
    src/qsp/game.c\
    src/qsp/locations.c\
    src/qsp/mathops.c\
    src/qsp/memwatch.c\
    src/qsp/menu.c\
    src/qsp/objects.c\
    src/qsp/playlist.c\
    src/qsp/statements.c\
    src/qsp/text.c\
    src/qsp/time.c\
    src/qsp/variables.c\
    src/qsp/variant.c\
    src/qsp/bindings/pocketbook/pocketbook_callbacks.c\
    src/qsp/bindings/pocketbook/default_coding.c\
    src/qsp/bindings/pocketbook/default_control.c\
    src/qsp/onig/regcomp.c\
    src/qsp/onig/regenc.c\
    src/qsp/onig/regerror.c\
    src/qsp/onig/regexec.c\
    src/qsp/onig/regparse.c\
    src/qsp/onig/regsyntax.c\
    src/qsp/onig/regtrav.c\
    src/qsp/onig/regversion.c\
    src/qsp/onig/st.c\
    src/qsp/onig/enc/ascii.c\
    src/qsp/onig/enc/cp1251.c\
    src/qsp/onig/enc/koi8_r.c \
    src/qsp/onig/enc/unicode.c\
    src/qsp/onig/enc/utf16_le.c\
    src/qsp/onig/enc/utf32_le.c

CDEPS = -MT$@ -MF`echo $@ | sed -e 's,\.o$$,.d,'` -MD -MP

OBJS  = $(addprefix $(OBJDIR)/,$(SOURCES_C:.c=.o)) \
        $(addprefix $(OBJDIR)/,$(SOURCES_CXX:.cxx=.o))\
        $(addprefix $(OBJDIR)/,$(SOURCES_CPP:.cpp=.o))

$(OBJDIR):
	mkdir -p $(OBJDIR)/src/qsp/onig/enc
	mkdir -p $(OBJDIR)/src/qsp/bindings/pocketbook

#$(OBJDIR)/%.o: src/%.c
#	$(CC) -c -o $@ $(CFLAGS) $(INCLUDES) $(CDEPS) $<

$(OBJDIR)/src/qsp/%.o: src/qsp/%.c
	$(CC) -c -o $@ $(CFLAGS) $(INCLUDES) $(CDEPS) $<

$(OBJDIR)/%.o: images/%.c
	$(CC) -c -o $@ $(CFLAGS) $(INCLUDES) $(CDEPS) $<

$(OBJDIR)/src/%.o: src/%.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $(INCLUDES) $(CDEPS) $<

$(OBJDIR)/src/%.o: src/%.cxx
	$(CXX) -c -o $@ $(CXXFLAGS) $(INCLUDES) $(CDEPS) $<

$(PROJECT) : $(OBJDIR) $(OBJS)
	$(LD) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

# Dependencies tracking:
-include $(OBJDIR)/*.d
-include $(OBJDIR)/src/*.d
-include $(OBJDIR)/src/qsp/*.d
-include $(OBJDIR)/src/qsp/bindings/pocketbook/*.d
