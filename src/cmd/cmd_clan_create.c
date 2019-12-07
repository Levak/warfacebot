/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015-2017 Levak Borok <levak92@gmail.com>
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

#include <wb_session.h>
#include <wb_xmpp_wf.h>
#include <wb_xmpp.h>
#include <wb_tools.h>
#include <wb_shop.h>
#include <wb_log.h>
#include <wb_item.h>
#include <wb_lang.h>

#include <stdlib.h>
#include <string.h>

struct cb_args
{
    char *name;
    char *description;
};

/* TODO: Use a dedicated shop manager */
static void _shop_cb(const char *msg,
                     enum xmpp_msg_type type,
                     void *args)
{
    /* Answer :
       <iq to='masterserver@warface/pve_2' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <shop_buy_offer error_status="0">
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
           <exp name="exp_item_01" added="50" total="xxxx" offerId="9871"/>
           <crown_money name='crown_money_item_01' added='100' total='xxxx'/>
          </purchased_item>
          <money game_money="AAA" cry_money="BBB" crown_money="CCC"/>
         </shop_buy_offer>
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (type & XMPP_TYPE_ERROR)
    {
        xprintf("Error while purshasing item\n");
    }
    else
    {
        char *data = wf_get_query_content(msg);

        if (data != NULL)
        {
            unsigned int error_code =
                get_info_int(data, "error_status='", "'", NULL);

            if (!error_code)
            {
                    xmpp_iq_clan_create(a->name, a->description, NULL, NULL);

                    /* TODO: Use a dedicated shop manager */
                    profile_item_list_update(NULL, NULL);
            }
            else
            {
                const char *reason = NULL;

                switch (error_code)
                {
                    case 1:
                        reason = LANG(shop_error_no_money);
                        break;
                    case 2:
                        reason = LANG(shop_error_restricted);
                        break;
                    case 3:
                        reason = LANG(shop_error_out_of_store);
                        break;
                    case 4:
                        reason = LANG(shop_error_limit_reached);
                        break;
                    case 5:
                        reason = LANG(shop_error_no_item);
                        break;
                    case 6:
                        reason = LANG(shop_error_tag);
                        break;
                    case 8:
                        reason = LANG(shop_error_timeout);
                        break;
                    default:
                        break;
                }

                if (reason != NULL)
                    eprintf("%s: %s", LANG(error_clan_create), reason);
                else
                    eprintf("%s (code: %d)", LANG(error_clan_create), error_code);
            }
        }

        free(data);
    }

    free(a->name);
    free(a->description);
    free(a);
}

void cmd_clan_create(const char *name, const char *description)
{
    if (session.wf.shop.offers != NULL)
    {
        /* TODO: sync with online cvars */
        if (1)
        {
            const struct game_item *i = profile_item_list_get("clan_creation_item_01");

            if (i != NULL && i->quantity > 0)
            {
                xmpp_iq_clan_create(name, description, NULL, NULL);

                /* TODO: Use a dedicated shop manager */
                profile_item_list_update(NULL, NULL);
            }
            else
            {
                const struct shop_offer *o =
                    list_get(session.wf.shop.offers, "clan_creation_item_01");

                if (o != NULL)
                {
                    struct cb_args *a = calloc(1, sizeof (struct cb_args));

                    a->name = strdup(name);
                    a->description = strdup(description);

                    xmpp_send_iq_get(
                        JID_MS(session.online.channel),
                        _shop_cb, a,
                        "<query xmlns='urn:cryonline:k01'>"
                        "<shop_buy_offer"
                        " hash='%s' supplier_id='1'"
                        " offer_id='%d'/>"
                        "</query>",
                        session.wf.shop.hash,
                        o->id);
                }
                else
                {
                    eprintf("Cannot find clan creation item\n");
                }
            }
        }
        else
        {
            xmpp_iq_clan_create(name, description, NULL, NULL);

            /* TODO: Use a dedicated shop manager */
            profile_item_list_update(NULL, NULL);
        }
    }
    else
    {
        eprintf("Shop offers not fetched yet\n");
    }
}

void cmd_clan_create_wrapper(const char *name,
                             const char *description)
{
    cmd_clan_create(name, description);
}

int cmd_clan_create_completions(struct list *l, int arg_index)
{
    switch (arg_index)
    {
        case 1:
        case 2:
            break;

        default:
            break;
    }

    return 1;
}
