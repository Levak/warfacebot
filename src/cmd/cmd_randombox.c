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
#include <wb_stream.h>
#include <wb_shop.h>
#include <wb_log.h>
#include <wb_list.h>
#include <wb_item.h>
#include <wb_lang.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define SHOP_MAX_BATCH 5

static unsigned int random_box_items_init = 0;
static struct random_box {
    const char *const name;
    const char *const shop_name;
    const char *const token_name;
    const unsigned int max_tokens;
    unsigned int shop_id[SHOP_MAX_BATCH];
    unsigned int box_id;
} random_box_items[] = {
    { "bullpup", "random_box_01", "box_token_cry_money_05", 1000, { 0, }, 0 },
    { "exarh", "random_box_02", "box_token_cry_money_34", 1000, { 0, }, 0 },
    { "kksmg", "random_box_03", "box_token_cry_money_03", 1000, { 0, }, 0 },
    { "bt50", "random_box_04", "box_token_cry_money_04", 1000, { 0, }, 0 },
    { "eagle", "random_box_05", "box_token_cry_money_01", 1000, { 0, }, 0 },
    { "us12", "random_box_06", NULL, 0, { 0, }, 0 },
    { "calico", "random_box_07", NULL, 0, { 0, }, 0 },
    { "ccr", "random_box_08", NULL, 0, { 0, }, 0 },
    { "ump", "random_box_09", NULL, 0, { 0, }, 0 },
    { "ak47", "random_box_10", "box_token_cry_money_10", 1000, { 0, }, 0 },
    { "rk14", "random_box_11", NULL, 0, { 0, }, 0 },
    { "fas12", "random_box_12", NULL, 0, { 0, }, 0 },
    { "fas12", "random_box_west_12", NULL, 0, { 0, }, 0 },
    { "t27", "random_box_13", NULL, 0, { 0, }, 0 },
    { "twm", "random_box_14", "box_token_cry_money_14", 1000, { 0, }, 0 },
    { "ac7smg", "random_box_15", NULL, 0, { 0, }, 0 },
    { "gu7", "random_box_16", NULL, 0, { 0, }, 0 },
    { "gu7", "random_box_west_16", NULL, 0, { 0, }, 0 },
    { "coin", "random_box_17", NULL, 0, { 0, }, 0 },
    { "msg", "random_box_18", NULL, 0, { 0, }, 0 },
    { "machete", "random_box_19", NULL, 0, { 0, }, 0 },
    { "machete", "random_box_west_19", NULL, 0, { 0, }, 0 },
    { "s18g", "random_box_20", NULL, 0, { 0, }, 0 },
    { "miller", "random_box_21", NULL, 0, { 0, }, 0 },
    { "katana", "random_box_22", NULL, 0, { 0, }, 0 },
    { "lmg3", "random_box_23", NULL, 0, { 0, }, 0 },
    { "lmg3", "random_box_west_23", NULL, 0, { 0, }, 0 },
    { "seven", "random_box_24", NULL, 0, { 0, }, 0 },
    { "pink", "random_box_25", NULL, 0, { 0, }, 0 },
    { "shark", "random_box_26", NULL, 0, { 0, }, 0 },
    { "shark", "random_box_west_26", NULL, 0, { 0, }, 0 },
    { "870", "random_box_27", NULL, 0, { 0, }, 0 },
    { "870", "random_box_west_27", NULL, 0, { 0, }, 0 },
    { "ccrcqb", "random_box_28", NULL, 0, { 0, }, 0 },
    { "x308", "random_box_29", "box_token_cry_money_29", 1000, { 0, }, 0 },
    { "para", "random_box_30", "box_token_cry_money_30", 1000, { 0, }, 0 },
    { "metasoma", "random_box_31", NULL, 0, { 0, }, 0 },
    { "metasoma", "random_box_west_31", NULL, 0, { 0, }, 0 },
    { "ay226", "random_box_32", NULL, 0, { 0, }, 0 },
    { "rba", "random_box_33", NULL, 0, { 0, }, 0 },
    { "sat", "random_box_34", "box_token_cry_money_02", 1000, { 0, }, 0 },
    { "exarl", "random_box_35", "box_token_cry_money_35", 1000, { 0, }, 0 },
    { "r16a4", "random_box_36", "box_token_cry_money_35", 1000, { 0, }, 0 },
    { "axe", "random_box_37", "box_token_cry_money_37", 1000, { 0, }, 0 },
    { "s22", "random_box_38", "box_token_cry_money_38", 1000, { 0, }, 0 },
    { "volcano", "random_box_39", NULL, 0, { 0, }, 0 },
    { "atf", "random_box_40", "box_token_cry_money_40", 1000, { 0, }, 0 },
    { "scout", "random_box_41", "box_token_cry_money_41", 1000, { 0, }, 0 },
    { "honey", "random_box_42", "box_token_cry_money_42", 1000, { 0, }, 0 },
    { "volcano2", "random_box_43", NULL, 0, { 0, }, 0 },
    { "europe", "random_box_44", NULL, 0, { 0, }, 0 },
    { "es-msg", "random_box_45", NULL, 0, { 0, }, 0 },
    { "es-exarl", "random_box_46", NULL, 0, { 0, }, 0 },
    { "es-x308", "random_box_47", NULL, 0, { 0, }, 0 },
    { "es-ay226", "random_box_48", NULL, 0, { 0, }, 0 },
    { "es-katana", "random_box_49", NULL, 0, { 0, }, 0 },
    { "es-ccr", "random_box_50", NULL, 0, { 0, }, 0 },
    { "dogsoldier", "random_box_51", NULL, 0, { 0, }, 0 },
    { "dogengineer", "random_box_52", NULL, 0, { 0, }, 0 },
    { "dogmedic", "random_box_53", NULL, 0, { 0, }, 0 },
    { "dogsniper", "random_box_54", NULL, 0, { 0, }, 0 },
    { "ltr6", "random_box_55", "box_token_cry_money_55", 1000, { 0, }, 0 },
    { "r65e4", "random_box_56", "box_token_cry_money_56", 1000, { 0, }, 0 },

    { "kommando", "random_box_58", NULL, 0, { 0, }, 0 },
    { "kommando", "random_box_west_58", NULL, 0, { 0, }, 0 },
    { "wx86", "random_box_59", "box_token_cry_money_59", 1000, { 0, }, 0 },
    { "jade", "random_box_60", NULL, 0, { 0, }, 0 },
    { "dragunov", "random_box_61", "box_token_cry_money_61", 1000, { 0, }, 0 },
    { "scarlet", "random_box_62", NULL, 0, { 0, }, 0 },

    { "zsd-exarh", "random_box_65", NULL, 0, { 0, }, 0 },
    { "zsd-bullpup", "random_box_66", NULL, 0, { 0, }, 0 },
    { "zsd-ac7smg", "random_box_67", NULL, 0, { 0, }, 0 },
    { "zsd-s18g", "random_box_68", NULL, 0, { 0, }, 0 },
    { "zsd-axe", "random_box_69", NULL, 0, { 0, }, 0 },
    { "zsd-scout", "random_box_70", NULL, 0, { 0, }, 0 },

    { "kapow", "random_box_72", "box_token_cry_money_72", 1000, { 0, }, 0 },
    { "peg", "random_box_73", "box_token_cry_money_73", 1000, { 0, }, 0 },
    { "crazyhorse", "random_box_74", "box_token_cry_money_74", 1000, { 0, }, 0 },
    { "zsd-atf", "random_box_75", NULL, 0, { 0, }, 0 },
    { "smg9", "random_box_76", "box_token_cry_money_76", 1000, { 0, }, 0 },
    { "frzn-r16", "random_box_77", NULL, 0, { 0, }, 0 },
    { "frzn-atf", "random_box_78", NULL, 0, { 0, }, 0 },
    { "frzn-honey", "random_box_79", NULL, 0, { 0, }, 0 },
    { "frzn-ay226", "random_box_80", NULL, 0, { 0, }, 0 },
    { "frzn-scout", "random_box_81", NULL, 0, { 0, }, 0 },
    { "frzn-axe", "random_box_82", NULL, 0, { 0, }, 0 },
    { "goldsmoke", "random_box_skin_83", NULL, 0, { 0, }, 0 },
    { "es-seven", "random_box_84", NULL, 0, { 0, }, 0 },
    { "es-ccr-cqb", "random_box_85", NULL, 0, { 0, }, 0 },
    { "es-miller", "random_box_86", NULL, 0, { 0, }, 0 },
    { "es-s18g", "random_box_87", NULL, 0, { 0, }, 0 },
    { "es-machete", "random_box_88", NULL, 0, { 0, }, 0 },
    { "es-t27", "random_box_89", NULL, 0, { 0, }, 0 },
    { "arx", "random_box_90", "box_token_cry_money_90", 1000, { 0, }, 0 },
    { "cv12", "random_box_91", "box_token_cry_money_91", 1000, { 0, }, 0 },
    { "ap84", "random_box_92", NULL, 0, { 0, }, 0 },
    { "evo3", "random_box_93", "box_token_cry_money_93", 1000, { 0, }, 0 },
    { "gq50", "random_box_94", "box_token_cry_money_94", 1000, { 0, }, 0 },

    { "santa-m4", "random_box_santa_01", NULL, 0, { 0, }, 0 },
    { "santa-fy103", "random_box_santa_02", NULL, 0, { 0, }, 0 },
    { "santa-smg19", "random_box_santa_03", NULL, 0, { 0, }, 0 },
    { "santa-bnp", "random_box_santa_04", NULL, 0, { 0, }, 0 },
    { "santa-p57", "random_box_santa_05", NULL, 0, { 0, }, 0 },

    { "skins-argar", "random_box_skin_01", NULL, 0, { 0, }, 0 },
    { "skins-rst14", "random_box_skin_02", NULL, 0, { 0, }, 0 },
    { "skins-mxstorm", "random_box_skin_03", NULL, 0, { 0, }, 0 },
    { "skins-r98b", "random_box_skin_04", NULL, 0, { 0, }, 0 },
    { "skins-r119d1", "random_box_skin_05", NULL, 0, { 0, }, 0 },
    { "skin-anubis-fy103", "random_box_skin_06", NULL, 0, { 0, }, 0 },
    { "skin-anubis-everest", "random_box_skin_07", NULL, 0, { 0, }, 0 },
    { "skin-anubis-smg19", "random_box_skin_08", NULL, 0, { 0, }, 0 },
    { "skin-anubis-viper", "random_box_skin_09", NULL, 0, { 0, }, 0 },
    { "skin-anubis-python", "random_box_skin_10", NULL, 0, { 0, }, 0 },
    { "skin-bird-fy103", "random_box_skin_11", NULL, 0, { 0, }, 0 },
    { "skin-bird-bullpup", "random_box_skin_12", NULL, 0, { 0, }, 0 },
    { "skin-bird-kksmg", "random_box_skin_13", NULL, 0, { 0, }, 0 },
    { "skin-bird-miller", "random_box_skin_14", NULL, 0, { 0, }, 0 },
    { "skin-bird-s18g", "random_box_skin_15", NULL, 0, { 0, }, 0 },
    { "skin-bird-axe", "random_box_skin_16", NULL, 0, { 0, }, 0 },
    { "skins-fy47", "random_box_skin_17", NULL, 0, { 0, }, 0 },
    { "skin-bird-r16a4", "random_box_skin_18", NULL, 0, { 0, }, 0 },
};

