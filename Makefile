LDLIBS+= -lreadline -pthread -lcrypto -lssl
DBGFLAGS= -ggdb3 -g -DDEBUG
OBJ = src/wb.o

all: wb

wb: $(OBJ)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

wbs: CFLAGS+= -DSTAT_BOT
wbs: $(OBJ)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

debug-wb : CFLAGS+= $(DBGFLAGS)
debug-wb : clean wb
debug-wbs: CFLAGS+= $(DBGFLAGS)
debug-wbs: clean wbs

clean:
	$(RM) wb wbs $(OBJ)

