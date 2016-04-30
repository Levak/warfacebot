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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <wb_session.h>
#include <wb_xmpp_wf.h>
#include <wb_xmpp.h>
#include <wb_tools.h>
#include <wb_stream.h>

struct cmd_randombox_args_cb_t
{
	int gotNeeded;
	int xp;
	int moneyLeft, stopMoney;
	int rid;
	char *needed;
};

static void _randombox_cb ( const char *msg,
	enum xmpp_msg_type type,
	void *args )
{
	/* Answer :
	<iq to='masterserver@warface/pve_2' type='get'>
	<query xmlns='urn:cryonline:k01'>
	<shop_buy_multiple_offer error_status="0">
	<purchased_item>
	<exp name="exp_item_01" added="50" total="40014" offerId="9870"/>
	<profile_item name="flashbang" profile_item_id="xxxxx"
	offerId="9870" added_expiration="1 day"
	added_quantity="0" error_status="0">
	<item id="xxxxx" name="flashbang" attached_to="0"
	config="dm=0;material=;pocket_index=3246082"
	slot="0" equipped="0" default="0"
	permanent="0" expired_confirmed="0"
	buy_time_utc="1429646487"
	expiration_time_utc="1449778407"
	seconds_left="172628"/>
	</profile_item>
	<exp name="exp_item_01" added="50" total="40064" offerId="9871"/>
	...
	</purchased_item>
	<money game_money="AAA" cry_money="BBB" crown_money="CCC"/>
	</shop_buy_multiple_offer>
	</query>
	</iq>
	*/
	
	struct cmd_randombox_args_cb_t *randombox_args = (struct cmd_randombox_args_cb_t*) args;

	if ( type & XMPP_TYPE_ERROR )
	{
		LOGPRINT ( KRED BOLD "Error while purchasing items\n" );
		return;
	}

	char *data = wf_get_query_content ( msg );

	if ( data != NULL )
	{
		//printf ( "Answer: \n---------\n%s\n--------\n\n", data );

		unsigned int error_code = get_info_int ( data, "error_status='", "'", NULL );
		unsigned int money_left = get_info_int ( data, "game_money='", "'", NULL );

		const char *m = strstr ( data, "<shop_buy_multiple_offer" );

		if ( m != NULL && ( error_code == 1 || error_code == 0 ) )
		{
			unsigned total_xp = 0;

			m += sizeof ( "<shop_buy_multiple_offer" );

			do
			{

				const char *exp_s = strstr ( m, "<exp" );
				const char *profile_item_s = strstr ( m, "<profile_item" );

				if ( exp_s != NULL
					 && ( profile_item_s == NULL || exp_s < profile_item_s ) )
				{
					m = exp_s + sizeof ( "<exp" );

					total_xp += get_info_int ( m, "added='", "'", NULL );
				}
				else if ( profile_item_s != NULL )
				{
					m = profile_item_s + sizeof ( "<profile_item" );

					char *name = get_info ( m, "name='", "'", NULL );
					char *expir = get_info ( m, "added_expiration='", "'", NULL );
					char *quant = get_info ( m, "added_quantity='", "'", NULL );

					LOGPRINT ( "%-20s %-10s %s\n", "ITEM",
							   expir && expir[ 0 ] != '0' ? expir : quant,
							   name );

					if ( randombox_args->needed && strstr ( name, randombox_args->needed ) )
					{
						LOGPRINT ( KGRN BOLD "%-20s %s\a\n" KRST KWHT, "GOT ITEM", name );
						randombox_args->gotNeeded = 1;
					}

					free ( quant );
					free ( expir );
					free ( name );
				}
				else
				{
					break;
				}

			} while ( 1 );

			randombox_args->moneyLeft = money_left;
			randombox_args->xp += total_xp;

			LOGPRINT ( "%-20s " BOLD "%d\n", "MONEY LEFT", money_left );
		}
		else
		{
			switch ( error_code )
			{
				case 2:
					LOGPRINT ( KRED BOLD "Restricted purchase\n" );
					break;
				default:
					break;
			}
		}
	}

	free ( data );

	return;
}