struct cb_args
{
    int count;
    const struct random_box *box;
};

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

    struct cb_args *a = (struct cb_args *) args;

    if (type & XMPP_TYPE_ERROR)
    {
        xprintf("%s", LANG(shop_error_purshase));
        free(a);
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
        int need_resync = 0;

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

                    const char *s;
                    const char *c;

                    if (expir && expir[0] != '0')
                    {
                        s = expir;
                        c = "";
                    }
                    else if (quant && quant[0] != '0')
                    {
                        s = quant;
                        c = "";
                    }
                    else if (perm)
                    {
                        s = "100%";
                        c = "\033[32;1m";
                    }
                    else
                    {
                        s = LANG(shop_item_permanent);
                        c = "\033[33;1m";
                        need_resync = 1;
                    }

                    xprintf("%s: %s%9s\033[0m %s",
                            LANG(shop_rb_item),
                            c,
                            s,
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
                xprintf("%s: %9u %s",
                        LANG(shop_rb_item),
                        total_xp,
                        LANG(experience_short));

            if (total_crown != 0)
                xprintf("%s: %9u %s",
                        LANG(shop_rb_item),
                        total_crown,
                        LANG(money_crown_short));

            session.profile.experience += total_xp;
            session.profile.money.game = game_money_left;
            session.profile.money.crown = crown_money_left;
            session.profile.money.cry = cry_money_left;

            if (error_code == 1)
            {
                xprintf("%s: %s",
                        LANG(error),
                        LANG(shop_error_no_money));
                need_resync = 1;
            }
            else
            {
                struct game_item *it = NULL;

                if (a->box->token_name != NULL)
                {
                    it = profile_item_list_get(a->box->token_name);

                    if (it != NULL)
                    {
                        it->quantity += a->count;
                    }
                    else
                    {
                        need_resync = 1;
                    }
                }
            }

            xprintf("%s: %9d %s - %9d %s - %9d %s",
                    LANG(shop_money_left),
                    game_money_left,
                    LANG(money_game_short),
                    crown_money_left,
                    LANG(money_crown_short),
                    cry_money_left,
                    LANG(money_cry_short));

            if (need_resync)
            {
                profile_item_list_update(NULL, NULL);
            }
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
                eprintf("%s: %s", LANG(error), reason);
            else
                eprintf("%s (code: %d)", LANG(error), error_code);
        }
    }

    free(data);
    free(a);
}

