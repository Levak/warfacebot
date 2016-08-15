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

#include <wb_session.h>
#include <wb_xmpp_wf.h>
#include <wb_xmpp.h>
#include <wb_tools.h>
#include <wb_stream.h>
#include <wb_shop.h>
#include <wb_log.h>
#include <wb_list.h>

#include <stdlib.h>
#include <string.h>

static void _randombox_cb(const char *msg,
                          enum xmpp_msg_type type,
                          void *args)
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
           <exp name="exp_item_01" added="50" total="xxxx" offerId="9871"/>
           <crown_money name='crown_money_item_01' added='100' total='xxxx'/>
            ...
          </purchased_item>
          <money game_money="AAA" cry_money="BBB" crown_money="CCC"/>
         </shop_buy_multiple_offer>
        </query>
       </iq>
     */

    if (type & XMPP_TYPE_ERROR)
    {
        xprintf("Error while purshasing items\n");
        return;
    }

    char *data = wf_get_query_content(msg);

    if (data != NULL)
    {
        unsigned int error_code = get_info_int(data, "error_status='", "'", NULL);
        unsigned int game_money_left = get_info_int(data, "game_money='", "'", NULL);
        unsigned int crown_money_left = get_info_int(data, "crown_money='", "'", NULL);
        unsigned int cry_money_left = get_info_int(data, "cry_money='", "'", NULL);

        const char *m = strstr(data, "<shop_buy_multiple_offer");

        if (m != NULL && (error_code == 1 || error_code == 0))
        {
            unsigned total_xp = 0;
            unsigned total_crown = 0;

            m += sizeof ("<shop_buy_multiple_offer");

            do {

                const char *exp_s = strstr(m, "<exp");
                const char *crown_s = strstr(m, "<crown_money");
                const char *profile_item_s = strstr(m, "<profile_item");

                if (exp_s != NULL
                    && (profile_item_s == NULL || exp_s < profile_item_s)
                    && (crown_s == NULL || exp_s < crown_s))

                {
                    m = exp_s + sizeof ("<exp");

                    total_xp += get_info_int(m, "added='", "'", NULL);
                }
                else if (crown_s != NULL
                         && (profile_item_s == NULL || crown_s < profile_item_s)
                         && (exp_s == NULL || crown_s < exp_s))
                {
                    m = crown_s + sizeof ("<crown_money");

                    total_crown += get_info_int(m, "added='", "'", NULL);
                }
                else if (profile_item_s != NULL)
                {
                    m = profile_item_s + sizeof ("<profile_item");

                    char *name = get_info(m, "name='", "'", NULL);
                    char *expir = get_info(m, "added_expiration='", "'", NULL);
                    char *quant = get_info(m, "added_quantity='", "'", NULL);
                    int perm = get_info_int(m, "permanent='", "'", NULL);

                    xprintf("RB Item: %9s %s\n",
                           expir && expir[0] != '0' ? expir
                            : quant && quant[0] != '0' ? quant
                            : perm ? "100%"
                            : "Permanent",
                           name);

                    free(quant);
                    free(expir);
                    free(name);
                }
                else
                {
                    break;
                }

            } while (1);

            if (total_xp != 0)
                xprintf("RB Item: %9u XP\n",
                        total_xp);

            if (total_crown != 0)
                xprintf("RB Item: %9u crown\n",
                        total_crown);

            session.profile.experience += total_xp;
            session.profile.money.game = game_money_left;
            session.profile.money.crown = crown_money_left;
            session.profile.money.cry = cry_money_left;

            xprintf("Money left: %9d WFD - %9d Crowns - %9d K\n",
                    game_money_left,
                    crown_money_left,
                    cry_money_left);
        }
        else
        {
            switch (error_code)
            {
                case 1:
                    xprintf("Not enough money\n");
                    break;
                case 2:
                    xprintf("Restricted purshase\n");
                    break;
                case 3:
                    xprintf("Out of store\n");
                    break;
                case 4:
                    xprintf("Limit reached\n");
                    break;
                case 5:
                    xprintf("Item not available\n");
                    break;
                default:
                    xprintf("Error (code: %d)\n", error_code);
                    break;
            }
        }
    }

    free(data);

    return;
}

