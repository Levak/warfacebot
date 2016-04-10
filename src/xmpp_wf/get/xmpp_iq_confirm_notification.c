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
#include <wb_xmpp_wf.h>

#include <stdio.h>

enum e_notif_result
{
    NOTIF_ACCEPT = 0,
    NOTIF_REFUSE = 1,
};

static void confirm(const char *notif_id,
                    enum e_notif_type notif_type,
                    enum e_notif_result result)
{
    send_stream_format(session.wfs,
                       "<iq to='masterserver@warface/%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <confirm_notification>"
                       "   <notif id='%s' type='%d'>"
                       "    <confirmation result='%d' status='%d'"
                       "                  location=''/>"
                       "   </notif>"
                       "  </confirm_notification>"
                       " </query>"
                       "</iq>",
                       session.channel, notif_id, notif_type,
                       result, session.status);
}

void xmpp_iq_confirm_notification(const char *notif)
{
	
    char *notif_id = get_info(notif, "id='", "'", NULL);
    enum e_notif_type notif_type = get_info_int(notif, "type='", "'", NULL);

	if(notif_type != (NOTIF_STATUS_UPDATE | NOTIF_CLAN_INVITE) ||
	   (notif_type == NOTIF_CLAN_INVITE && session.clan_id == 0))
		confirm(notif_id, notif_type, NOTIF_ACCEPT);

    switch (notif_type)
    {
        /* Confirm consecutive logins */
        case NOTIF_NEW_RANK:
		{
			/*
			id='0' type='131072' confirmation='0' from_jid='masterserver@warface/pve_5' message=''><new_rank_reached old_rank='59' new_rank='60'/>

			*/
			int new_rank = get_info_int(notif, "new_rank='", "'", NULL);
			LOGPRINT(KGRN BOLD"%-20s RANK: %d\n", "Levelled up! \a", new_rank);
			break;
		}
		/* Gets triggered on clan role change or clan kick? */
		/*
		<iq from='masterserver@warface/pve_6' to='22889065@warface/GameClient' xml:lang='en' id='uid001d38c5' type='get'><query xmlns='urn:cryonline:k01'><sync_notifications><notif id='0' type='8' confirmation='0' from_jid='masterserver@warface/pve_6' message=''><message data='@clans_you_are_promoted_to_officer'/></notif></sync_notifications></query></iq>
		*/
		/*
		<iq from='masterserver@warface/pve_6'...><message data='@clans_you_was_kicked'/></notif></sync_notifications></query></iq>
		*/
        case NOTIF_UNLOCK_MISSION:
			if (strstr(notif, "message data='@clans_you_was_kicked'"))
			{
				session.clan_id = 0;
				LOGPRINT(BOLD KRED "%s\n", "KICKED FROM CLAN");
			}
			else if (strstr(notif, "message data='@clans_you_are_promoted_to_officer'"))
			{
				session.clan_role = CLAN_OFFICER;
				LOGPRINT("%-20s " BOLD "OFFICER\n", "CLAN ROLE");
			}
			else if (strstr(notif, "message data='@clans_you_are_demoted_to_regular'"))
			{
				session.clan_role = CLAN_MEMBER;
				LOGPRINT("%-20s " BOLD "REGULAR\n", "CLAN ROLE");
			}
			else if (strstr(notif, "message data='@clans_you_are_promoted_to_master'"))
			{
				session.clan_role = CLAN_MASTER;
				LOGPRINT("%-20s " BOLD "MASTER\n", "CLAN ROLE");
			}
			else if (strstr(notif, "message data='@clans_you_are_demoted_to_officer'"))
			{
				session.clan_role = CLAN_OFFICER;
				LOGPRINT("%-20s " BOLD "OFFICER\n", "CLAN ROLE");
			}
			else
			{
				puts(notif);
				LOGPRINT("%s\n", "Unlocked mission!");
			}
            break;
        case NOTIF_CONS_LOGIN:
			puts(notif);
            LOGPRINT("%s\n", "Getting daily reward!");
            break;
        case NOTIF_GIVE_ITEM:
		{
			/*
			id='129934139' type='256' confirmation='1' from_jid='masterserver@warface/pve_3' message=''><give_item name='shg27_shop' offer_type='Permanent'/>
			*/
			char *item_name = get_info(notif, "name='", "'", "RECEIVED ITEM");
			
			free(item_name);
            break;
		}
        case NOTIF_GIVE_RANDOM_BOX:
		{
			/*
			id='129832099' type='8192' confirmation='1' from_jid='masterserver@warface/pve_5' message=''><give_random_box name='random_box_rank_04'><purchased_item><profile_item name='coin_01' profile_item_id='81541368' offerId='0' added_expiration='0' added_quantity='8' error_status='0'><item id='81541368' name='coin_01' attached_to='0' config='' slot='0' equipped='0' default='0' permanent='0' expired_confirmed='0' buy_time_utc='1459424674' quantity='164'/></profile_item><profile_item name='sr16_shop' profile_item_id='81705267' offerId='0' added_expiration='14 day' added_quantity='0' error_status='0'><item id='81705267' name='sr16_shop' attached_to='0' config='dm=0;material=default;pocket_index=0' slot='0' equipped='0' default='0' permanent='0' expired_confirmed='0' buy_time_utc='1459534712' expiration_time_utc='1462299512' seconds_left='2238320'/></profile_item></purchased_item></give_random_box>
			*/
			
			char *name = NULL, *expiration = NULL, *rem, *backup = strdup(notif);
			rem = backup;
			char *rewards = strdup("RANDOM BOX      " BOLD);
			while(1)
			{
				rem = strstr(rem, "<profile_item");
				if(!rem)
					break;

				name = get_info(rem, "profile_item name='", "'", NULL);
				expiration = get_info(rem, "added_expiration='", "'", NULL);

				char *old_rewards = strdup(rewards);
				if (expiration && strlen(expiration) <= 2)
					FORMAT(rewards, "%s %24s", old_rewards, name);
				else
					FORMAT(rewards, "%s %24s, %-6s", old_rewards, name, expiration);
				rem += 10;
				free(old_rewards);
			}
			rem = strstr(backup, "exp name=");
			if(rem)
			{
				int xp_rewards = get_info_int(rem, "added='", "'", NULL);
				char *old_rewards = strdup(rewards);
				FORMAT(rewards, "%s   " KBLU BOLD "Experience %d", old_rewards, xp_rewards);
				free(old_rewards);

				session.experience += xp_rewards;
			}
            LOGPRINT("%s\n", rewards);
            confirm(notif_id, notif_type, NOTIF_ACCEPT);

			free(backup);
            break;
		}

        /* Accept any friend requests */
        case NOTIF_FRIEND_REQUEST:
		{
			char *nick = get_info(notif, "initiator='", "'", NULL);
			LOGPRINT("%-20s " KGRN BOLD "%s\n", "FRIEND INVITE FROM", nick);
            break;
		}

        /* Accept any clan invites only if we don't already have one */
        case NOTIF_CLAN_INVITE:
		{
			char *clan_name = get_info(notif, "clan_name='", "'", NULL);
			char *nick = get_info(notif, "initiator='", "'", NULL);
			LOGPRINT("%-20s " KGRN BOLD "%s\n", "CLAN INVITE FROM", nick);
			LOGPRINT("%-20s " BOLD "%s\n", "CLAN NAME", clan_name);
            break;
		}

        /* Old fashion peer_status_update */
        case NOTIF_STATUS_UPDATE:
        {
            char *jid = get_info(notif, "jid='", "'", NULL);
            char *nick = get_info(notif, "nickname='", "'", NULL);
            char *pid = get_info(notif, "profile_id='", "'", NULL);
            int status = get_info_int(notif, "status='", "'", NULL);
            int exp = get_info_int(notif, "experience='", "'", NULL);

            if (status <= STATUS_OFFLINE)
                jid = NULL;

            friend_list_add(jid, nick, pid, status, exp);
            xmpp_iq_peer_status_update(jid);

            free(jid);
            free(nick);
            free(pid);

            break;
        }

        case NOTIF_ANNOUNCEMENT:
        case NOTIF_ACHIEVEMENT:
            confirm(notif_id, notif_type, NOTIF_ACCEPT);
            break;

        default:
            break;
    }

    free(notif_id);
}
