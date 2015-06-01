/**
* WarfaceBot, a blind XMPP client for Warface (FPS)
* Copyright (C) 2015 Levak Borok <levak92@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <wb_tools.h>
#include <wb_stream.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <listener.h>

#include <stdlib.h>
#include <stdio.h>

void xmpp_iq_presence_cb ( const char *msg_id,
						const char *msg,
						void *args )
{
	// ->Human
	//<presence from='room.pve_11.395@conference.warface/Devil_Daga'
	//	to='20545716@warface/GameClient' xml:lang='en' type='unavailable'>
	//	<priority>0</priority>
	//	<c xmlns='http://jabber.org/protocol/caps' hash='sha-1' node='http://camaya.net/gloox'
	//	ver='0RyJmsC2EQAjYmYlhkMGaVEgE/8='/>
	//	<x xmlns='http://jabber.org/protocol/muc#user'>
	//		<item affiliation='owner' role='none'/>
	//	</x>
	//</presence>

	// ->Bot
	//<presence from='room.pve_12.3705@conference.warface/DevilsBitch2'
	//	to='20545716@warface/GameClient' xml:lang='en' id='uid00000018'>
	//	<x xmlns='http://jabber.org/protocol/muc#user'>
	//		<item affiliation='none' role='participant'/>
	//	</x>
	//</presence>
	char *nick = get_info ( msg, "/", "'", NULL );
	char *isHuman = get_info ( msg, "<priority>", "</", NULL );// Unused
	char *leaving = get_info ( msg, "type='", "'", NULL );
	LOGPRINT ( "%-16s "KGRN BOLD"%s\n"KRST,
			   (leaving) ? "Player Left" : "Player Joined",
			   nick
			   );
	if ( leaving )
		remove_listener ( nick );
	else if ( session.troll )
	{
		session.troll = 0;
		xmpp_iq_gameroom_leave ( );
	}
	free ( leaving );
	free ( isHuman );
	free ( nick );
}

void xmpp_iq_presence_r ( void )
{
	qh_register ( "presence", xmpp_iq_presence_cb, NULL );
}