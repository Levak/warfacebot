CC=gcc
CFLAGS+= -g -ggdb3 -Wall -Wextra -Wno-unused-parameter
CFLAGS+= -Iinclude -Ithird_party -DZLIB
LDLIBS+= -lz -lreadline -L=
LDLIBS_DEBUG+=
DBGFLAGS= -ggdb3 -g -DDEBUG

CFLAGS+= -DUSE_TLS
LDLIBS+= -lssl -lcrypto

OSTYPE?= $(shell uname -s | tr '[:upper:]' '[:lower:]')

ifneq (,$(findstring cygwin,$(OSTYPE)))       # CYGWIN
LDLIBS+= -lpthread
else ifneq (,$(findstring mingw,$(OSTYPE)))   # MINGW
LDLIBS+= -lpthread -lws2_32
else ifneq (,$(findstring linux,$(OSTYPE)))   # LINUX
LDLIBS+= -pthread
CFLAGS+= -pthread
endif

VALGRIND_API?= $(shell echo '\#include <valgrind/memcheck.h>' | $(CC) -E -x c -> /dev/null 2> /dev/null && echo '-DVALGRIND_API' || echo ' ')

CFLAGS+= $(VALGRIND_API)

OBJ = \
./src/clanmate.o \
./src/cmd/cmd_add_friend.o \
./src/cmd/cmd_change.o \
./src/cmd/cmd_channel.o \
./src/cmd/cmd_follow.o \
./src/cmd/cmd_invite.o \
./src/cmd/cmd_leave.o \
./src/cmd/cmd_master.o \
./src/cmd/cmd_missions.o \
./src/cmd/cmd_name.o \
./src/cmd/cmd_open.o \
./src/cmd/cmd_ready.o \
./src/cmd/cmd_remove_friend.o \
./src/cmd/cmd_safe.o \
./src/cmd/cmd_say.o \
./src/cmd/cmd_start.o \
./src/cmd/cmd_switch.o \
./src/cmd/cmd_whisper.o \
./src/cmd/cmd_whois.o \
./src/friend.o \
./src/game.o \
./src/mission.o \
./src/pvp_maps.o \
./src/session.o \
./src/stream/connect.o \
./src/stream/crypt.o \
./src/stream/recv.o \
./src/stream/send_format.o \
./src/stream/send.o \
./src/stream/tls.o \
./third_party/stub-strtok_r.o \
./third_party/wake.o \
./src/tools/base64.o \
./src/tools/geoip.o \
./src/tools/get_info.o \
./src/tools/list.o \
./src/tools/zlibb64.o \
./src/wb.o \
./src/xml/serializer.o \
./src/xmpp/get/xmpp_bind.o \
./src/xmpp/get/xmpp_iq_ping.o \
./src/xmpp/get/xmpp_iq_session.o \
./src/xmpp/get/xmpp_presence.o \
./src/xmpp/get/xmpp_room_message.o \
./src/xmpp/get/xmpp_sasl.o \
./src/xmpp/get/xmpp_starttls.o \
./src/xmpp/get/xmpp_stream.o \
./src/xmpp/id_handler.o \
./src/xmpp/query_handler.o \
./src/xmpp/result/xmpp_iq_ping.o \
./src/xmpp/room.o \
./src/xmpp/sasl.o \
./src/xmpp/tools.o \
./src/xmpp_wf/close.o \
./src/xmpp_wf/connect.o \
./src/xmpp_wf/get/xmpp_iq_account.o \
./src/xmpp_wf/get/xmpp_iq_confirm_notification.o \
./src/xmpp_wf/get/xmpp_iq_create_profile.o \
./src/xmpp_wf/get/xmpp_iq_follow_send.o \
./src/xmpp_wf/get/xmpp_iq_gameroom_askserver.o \
./src/xmpp_wf/get/xmpp_iq_gameroom_join.o \
./src/xmpp_wf/get/xmpp_iq_gameroom_kick.o \
./src/xmpp_wf/get/xmpp_iq_gameroom_leave.o \
./src/xmpp_wf/get/xmpp_iq_gameroom_open.o \
./src/xmpp_wf/get/xmpp_iq_gameroom_setinfo.o \
./src/xmpp_wf/get/xmpp_iq_gameroom_setname.o \
./src/xmpp_wf/get/xmpp_iq_gameroom_setplayer.o \
./src/xmpp_wf/get/xmpp_iq_gameroom_update_pvp.o \
./src/xmpp_wf/get/xmpp_iq_get_account_profiles.o \
./src/xmpp_wf/get/xmpp_iq_get_master_server.o \
./src/xmpp_wf/get/xmpp_iq_invitation_send.o \
./src/xmpp_wf/get/xmpp_iq_join_channel.o \
./src/xmpp_wf/get/xmpp_iq_missions_get_list.o \
./src/xmpp_wf/get/xmpp_iq_peer_clan_member_update.o \
./src/xmpp_wf/get/xmpp_iq_peer_player_info.o \
./src/xmpp_wf/get/xmpp_iq_peer_status_update.o \
./src/xmpp_wf/get/xmpp_iq_player_status.o \
./src/xmpp_wf/get/xmpp_iq_profile_info_get_status.o \
./src/xmpp_wf/get/xmpp_iq_remove_friend.o \
./src/xmpp_wf/get/xmpp_iq_send_invitation.o \
./src/xmpp_wf/get/xmpp_print_number_of_occupants.o \
./src/xmpp_wf/get/xmpp_promote_room_master.o \
./src/xmpp_wf/get/xmpp_send_message.o \
./src/xmpp_wf/result/xmpp_iq_clan_info.o \
./src/xmpp_wf/result/xmpp_iq_follow_send.o \
./src/xmpp_wf/result/xmpp_iq_friend_list.o \
./src/xmpp_wf/result/xmpp_iq_gameroom_loosemaster.o \
./src/xmpp_wf/result/xmpp_iq_gameroom_offer.o \
./src/xmpp_wf/result/xmpp_iq_gameroom_on_kicked.o \
./src/xmpp_wf/result/xmpp_iq_gameroom_sync.o \
./src/xmpp_wf/result/xmpp_iq_invitation_request.o \
./src/xmpp_wf/result/xmpp_iq_notification_broadcast.o \
./src/xmpp_wf/result/xmpp_iq_p2p_ping.o \
./src/xmpp_wf/result/xmpp_iq_peer_clan_member_update.o \
./src/xmpp_wf/result/xmpp_iq_peer_player_info.o \
./src/xmpp_wf/result/xmpp_iq_peer_status_update.o \
./src/xmpp_wf/result/xmpp_iq_preinvite_invite.o \
./src/xmpp_wf/result/xmpp_iq_sync_notifications.o \
./src/xmpp_wf/result/xmpp_message.o \
./src/xmpp_wf/tools.o \


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