static unsigned int random_box_items_init = 0;
static struct random_box {
    const char *name;
    const char *shop_name;
    unsigned int shop_id;
    unsigned int box_id;
} random_box_items[] = {
    { "bullpup", "random_box_01", 0, 0 },
    { "exarh", "random_box_02", 0, 0 },
    { "kksmg", "random_box_03", 0, 0 },
    { "bt50", "random_box_04", 0, 0 },
    { "eagle", "random_box_05", 0, 0 },
    { "us12", "random_box_06", 0, 0 },
    { "calico", "random_box_07", 0, 0 },
    { "ccr", "random_box_08", 0, 0 },
    { "ump", "random_box_09", 0, 0 },
    { "ak47", "random_box_10", 0, 0 },
    { "rk14", "random_box_11", 0, 0 },
    { "fas12", "random_box_12", 0, 0 },
    { "t27", "random_box_13", 0, 0 },
    { "twm", "random_box_14", 0, 0 },
    { "ac7smg", "random_box_15", 0, 0 },
    { "gu7", "random_box_16", 0, 0 },
    { "coin", "random_box_17", 0, 0 },
    { "msg", "random_box_18", 0, 0 },
    { "machete", "random_box_19", 0, 0 },
    { "s18g", "random_box_20", 0, 0 },
    { "miller", "random_box_21", 0, 0 },
    { "katana", "random_box_22", 0, 0 },
    { "lmg3", "random_box_23", 0, 0 },
    { "seven", "random_box_24", 0, 0 },
    { "pink", "random_box_25", 0, 0 },
    { "shark", "random_box_26", 0, 0 },
    { "870", "random_box_27", 0, 0 },
    { "ccrcqb", "random_box_28", 0, 0 },
    { "x308", "random_box_29", 0, 0 },
    { "para", "random_box_30", 0, 0 },
    { "metasoma", "random_box_31", 0, 0 },
    { "ay226", "random_box_32", 0, 0 },
    { "rba", "random_box_33", 0, 0 },
    { "sat", "random_box_34", 0, 0 },
    { "exarl", "random_box_35", 0, 0 },
    { "r16a4", "random_box_36", 0, 0 },
    { "axe", "random_box_37", 0, 0 },
    { "s22", "random_box_38", 0, 0 },
    { "volcano", "random_box_39", 0, 0 },
    { "atf", "random_box_40", 0, 0 },
    { "scout", "random_box_41", 0, 0 },
    { "honey", "random_box_42", 0, 0 },
    { "volcano2", "random_box_43", 0, 0 },
    { "europe", "random_box_44", 0, 0 },
    { "es-msg", "random_box_45", 0, 0 },
    { "es-exarl", "random_box_46", 0, 0 },
    { "es-x308", "random_box_47", 0, 0 },
    { "es-ay226", "random_box_48", 0, 0 },
    { "es-katana", "random_box_49", 0, 0 },
    { "es-ccr", "random_box_50", 0, 0 },
    { "dogsoldier", "random_box_51", 0, 0 },
    { "dogengineer", "random_box_52", 0, 0 },
    { "dogmedic", "random_box_53", 0, 0 },
    { "dogsniper", "random_box_54", 0, 0 },
    { "ltr6", "random_box_55", 0, 0 },
    { "r65e4", "random_box_56", 0, 0 },

    { "kommando", "random_box_58", 0, 0 },
};

