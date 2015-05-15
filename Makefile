CFLAGS+= -Wall -Wextra -Wno-unused-parameter
CFLAGS+= -Iinclude -DZLIB
LDLIBS+= -lz -L=
LDLIBS_DEBUG+= -lreadline
DBGFLAGS= -ggdb3 -g -DDEBUG
OBJ = 	\
./src/session.o\
./src/stream/connect.o\
./src/stream/recv.o\
./src/stream/send_format.o\
./src/stream/send.o\
./src/tools/base64.o\
./src/tools/get_info.o\
./src/tools/zlibb64.o\
./src/wb.o\
./src/xmpp/close.o\
./src/xmpp/connect.o\
./src/xmpp/id_handler.o\
./src/xmpp/query_handler.o\
./src/xmpp/sasl.o\
./src/xmpp/tools.o\
./src/xmpp_wf/get/xmpp_bind.o\
./src/xmpp_wf/get/xmpp_iq_account.o\
./src/xmpp_wf/get/xmpp_iq_create_profile.o\
./src/xmpp_wf/get/xmpp_iq_gameroom_leave.o\
./src/xmpp_wf/get/xmpp_iq_get_account_profiles.o\
./src/xmpp_wf/get/xmpp_iq_get_master_server.o\
./src/xmpp_wf/get/xmpp_iq_join_channel.o\
./src/xmpp_wf/get/xmpp_iq_peer_status_update.o\
./src/xmpp_wf/get/xmpp_iq_player_status.o\
./src/xmpp_wf/get/xmpp_iq_session.o\
./src/xmpp_wf/get/xmpp_print_number_of_occupants.o\
./src/xmpp_wf/get/xmpp_promote_room_master.o\
./src/xmpp_wf/get/xmpp_send_message.o\
./src/xmpp_wf/result/xmpp_iq_follow_send.o\
./src/xmpp_wf/result/xmpp_iq_friend_list.o\
./src/xmpp_wf/result/xmpp_iq_gameroom_sync.o\
./src/xmpp_wf/result/xmpp_iq_invitation_request.o\
./src/xmpp_wf/result/xmpp_iq_peer_status_update.o\
./src/xmpp_wf/result/xmpp_iq_ping.o\
./src/xmpp_wf/result/xmpp_iq_sync_notifications.o\
./src/xmpp_wf/result/xmpp_message.o\
./src/xmpp_wf/tools.o\

OSTYPE = $(shell uname -s)
$(info $(OSTYPE))
ifeq ($(OSTYPE),Cygwin)
LDLIBS+= -lpthread
else ifeq ($(OSTYPE),Linux)
LDLIBS+= -pthread
CFLAGS+= -pthread
else
LDLIBS+= -lpthread -lws2_32
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

