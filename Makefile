LDLIBS+= -lcrypto -lssl
LDLIBS_DEBUG+= -lreadline
DBGFLAGS= -ggdb3 -g -DDEBUG
OBJ = src/wb.o

ifeq ($(OSTYPE), cygwin)
LDLIBS+= -lpthread
else
LDLIBS+= -pthread
endif

all: wb

wb: $(OBJ)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

wbs: CFLAGS+= -DSTAT_BOT
wbs: $(OBJ)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

debug-wb : CFLAGS+= $(DBGFLAGS)
debug-wb : LDLIBS+= $(LDLIBS_DEBUG)
debug-wb : wb

debug-wbs: CFLAGS+= $(DBGFLAGS)
debug-wbs: LDLIBS+= $(LDLIBS_DEBUG)
debug-wbs: wbs

clean:
	$(RM) wb wbs $(OBJ)

