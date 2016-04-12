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
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static void xmpp_print_number_of_occupants_cb ( const char *msg,
enum xmpp_msg_type type,
	void *args )
{
	char *num = get_info ( msg,
						   "var='muc#roominfo_occupants'><value>",
						   "</value>",
						   NULL );

	fprintf ( stderr, "%u %s\n", (unsigned) time ( NULL ), num );

	free ( num );
}

void xmpp_print_number_of_occupants ( int wfs, char *room )
{
	t_uid id;

	idh_generate_unique_id ( &id );
	idh_register ( &id, 0, xmpp_print_number_of_occupants_cb, NULL );

	send_stream_format ( wfs,
						 "<iq to='%s' type='get' id='%s'>"
						 " <query xmlns='http://jabber.org/protocol/disco#info'/>"
						 "</iq>",
						 room, &id );
}