static void init_rb_items(void)
{
    if (random_box_items_init == 0)
    {
        if (session.wf.shop.offers != NULL)
        {
            unsigned int i = 0;
            for (; i < sizeof (random_box_items) / sizeof (random_box_items[0]);
                 ++i)
            {
                const struct shop_offer *o =
                    list_get(session.wf.shop.offers, random_box_items[i].shop_name);

                if (o == NULL)
                    continue;

                random_box_items[i].shop_id = o->id;
            }

            random_box_items_init = 1;
        }
        else
        {
            eprintf("Shop offers not fetched yet\n");
            return;
        }
    }
}

void cmd_randombox(const char *name, unsigned int count)
{
    init_rb_items();

    if (name == NULL)
    {
        xprintf("Random boxes available:\n");

       unsigned int i = 0;
       for (; i < sizeof (random_box_items) / sizeof (random_box_items[0]);
            ++i)
       {
           if (random_box_items[i].shop_id != 0)
           {
                const struct shop_offer *o =
                    list_get(session.wf.shop.offers, random_box_items[i].shop_name);

                if (o == NULL)
                    continue;

                const char *currency = "";
                unsigned int price = 0;

                if (o->price.cry.curr != 0)
                {
                    currency = "K";
                    price = o->price.cry.curr;
                }
                else if (o->price.crown.curr != 0)
                {
                    currency = "crown";
                    price = o->price.crown.curr;
                }
                else if (o->price.game.curr != 0)
                {
                    currency = "wfd";
                    price = o->price.game.curr;
                }
                else if (o->price.key.curr != 0)
                {
                    currency = " key";
                    price = o->price.key.curr;
                }

                xprintf(" - %11s %u %s\n",
                        random_box_items[i].name,
                        price,
                        currency);
           }
       }

       return;
    }

    if (count <= 0 || count > 5)
    {
        eprintf("Invalid amount\n");
        return;
    }

    struct random_box *box = NULL;
    unsigned int i = 0;

    for (; i < sizeof (random_box_items) / sizeof (random_box_items[0]);
         ++i)
    {
        if (strcmp(name, random_box_items[i].name) == 0
            || strcmp(name, random_box_items[i].shop_name) == 0)
        {
            box = &random_box_items[i];
            break;
        }
    }

    if (box == NULL)
    {
        eprintf("Unknown randombox\n");
        return;
    }

    {
        char *offers = NULL;
        unsigned int i = 0;

        if (box->box_id + count > 5)
            box->box_id = 0;

        for (; i < count; ++i)
        {
            char *s;
            FORMAT(s, "%s<offer id='%d'/>",
                   offers ? offers : "",
                   box->shop_id + box->box_id);
            free(offers);
            offers = s;

            box->box_id = box->box_id + 1;
        }

        xmpp_send_iq_get(
            JID_MS(session.online.channel),
            _randombox_cb, NULL,
            "<query xmlns='urn:cryonline:k01'>"
            "<shop_buy_multiple_offer hash='%i' supplier_id='1'>"
            "%s"
            "</shop_buy_multiple_offer>"
            "</query>",
            session.wf.shop.hash,
            offers);

        free(offers);
    }
}

void cmd_randombox_wrapper(const char *name,
                           const char *count_str)
{
    if (name == NULL)
        cmd_randombox(NULL, 0);
    else if (count_str != NULL)
        cmd_randombox(name,
                      strtol(count_str, NULL, 10));
    else
        xprintf("Box count required\n");
}

int cmd_randombox_completions(struct list *l, int arg_index)
{
    switch (arg_index)
    {
        case 1:
        {
            init_rb_items();

            unsigned int i = 0;
            for (; i < sizeof (random_box_items) / sizeof (random_box_items[0]);
                 ++i)
            {
                if (random_box_items[i].shop_id != 0)
                {
                    list_add(l, strdup(random_box_items[i].name));
                }
            }

            break;
        }

        case 2:
            list_add(l, strdup("1"));
            list_add(l, strdup("2"));
            list_add(l, strdup("3"));
            list_add(l, strdup("4"));
            list_add(l, strdup("5"));
            break;

        default:
            break;
    }

    return 1;
}
