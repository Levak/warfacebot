CFLAGS+= -Wall -Wextra -Wno-unused-parameter
LDLIBS+= -lcrypto -lssl -L=
LDLIBS_DEBUG+= -lreadline
DBGFLAGS= -ggdb3 -g -DDEBUG
OBJ = src/wb.o

$(info $(OSTYPE))
ifeq ($(OSTYPE), cygwin)
LDLIBS+= -lpthread
else
ifneq ($(OSTYPE),)
LDLIBS+= -pthread
else
LDLIBS+= -lpthread -lws2_32
endif
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