void *thread_buyboxes ( void *args )
{
	struct cmd_randombox_args_cb_t *randombox_args = ( struct cmd_randombox_args_cb_t* ) args;

	while ( randombox_args->moneyLeft > randombox_args->stopMoney && !randombox_args->gotNeeded )
	{
		char *offers = NULL;
		unsigned int i = 0;

		for ( ; i < 5; ++i )
		{
			char *s;
			FORMAT ( s, "%s<offer id='%d'/>", offers ? offers : "", randombox_args->rid + i );
			free ( offers );
			offers = s;
		}
		
		t_uid id;

		idh_generate_unique_id ( &id );
		idh_register ( &id, 0, _randombox_cb, randombox_args );

		send_stream_format ( session.wfs,
							 "<iq id='%s' to='masterserver@warface/%s' type='get'>"
							 "<query xmlns='urn:cryonline:k01'>"
							 "<shop_buy_multiple_offer supplier_id='1'>"
							 "%s"
							 "</shop_buy_multiple_offer>"
							 "</query>"
							 "</iq>",
							 &id, session.channel, offers );

		free ( offers );

		sleep ( 1 );
	}
	sleep ( 2 );
	
	session.game_money = randombox_args->moneyLeft;
	session.experience += randombox_args->xp;
	LOGPRINT ( "%-20s " BOLD "%d\n", "TOTAL XP EARNED", randombox_args->xp );

	free ( randombox_args->needed );
	free ( randombox_args );
	pthread_exit ( NULL );
}

void cmd_randombox ( const char *name, const char *needed, int moneyLeft )
{
	if ( name == NULL )
		return;

	unsigned int rid = 0;

	struct cmd_randombox_args_cb_t *randombox_args = calloc ( 1, sizeof ( struct cmd_randombox_args_cb_t ) );
	randombox_args->gotNeeded = 0;
	randombox_args->xp = 0;
	randombox_args->needed = strdup ( needed );

	if ( strstr ( name, "eagle" ) ) rid = 8944;
	else if ( strstr ( name, "ak47" ) ) rid = 9870;
	else if ( strstr ( name, "miller" ) ) rid = 10443;
	else if ( strstr ( name, "870" ) ) rid = 10450;
	else if ( strstr ( name, "twm" ) ) rid = 10572;
	else if ( strstr ( name, "gu7" ) ) rid = 10814;
	else if ( strstr ( name, "acc7" ) ) rid = 10832;
	else if ( strstr ( name, "ump" ) ) rid = 12843;
	else if ( strstr ( name, "para" ) ) rid = 12944;
	else if ( strstr ( name, "s18g" ) ) rid = 12969;
	else if ( strstr ( name, "fararm" ) ) rid = 13000;
	else if ( strstr ( name, "exarl" ) ) rid = 13007;
	else if ( strstr ( name, "shark" ) ) rid = 13012;
	else if ( strstr ( name, "x308" ) ) rid = 13020;
	else if ( strstr ( name, "machete" ) ) rid = 13055;
	else if ( strstr ( name, "axe" ) ) rid = 13060;
	else if ( strstr ( name, "r16a4" ) ) rid = 13065;
	else if ( strstr ( name, "lmg3" ) ) rid = 13148;
	else if ( strstr ( name, "s22" ) ) rid = 13161;
	else
		LOGPRINT ( KRED BOLD "Unknown randombox\n" );

	if ( rid > 0 )
	{
		randombox_args->rid = rid;
		randombox_args->moneyLeft = session.game_money;
		randombox_args->stopMoney = moneyLeft;

		pthread_t th_buyboxes;

		if ( pthread_create ( &th_buyboxes, NULL, &thread_buyboxes, randombox_args ) == -1 )
			perror ( "pthread_create" );
		else
			pthread_detach ( th_buyboxes );
	}
}