static void init_rb_item(const struct shop_offer *o, struct random_box *rb)
{
    /* If enough space and current shop offer matches RB name */
    if (rb->shop_id[rb->box_id] == 0
        && 0 == strcmp(o->name, rb->shop_name))
    {
        /* Insertion sort the current shop offer */
        for (unsigned int i = 0; i < rb->box_id; ++i)
        {
            const struct shop_offer *o2 =
                offer_list_get_by_id(rb->shop_id[i]);

            /* We can't find the offer back? weird */
            assert(o2 != NULL);
            if (o2 == NULL)
                continue;

            unsigned int o2_price =
                o2->price.game.curr
                + o2->price.crown.curr
                + o2->price.cry.curr
                + o2->price.key.curr;
            unsigned int o_price =
                o->price.game.curr
                + o->price.crown.curr
                + o->price.cry.curr
                + o->price.key.curr;

            /* Sort offers by price (higher first) */
            if (o_price > o2_price)
            {
                /* Store current item and propagate other ones */
                rb->shop_id[i] = o->id;
                o = o2;
            }
        }

       rb->shop_id[rb->box_id] = o->id;

       ++rb->box_id;
       if (rb->box_id >= SHOP_MAX_BATCH)
           rb->box_id = 0;
    }
}

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
                list_foreach(session.wf.shop.offers,
                             (f_list_callback) init_rb_item,
                             &random_box_items[i]);
            }

            random_box_items_init = 1;
        }
        else
        {
            eprintf("%s", LANG(shop_error_not_ready));
            return;
        }
    }
}

