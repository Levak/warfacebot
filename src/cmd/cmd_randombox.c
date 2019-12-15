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
    { "scarh", "random_box_02", "box_token_cry_money_34", 1000, { 0, }, 0 },
    { "ctar", "random_box_03", "box_token_cry_money_03", 1000, { 0, }, 0 },
    { "as50", "random_box_04", "box_token_cry_money_04", 1000, { 0, }, 0 },
    { "as50-c", "random_box_04_crown", "box_token_cry_money_04", 1000, { 0, }, 0 },
    { "eagle", "random_box_05", "box_token_cry_money_01", 1000, { 0, }, 0 },
    { "usas12", "random_box_06", NULL, 0, { 0, }, 0 },
    { "calico", "random_box_07", NULL, 0, { 0, }, 0 },
    { "acr", "random_box_08", NULL, 0, { 0, }, 0 },
    { "ump", "random_box_09", NULL, 0, { 0, }, 0 },
    { "ump-c", "random_box_09_crown", NULL, 0, { 0, }, 0 },
    { "ak47", "random_box_10", "box_token_cry_money_10", 1000, { 0, }, 0 },
    { "mk14", "random_box_11", NULL, 0, { 0, }, 0 },
    { "mk14-c", "random_box_11_crown", NULL, 0, { 0, }, 0 },
    { "aa12", "random_box_12", NULL, 0, { 0, }, 0 },
    { "aa12", "random_box_west_12", NULL, 0, { 0, }, 0 },
    { "type97", "random_box_13", NULL, 0, { 0, }, 0 },
    { "awm", "random_box_14", "box_token_cry_money_14", 1000, { 0, }, 0 },
    { "xm8", "random_box_15", NULL, 0, { 0, }, 0 },
    { "daewoo-k7", "random_box_16", NULL, 0, { 0, }, 0 },
    { "daewoo-k7", "random_box_west_16", NULL, 0, { 0, }, 0 },
    { "coin", "random_box_17", NULL, 0, { 0, }, 0 },
    { "mossberg", "random_box_18", NULL, 0, { 0, }, 0 },
    { "machete", "random_box_19", NULL, 0, { 0, }, 0 },
    { "machete", "random_box_west_19", NULL, 0, { 0, }, 0 },
    { "glock-18c", "random_box_20", NULL, 0, { 0, }, 0 },
    { "mcmillan", "random_box_21", NULL, 0, { 0, }, 0 },
    { "katana", "random_box_22", NULL, 0, { 0, }, 0 },
    { "mg3", "random_box_23", NULL, 0, { 0, }, 0 },
    { "mg3", "random_box_west_23", NULL, 0, { 0, }, 0 },
    { "six12", "random_box_24", NULL, 0, { 0, }, 0 },
    { "pink", "random_box_25", NULL, 0, { 0, }, 0 },
    { "grach", "random_box_26", NULL, 0, { 0, }, 0 },
    { "grach", "random_box_west_26", NULL, 0, { 0, }, 0 },
    { "870", "random_box_27", NULL, 0, { 0, }, 0 },
    { "870", "random_box_west_27", NULL, 0, { 0, }, 0 },
    { "acrcqb", "random_box_28", NULL, 0, { 0, }, 0 },
    { "ax308", "random_box_29", "box_token_cry_money_29", 1000, { 0, }, 0 },
    { "para", "random_box_30", "box_token_cry_money_30", 1000, { 0, }, 0 },
    { "para-c", "random_box_30_crown", "box_token_cry_money_30", 1000, { 0, }, 0 },
    { "skorpion83", "random_box_31", NULL, 0, { 0, }, 0 },
    { "skorpion83", "random_box_west_31", NULL, 0, { 0, }, 0 },
    { "sig-sauer", "random_box_32", "box_token_cry_money_32", 1000, { 0, }, 0 },
    { "dsa-spr", "random_box_33", NULL, 0, { 0, }, 0 },
    { "sat", "random_box_34", "box_token_cry_money_02", 1000, { 0, }, 0 },
    { "scarl", "random_box_35", "box_token_cry_money_35", 1000, { 0, }, 0 },
    { "scarl-c", "random_box_35_crown", "box_token_cry_money_35", 1000, { 0, }, 0 },
    { "m16a3", "random_box_36", "box_token_cry_money_36", 1000, { 0, }, 0 },
    { "axe", "random_box_37", "box_token_cry_money_37", 1000, { 0, }, 0 },
    { "remington-r11", "random_box_38", "box_token_cry_money_38", 1000, { 0, }, 0 },
    { "volcano", "random_box_39", NULL, 0, { 0, }, 0 },
    { "fabarm-stf", "random_box_40", "box_token_cry_money_40", 1000, { 0, }, 0 },
    { "fabarm-stf-c", "random_box_40_crown", "box_token_cry_money_40", 1000, { 0, }, 0 },
    { "scout", "random_box_41", "box_token_cry_money_41", 1000, { 0, }, 0 },
    { "honey", "random_box_42", "box_token_cry_money_42", 1000, { 0, }, 0 },
    { "volcano2", "random_box_43", NULL, 0, { 0, }, 0 },
    { "europe", "random_box_44", NULL, 0, { 0, }, 0 },
    { "es-mossberg", "random_box_45", NULL, 0, { 0, }, 0 },
    { "es-mossberg-c", "random_box_45_crown", NULL, 0, { 0, }, 0 },
    { "es-scarl", "random_box_46", NULL, 0, { 0, }, 0 },
    { "es-scarl-c", "random_box_46_crown", NULL, 0, { 0, }, 0 },
    { "es-ax308", "random_box_47", NULL, 0, { 0, }, 0 },
    { "es-ax308-c", "random_box_47_crown", NULL, 0, { 0, }, 0 },
    { "es-sig-sauer", "random_box_48", NULL, 0, { 0, }, 0 },
    { "es-sig-sauer-c", "random_box_48_crown", NULL, 0, { 0, }, 0 },
    { "es-katana", "random_box_49", NULL, 0, { 0, }, 0 },
    { "es-acr", "random_box_50", NULL, 0, { 0, }, 0 },
    { "es-acr-c", "random_box_50_crown", NULL, 0, { 0, }, 0 },
    { "dogsoldier", "random_box_51", NULL, 0, { 0, }, 0 },
    { "dogengineer", "random_box_52", NULL, 0, { 0, }, 0 },
    { "dogmedic", "random_box_53", NULL, 0, { 0, }, 0 },
    { "dogsniper", "random_box_54", NULL, 0, { 0, }, 0 },
    { "fabarm-xlr", "random_box_55", "box_token_cry_money_55", 1000, { 0, }, 0 },
    { "fabarm-xlr-c", "random_box_55_crown", "box_token_cry_money_55", 1000, { 0, }, 0 },
    { "enfield", "random_box_56", "box_token_cry_money_56", 1000, { 0, }, 0 },
    { "enfield-c", "random_box_56_crown", "box_token_cry_money_56", 1000, { 0, }, 0 },

    { "kommando", "random_box_58", NULL, 0, { 0, }, 0 },
    { "kommando", "random_box_west_58", NULL, 0, { 0, }, 0 },
    { "cz75", "random_box_59", "box_token_cry_money_59", 1000, { 0, }, 0 },
    { "jade", "random_box_60", NULL, 0, { 0, }, 0 },
    { "svu-as", "random_box_61", "box_token_cry_money_61", 1000, { 0, }, 0 },
    { "scarlet", "random_box_62", NULL, 0, { 0, }, 0 },

    { "zsd-scarh", "random_box_65", NULL, 0, { 0, }, 0 },
    { "zsd-scarh-c", "random_box_65_crown", NULL, 0, { 0, }, 0 },
    { "zsd-bullpup", "random_box_66", NULL, 0, { 0, }, 0 },
    { "zsd-bullpup-c", "random_box_66_crown", NULL, 0, { 0, }, 0 },
    { "zsd-xm8", "random_box_67", NULL, 0, { 0, }, 0 },
    { "zsd-xm8-c", "random_box_67_crown", NULL, 0, { 0, }, 0 },
    { "zsd-glock-18c", "random_box_68", NULL, 0, { 0, }, 0 },
    { "zsd-glock-18c-c", "random_box_68_crown", NULL, 0, { 0, }, 0 },
    { "zsd-axe", "random_box_69", NULL, 0, { 0, }, 0 },
    { "zsd-axe-c", "random_box_69_crown", NULL, 0, { 0, }, 0 },
    { "zsd-scout", "random_box_70", NULL, 0, { 0, }, 0 },
    { "zsd-scout-c", "random_box_70_crown", NULL, 0, { 0, }, 0 },

    { "pecheneg", "random_box_72", "box_token_cry_money_72", 1000, { 0, }, 0 },
    { "mag7", "random_box_73", "box_token_cry_money_73", 1000, { 0, }, 0 },
    { "crazyhorse", "random_box_74", "box_token_cry_money_74", 1000, { 0, }, 0 },
    { "zsd-atf", "random_box_75", NULL, 0, { 0, }, 0 },
    { "zsd-atf-c", "random_box_75_crown", NULL, 0, { 0, }, 0 },
    { "magpul", "random_box_76", "box_token_cry_money_76", 1000, { 0, }, 0 },
    { "frzn-m16a3", "random_box_77", NULL, 0, { 0, }, 0 },
    { "frzn-fabarm-stf", "random_box_78", NULL, 0, { 0, }, 0 },
    { "frzn-honey", "random_box_79", NULL, 0, { 0, }, 0 },
    { "frzn-sig-sauer", "random_box_80", NULL, 0, { 0, }, 0 },
    { "frzn-scout", "random_box_81", NULL, 0, { 0, }, 0 },
    { "frzn-axe", "random_box_82", NULL, 0, { 0, }, 0 },
    { "goldsmoke", "random_box_skin_83", NULL, 0, { 0, }, 0 },
    { "es-six12", "random_box_84", NULL, 0, { 0, }, 0 },
    { "es-six12-c", "random_box_84_crown", NULL, 0, { 0, }, 0 },
    { "es-acr-cqb", "random_box_85", NULL, 0, { 0, }, 0 },
    { "es-acr-cqb-c", "random_box_85_crown", NULL, 0, { 0, }, 0 },
    { "es-mcmillan", "random_box_86", NULL, 0, { 0, }, 0 },
    { "es-mcmillan-c", "random_box_86_crown", NULL, 0, { 0, }, 0 },
    { "es-glock-18c", "random_box_87", NULL, 0, { 0, }, 0 },
    { "es-glock-18c-c", "random_box_87_crown", NULL, 0, { 0, }, 0 },
    { "es-machete", "random_box_88", NULL, 0, { 0, }, 0 },
    { "es-type97", "random_box_89", NULL, 0, { 0, }, 0 },
    { "es-type97-c", "random_box_89_crown", NULL, 0, { 0, }, 0 },
    { "arx160", "random_box_90", "box_token_cry_money_90", 1000, { 0, }, 0 },
    { "dp12", "random_box_91", "box_token_cry_money_91", 1000, { 0, }, 0 },
    { "pm84", "random_box_92", "box_token_cry_money_92", 1000, { 0, }, 0 },
    { "scorpion-evo", "random_box_93", "box_token_cry_money_93", 1000, { 0, }, 0 },
    { "bushmaster", "random_box_94", "box_token_cry_money_94", 1000, { 0, }, 0 },
    { "swmp-r8", "random_box_95", "box_token_cry_money_95", 1000, { 0, }, 0 },
    { "rpd-custom", "random_box_96", "box_token_cry_money_96", 1000, { 0, }, 0 },
    { "type97as", "random_box_97", "box_token_cry_money_97", 1000, { 0, }, 0 },
    { "winchester", "random_box_98", "box_token_cry_money_98", 1000, { 0, }, 0 },
    { "fabarm-pss10", "random_box_99", "box_token_cry_money_99", 1000, { 0, }, 0 },

    { "desert", "random_box_101", "box_token_cry_money_101", 1000, { 0, }, 0 },

    { "radon", "random_box_103", "box_token_cry_money_103", 1000, { 0, }, 0 },
    { "m1911a1", "random_box_104", "box_token_cry_money_104", 1000, { 0, }, 0 },
    { "s60b3", "random_box_105", "box_token_cry_money_105", 1000, { 0, }, 0 },

    { "fnfal", "random_box_113", "box_token_cry_money_113", 1000, { 0, }, 0 },

    { "qsz92", "random_box_120", "box_token_cry_money_120", 1000, { 0, }, 0 },
    { "derya", "random_box_121", "box_token_cry_money_121", 1000, { 0, }, 0 },
    { "veresk", "random_box_122", "box_token_cry_money_122", 1000, { 0, }, 0 },

    { "micro-roni", "box_smg43", "box_token_cry_money_smg43", 1000, { 0, }, 0 },
    { "fostech", "box_weapon_shg45", "box_token_cry_money_shg45", 1000, { 0, }, 0 },
    { "remington-msr", "box_sr44", "box_token_cry_money_sr44", 1000, { 0, }, 0 },
    { "dp27", "box_mg24", "box_token_cry_money_mg24", 1000, { 0, }, 0 },
    { "taurus", "box_pt35", "box_token_cry_money_pt35", 1000, { 0, }, 0 },
    { "parrot", "box_pt37", "box_token_cry_money_pt37", 1000, { 0, }, 0 },
    { "parrot-c", "box_crown_pt37", "box_token_cry_money_pt37", 1000, { 0, }, 0 },
    { "spike", "box_shg46", "box_token_cry_money_shg46", 1000, { 0, }, 0 },
    { "balisong", "box_kn19", "box_token_cry_money_kn19", 1000, { 0, }, 0 },
    { "harms", "box_ar32", "box_token_cry_money_ar32", 1000, { 0, }, 0 },
    { "harms-c", "box_crown_ar32", "box_token_cry_money_ar32", 1000, { 0, }, 0 },
    { "hcar", "box_ar33", "box_token_cry_money_ar33", 1000, { 0, }, 0 },
    { "hcar-c", "box_crown_ar33", "box_token_cry_money_ar33", 1000, { 0, }, 0 },
    { "ung12", "box_shg47", "box_token_cry_money_shg47", 1000, { 0, }, 0 },
    { "ung12-c", "box_crown_shg47", "box_token_cry_money_shg47", 1000, { 0, }, 0 },
    { "hkg28", "box_sr45", "box_token_cry_money_sr45", 1000, { 0, }, 0 },
    { "hkg28-c", "box_crown_sr45", "box_token_cry_money_sr45", 1000, { 0, }, 0 },
    { "maxim", "box_pt38", "box_token_cry_money_pt38", 1000, { 0, }, 0 },
    { "maxim-c", "box_crown_pt38", "box_token_cry_money_pt38", 1000, { 0, }, 0 },
    { "marlin", "box_shg50", "box_token_cry_money_shg50", 1000, { 0, }, 0 },
    { "marlin-c", "box_crown_shg50", "box_token_cry_money_shg50", 1000, { 0, }, 0 },
    { "lwrc", "box_smg45", "box_token_cry_money_smg45", 1000, { 0, }, 0 },
    { "lwrc-c", "box_crown_smg45", "box_token_cry_money_smg45", 1000, { 0, }, 0 },
    { "mpx", "box_mpx", "box_token_cry_money_smg46", 1000, { 0, }, 0 },
    { "mpx-c", "box_crown_mpx", "box_token_cry_money_smg46", 1000, { 0, }, 0 },
    { "tommy", "box_smg47", "box_token_cry_money_smg47", 1000, { 0, }, 0 },
    { "tommy-c", "box_crown_smg47", "box_token_cry_money_smg47", 1000, { 0, }, 0 },
    { "stoner", "box_mg25", "box_token_cry_money_mg25", 1000, { 0, }, 0 },
    { "stoner-c", "box_crown_mg25", "box_token_cry_money_mg25", 1000, { 0, }, 0 },
    { "ak12", "box_ar35", "box_token_cry_money_ar35", 1000, { 0, }, 0 },
    { "ak12-c", "box_crown_ar35", "box_token_cry_money_ar35", 1000, { 0, }, 0 },
    { "truvelo", "box_sr47", "box_token_cry_money_sr47", 1000, { 0, }, 0 },
    { "truvelo-c", "box_crown_sr47", "box_token_cry_money_sr47", 1000, { 0, }, 0 },
    { "ts12", "box_shg52", "box_token_cry_money_shg52", 1000, { 0, }, 0 },
    { "ts12-c", "box_crown_shg52", "box_token_cry_money_shg52", 1000, { 0, }, 0 },
    { "tec9", "box_pt36", "box_token_cry_money_pt36", 1000, { 0, }, 0 },
    { "tec9-c", "box_crown_pt36", "box_token_cry_money_pt36", 1000, { 0, }, 0 },
    { "f90", "box_ar36", "box_token_cry_money_ar36", 1000, { 0, }, 0 },
    { "f90-c", "box_crown_ar36", "box_token_cry_money_ar36", 1000, { 0, }, 0 },
    { "tomahawk", "box_kn21", "box_token_cry_money_kn21", 1000, { 0, }, 0 },
    { "tomahawk-c", "box_crown_kn21", "box_token_cry_money_kn21", 1000, { 0, }, 0 },
    { "sv98", "box_sr48", "box_token_cry_money_sr48", 1000, { 0, }, 0 },
    { "sv98-c", "box_crown_sr48", "box_token_cry_money_sr48", 1000, { 0, }, 0 },
    { "kraken", "box_sr46", "box_token_cry_money_sr46", 1000, { 0, }, 0 },
    { "typhoon", "box_shg51", "box_token_cry_money_shg51", 1000, { 0, }, 0 },
    { "asval", "box_ar23", "box_token_cry_money_ar23", 1000, { 0, }, 0 },
    { "am17", "box_smg49", "box_token_cry_money_smg49", 1000, { 0, }, 0 },
    { "am17-c", "box_crown_smg49", "box_token_cry_money_smg49", 1000, { 0, }, 0 },

    { "rad-arx160", "random_box_rad01_ar27", NULL, 0, { 0, }, 0 },
    { "rad-katana", "random_box_rad01_kn13", NULL, 0, { 0, }, 0 },
    { "rad-axe", "random_box_rad01_kn16", NULL, 0, { 0, }, 0 },
    { "rad-pecheneg", "random_box_rad01_mg22", NULL, 0, { 0, }, 0 },
    { "rad-m1911a1", "random_box_rad01_pt16", NULL, 0, { 0, }, 0 },
    { "rad-swmp-r8", "random_box_rad01_pt33", NULL, 0, { 0, }, 0 },
    { "rad-mag7", "random_box_rad01_shg40", NULL, 0, { 0, }, 0 },
    { "rad-dp12", "random_box_rad01_shg42", NULL, 0, { 0, }, 0 },
    { "rad-honey", "random_box_rad01_smg33", NULL, 0, { 0, }, 0 },
    { "rad-scorpion-evo", "random_box_rad01_smg38", NULL, 0, { 0, }, 0 },
    { "rad-ax308", "random_box_rad01_sr31", NULL, 0, { 0, }, 0 },
    { "rad-remington-r11", "random_box_rad01_sr33", NULL, 0, { 0, }, 0 },

    { "ice-arx160", "box_ice01_ar27", NULL, 0, { 0, }, 0 },
    { "ice-parrot", "box_ice01_pt37", NULL, 0, { 0, }, 0 },
    { "ice-dp12", "box_ice01_shg42", NULL, 0, { 0, }, 0 },
    { "ice-scorpion", "box_ice01_smg38", NULL, 0, { 0, }, 0 },
    { "ice-bushmaster", "box_ice01_sr42", NULL, 0, { 0, }, 0 },

    { "jp-hcar", "box_jp01_ar33", NULL, 0, { 0, }, 0 },
    { "jp-katana", "box_jp01_kn13", NULL, 0, { 0, }, 0 },
    { "jp-balisong", "box_jp01_kn19", NULL, 0, { 0, }, 0 },
    { "jp-rpd-custom", "box_jp01_mg23", NULL, 0, { 0, }, 0 },
    { "jp-m1911a1", "box_jp01_pt16", NULL, 0, { 0, }, 0 },
    { "jp-swmp-r8", "box_jp01_pt33", NULL, 0, { 0, }, 0 },
    { "jp-fabarm-xlr", "box_jp01_shg38", NULL, 0, { 0, }, 0 },
    { "jp-mag7", "box_jp01_shg40", NULL, 0, { 0, }, 0 },
    { "jp-acrcqb", "box_jp01_smg25", NULL, 0, { 0, }, 0 },
    { "jp-scorpion", "box_jp01_smg38", NULL, 0, { 0, }, 0 },
    { "jp-crazyhorse", "box_jp01_sr38", NULL, 0, { 0, }, 0 },
    { "jp-remington-msr", "box_jp01_sr44", NULL, 0, { 0, }, 0 },

    { "santa-benelli", "random_box_santa_01", NULL, 0, { 0, }, 0 },
    { "santa-ak103", "random_box_santa_02", NULL, 0, { 0, }, 0 },
    { "santa-bizon", "random_box_santa_03", NULL, 0, { 0, }, 0 },
    { "santa-dsr", "random_box_santa_04", NULL, 0, { 0, }, 0 },
    { "santa-fiveseven", "random_box_santa_05", NULL, 0, { 0, }, 0 },
    { "santa-vhs", "box_xmas04_ar31", NULL, 0, { 0, }, 0 },
    { "santa-bowie", "box_xmas04_kn18", NULL, 0, { 0, }, 0 },
    { "santa-mpa", "box_xmas04_pt34", NULL, 0, { 0, }, 0 },
    { "santa-hgc-custom", "box_xmas04_shg41", NULL, 0, { 0, }, 0 },
    { "santa-uzi-pro", "box_xmas04_smg42", NULL, 0, { 0, }, 0 },
    { "santa-gepard", "box_xmas04_sr43", NULL, 0, { 0, }, 0 },

    { "frost-saigry", "box_xmas05_ar28", NULL, 0, { 0, }, 0 },
    { "frost-bowie", "box_xmas05_kn18", NULL, 0, { 0, }, 0 },
    { "frost-mpa", "box_xmas05_pt34", NULL, 0, { 0, }, 0 },
    { "frost-sap8", "box_xmas05_shg48", NULL, 0, { 0, }, 0 },
    { "frost-uzi-pro", "box_xmas05_smg42", NULL, 0, { 0, }, 0 },
    { "frost-gepard", "box_xmas05_sr43", NULL, 0, { 0, }, 0 },

    { "dragon-black", "box_dragon_black", NULL, 0, { 0, }, 0 },
    { "dragon-jade", "box_dragon_jade", NULL, 0, { 0, }, 0 },
    { "dragon-red", "box_dragon_red", NULL, 0, { 0, }, 0 },

    { "skins-asval", "random_box_skin_01", NULL, 0, { 0, }, 0 },
    { "skins-utas", "random_box_skin_02", NULL, 0, { 0, }, 0 },
    { "skins-mxstorm", "random_box_skin_03", NULL, 0, { 0, }, 0 },
    { "skins-m98b", "random_box_skin_04", NULL, 0, { 0, }, 0 },
    { "skins-m1911a1", "random_box_skin_05", NULL, 0, { 0, }, 0 },
    { "anubis-ak103", "random_box_skin_06", NULL, 0, { 0, }, 0 },
    { "anubis-alpine", "random_box_skin_07", NULL, 0, { 0, }, 0 },
    { "anubis-bizon", "random_box_skin_08", NULL, 0, { 0, }, 0 },
    { "anubis-vepr", "random_box_skin_09", NULL, 0, { 0, }, 0 },
    { "anubis-python", "random_box_skin_10", NULL, 0, { 0, }, 0 },
    { "skin-bird-ak103", "random_box_skin_11", NULL, 0, { 0, }, 0 },
    { "skin-bird-bullpup", "random_box_skin_12", NULL, 0, { 0, }, 0 },
    { "skin-bird-ctar", "random_box_skin_13", NULL, 0, { 0, }, 0 },
    { "skin-bird-mcmillan", "random_box_skin_14", NULL, 0, { 0, }, 0 },
    { "skin-bird-glock-18c", "random_box_skin_15", NULL, 0, { 0, }, 0 },
    { "skin-bird-axe", "random_box_skin_16", NULL, 0, { 0, }, 0 },
    { "skins-ak47", "random_box_skin_17", NULL, 0, { 0, }, 0 },
    { "skin-bird-m16a3", "random_box_skin_18", NULL, 0, { 0, }, 0 },
    { "anubis-radon", "random_box_skin_19", NULL, 0, { 0, }, 0 },
    { "anubis-radon-c", "random_box_skin_19_crown", NULL, 0, { 0, }, 0 },
    { "anubis-venom", "random_box_skin_20", NULL, 0, { 0, }, 0 },
    { "anubis-venom-c", "random_box_skin_20_crown", NULL, 0, { 0, }, 0 },
    { "anubis-orsis", "random_box_skin_21", NULL, 0, { 0, }, 0 },
    { "anubis-orsis-c", "random_box_skin_21_crown", NULL, 0, { 0, }, 0 },
    { "anubis-pp2000", "random_box_skin_22", NULL, 0, { 0, }, 0 },
    { "anubis-pp2000-c", "random_box_skin_22_crown", NULL, 0, { 0, }, 0 },
    { "anubis-walther", "random_box_skin_23", NULL, 0, { 0, }, 0 },
    { "zsd03", "random_box_zsd03_01", NULL, 0, { 0, }, 0 },
    { "zsd03-c", "random_box_zsd03_01_crown", NULL, 0, { 0, }, 0 },
    { "skins-balisong", "box_skin_kn19", NULL, 0, { 0, }, 0 },
    { "skins-pecheneg", "box_skin_mg22", NULL, 0, { 0, }, 0 },
    { "skins-parrot", "box_skin_pt32", NULL, 0, { 0, }, 0 },
    { "skins-sat", "box_skin_shg35", NULL, 0, { 0, }, 0 },
    { "skins-scorpion", "box_skin_smg38", NULL, 0, { 0, }, 0 },
    { "skins-ax308", "box_skin_sr31", NULL, 0, { 0, }, 0 },

    { "ec-kommando", "box_ec02_kn17", NULL, 0, { 0, }, 0 },
    { "ec-scarl", "box_ec02_smg31", NULL, 0, { 0, }, 0 },
    { "ec-mossberg", "box_ec02_shg08", NULL, 0, { 0, }, 0 },
    { "ec-mcmillan", "box_ec02_sr09", NULL, 0, { 0, }, 0 },
    { "ec-enfield", "box_ec02_ar25", NULL, 0, { 0, }, 0 },
    { "ec-cz75", "box_ec02_pt32", NULL, 0, { 0, }, 0 },

    { "cny-acr", "box_cny01_ar20", NULL, 0, { 0, }, 0 },
    { "cny-marine", "box_cny01_kn02", NULL, 0, { 0, }, 0 },
    { "cny-eagle", "box_cny01_pt01", NULL, 0, { 0, }, 0 },
    { "cny-cobra", "box_cny01_shg11", NULL, 0, { 0, }, 0 },
    { "cny-stf", "box_cny01_shg37", NULL, 0, { 0, }, 0 },
    { "cny-daewoo-k1", "box_cny01_smg12", NULL, 0, { 0, }, 0 },
    { "cny-acrcqb", "box_cny01_smg25", NULL, 0, { 0, }, 0 },
    { "cny-as50", "box_cny01_sr04", NULL, 0, { 0, }, 0 },

    { "crown-set1", "random_box_crown_01", NULL, 0, { 0, }, 0 },

    { "black-friday", "random_box_bf_01", NULL, 0, { 0, }, 0 },

    { "ps4-es", "box_ps4_01", NULL, 0, { 0, }, 0 },
    { "ps4-ice", "box_ps4_02", NULL, 0, { 0, }, 0 },
    { "ps4-anubis", "box_ps4_03", NULL, 0, { 0, }, 0 },
    { "ps4-zsd", "box_ps4_04", NULL, 0, { 0, }, 0 },
    { "ps4-rad", "box_ps4_05", NULL, 0, { 0, }, 0 },
    { "ps4-secondary", "box_ps4_06", NULL, 0, { 0, }, 0 },
    { "ps4-main", "box_ps4_07", NULL, 0, { 0, }, 0 },

    { "ps4-radon", "box_ps4_01_ar26", NULL, 0, { 0, }, 0 },
    { "ps4-saigry", "box_ps4_01_ar28", NULL, 0, { 0, }, 0 },
    { "ps4-fnfal", "box_ps4_01_ar30", NULL, 0, { 0, }, 0 },
    { "ps4-axe", "box_ps4_01_kn16", NULL, 0, { 0, }, 0 },
    { "ps4-walther", "box_ps4_01_pt31", NULL, 0, { 0, }, 0 },
    { "ps4-mpa", "box_ps4_01_pt34", NULL, 0, { 0, }, 0 },
    { "ps4-fabarm-pss10", "box_ps4_01_shg43", NULL, 0, { 0, }, 0 },
    { "ps4-ung12", "box_ps4_01_shg47", NULL, 0, { 0, }, 0 },
    { "ps4-xm8", "box_ps4_01_smg08", NULL, 0, { 0, }, 0 },
    { "ps4-hkmp5a5", "box_ps4_01_smg39", NULL, 0, { 0, }, 0 },
    { "ps4-dsa-spr", "box_ps4_01_sr32", NULL, 0, { 0, }, 0 },
    { "ps4-remington-r11", "box_ps4_01_sr33", NULL, 0, { 0, }, 0 },
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
