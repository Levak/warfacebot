/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015, 2016 Levak Borok <levak92@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <wb_tools.h>
#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_game.h>
#include <wb_mission.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cb_args
{
    char *channel;
    f_join_channel_cb cb;
    void *args;
};

static void xmpp_iq_join_channel_cb(const char *msg,
                                    enum xmpp_msg_type type,
                                    void *args)
{
    /* Answer
      <iq from='masterserver@warface/pve_12' to='xxxxxx@warface/GameClient' type='result'>
       <query xmlns='urn:cryonline:k01'>
        <data query_name='join_channel' compressedData='...' originalSize='13480'/>
       </query>
      </iq>
     */

	/*
	 <join_channel><character nick='DevilsBitch6' gender='male' height='1' fatness='0' head='default_head_17' current_class='0' experience='1127035' pvp_rating_points='0' banner_badge='4294967295' banner_mark='4294967295' banner_stripe='4294967295' game_money='483985' cry_money='0' crown_money='144'>
	*/

    struct cb_args *a = (struct cb_args *) args;
    char *data = wf_get_query_content(msg);

    if (type & XMPP_TYPE_ERROR)
    {
        fprintf(stderr, "Failed to join channel\nReason: ");

        int code = get_info_int(msg, "code='", "'", NULL);
        int custom_code = get_info_int(msg, "custom_code='", "'", NULL);

        switch (code)
        {
            case 1006:
                fprintf(stderr, "QoS limit reached\n");
                break;
            case 503:
                fprintf(stderr, "Invalid channel (%s)\n", a->channel);
                break;
            case 8:
                switch (custom_code)
                {
                    case 0:
                        fprintf(stderr, "Invalid token (%s) or userid (%s)\n",
                                session.active_token,
                                session.online_id);
                        break;
                    case 1:
                        fprintf(stderr, "Invalid profile_id (%s)\n",
                                session.profile_id);
                        break;
                    case 2:
                        fprintf(stderr, "Game version mismatch (%s)\n",
                                game_version_get());
                        break;
                    case 3:
                        fprintf(stderr, "Banned\n");
                        break;
                    case 5:
                        fprintf(stderr, "Rank restricted\n");
                        break;
                    default:
                        fprintf(stderr, "Unknown code (%d)\n", custom_code);
                        break;
                }
                break;
            default:
                fprintf(stderr, "Unknown\n");
                break;
        }
    }
    else
    {
        /* Leave previous room if any */
        xmpp_iq_gameroom_leave();

        if (data != NULL)
        {
			
			unsigned int new_experience;
			unsigned int new_crowns;
			int new_money;
			new_experience = get_info_int(data, "experience='", "'", NULL);
			new_crowns = get_info_int(data, "crown_money='", "'", NULL);
			new_money = get_info_int(data, "game_money='", "'", NULL);
			if (new_experience && new_experience != session.experience)
				LOGPRINT("%-20s " BOLD "%u\n", "EXPERIENCE", session.experience = new_experience);
			if (new_crowns && new_crowns != session.crowns)
				LOGPRINT("%-20s " BOLD "%u\n", "CROWNS", session.crowns = new_crowns);
			if (new_money && new_money != session.game_money)
				LOGPRINT("%-20s " BOLD "%u\n", "MONEY", session.game_money = new_money);

            if (a->channel != NULL)
            {
                free(session.channel);
                session.channel = strdup(a->channel);
            }


            char *m = data;

            while ((m = strstr(m, "<notif")))
            {
                char *notif = get_info(m, "<notif", "</notif>", NULL);

                xmpp_iq_confirm_notification(notif);
                free(notif);
                ++m;
            }
        }

		if (!a || !a->args)
		{
			/* Ask for today's missions list */
			mission_list_update(NULL, NULL);
		}

		/* Inform to k01 our status */
		xmpp_iq_player_status(STATUS_ONLINE | STATUS_LOBBY);

        if (a->cb)
            a->cb(a->args);
    }

    free(data);
    free(a->channel);
    free(a);
}

void xmpp_iq_join_channel(const char *channel, f_join_channel_cb f, void *args)
{
    int is_switch = session.status >= STATUS_LOBBY;
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = f;
    a->args = args;

    if (channel == NULL)
        channel = session.channel;

    if (channel)
        a->channel = strdup(channel);
    else
        a->channel = NULL;

    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_iq_join_channel_cb, a);

    /* Join CryOnline channel */
    send_stream_format(session.wfs,
                       "<iq id='%s' to='k01.warface' type='get'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<%s_channel version='%s' token='%s' region_id='global'"
                       "     profile_id='%s' user_id='%s' resource='%s'"
                       "     user_data='' hw_id='' build_type='--release'/>"
                       "</query>"
                       "</iq>",
                       &id, is_switch ? "switch" : "join",
                       game_version_get(),
                       session.active_token, session.profile_id,
                       session.online_id, a->channel);
}