void cmd_randombox(const char *name, unsigned int count)
{
    init_rb_items();

    if (name == NULL)
    {
        xprintf("%s", LANG(console_randombox_available));

        unsigned int i = 0;
        for (; i < sizeof (random_box_items) / sizeof (random_box_items[0]);
             ++i)
        {
            if (random_box_items[i].shop_id[0] != 0)
            {
                const struct shop_offer *o =
                    offer_list_get_by_id(random_box_items[i].shop_id[0]);

                /* We can't find the offer back? weird */
                assert(o != NULL);
                if (o == NULL)
                    continue;

                const char *currency = "";
                unsigned int price = 0;
                unsigned int tokens = 0;
                unsigned int max_tokens = 0;
                char enough = 0;

                struct game_item *it = NULL;

                if (random_box_items[i].token_name != NULL)
                {
                    max_tokens = random_box_items[i].max_tokens;
                    it = profile_item_list_get(random_box_items[i].token_name);

                    if (it != NULL)
                    {
                        tokens = it->quantity;
                    }
                }

                if (o->price.cry.curr != 0)
                {
                    currency = LANG(money_cry_short);
                    price = o->price.cry.curr;
                    enough = price <= session.profile.money.cry;
                }
                else if (o->price.crown.curr != 0)
                {
                    currency = LANG(money_crown_short);
                    price = o->price.crown.curr;
                    enough = price <= session.profile.money.crown;
                }
                else if (o->price.game.curr != 0)
                {
                    currency = LANG(money_game_short);
                    price = o->price.game.curr;
                    enough = price <= session.profile.money.game;
                }
                else if (o->price.key.curr != 0)
                {
                    currency = LANG(money_key_short);
                    price = o->price.key.curr;
                }

                xprintf(random_box_items[i].token_name != NULL
                        ? " - %11s \033[%d;1m%u\033[0m %s (%u/%u)"
                        : " - %11s \033[%d;1m%u\033[0m %s",
                        random_box_items[i].name,
                        enough ? 32 : 31,
                        price,
                        currency,
                        tokens,
                        max_tokens);
            }
        }

        return;
    }

    if (count <= 0 || count > SHOP_MAX_BATCH)
    {
        eprintf("%s", LANG(console_randombox_invalid_amount));
        return;
    }

    struct random_box *box = NULL;
    unsigned int i = 0;

    for (; i < sizeof (random_box_items) / sizeof (random_box_items[0]);
         ++i)
    {
        if ((strcmp(name, random_box_items[i].name) == 0
             || strcmp(name, random_box_items[i].shop_name) == 0)
            && random_box_items[i].shop_id[0] != 0)
        {
            box = &random_box_items[i];
            break;
        }
    }

    if (box == NULL)
    {
        eprintf("%s", LANG(console_randombox_unknown));
        return;
    }

    {
        char *offers = NULL;
        unsigned int i = 0;

        if (box->box_id + count > SHOP_MAX_BATCH)
            box->box_id = 0;

        for (; i < count; ++i)
        {
            char *s;
            FORMAT(s, "%s<offer id='%d'/>",
                   offers ? offers : "",
                   box->shop_id[box->box_id]);
            free(offers);
            offers = s;

            box->box_id = box->box_id + 1;
        }

        struct cb_args *a = calloc(1, sizeof (struct cb_args));
        a->count = count;
        a->box = box;

        xmpp_send_iq_get(
            JID_MS(session.online.channel),
            _randombox_cb, a,
            "<query xmlns='urn:cryonline:k01'>"
            "<shop_buy_multiple_offer hash='%s' supplier_id='1'>"
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
        eprintf("%s", LANG(console_randombox_required_amount));
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
                if (random_box_items[i].shop_id[0] != 0)
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
