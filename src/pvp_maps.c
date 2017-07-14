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

#include <wb_list.h>
#include <wb_pvp_maps.h>

#include <string.h>
#include <stdlib.h>

static void add(struct list *missions,
                const char *name, const char *uid)
{
    struct mission *map = calloc(sizeof(struct mission), 1);

    map->mission_key = strdup(uid);
    map->no_team = 0;
    map->name = strdup(name);
    map->setting = NULL;
    map->mode = strdup("pvp");
    map->mode_name = NULL;
    map->mode_icon = NULL;
    map->description = NULL;
    map->image = NULL;
    map->difficulty = NULL;
    map->type = strdup(name);
    map->time_of_day = NULL;
    map->crown_reward_gold = 0;
    map->crown_perf_gold = 0;
    map->crown_time_gold = 0;

    list_add(missions, map);
}

void pvp_maps_add_to_list(struct list *ml)
{
    add(ml, "ctf_breach", "45188feb-0017-46ec-bb57-189dbbd6be2d");
    add(ml, "ctf_construction", "73904867-1107-4db2-a7eb-4593d580f835");
    add(ml, "ctf_convoy", "f32bc4ca-6420-4034-82d6-c8041e3bd3fa");
    add(ml, "ctf_deposit", "d5da06ce-a425-4013-abd8-6f9bfbe9f763");
    add(ml, "ctf_longway", "7684fdcf-f6aa-431e-9616-608eccd3a847");
    add(ml, "ctf_quarry", "f32bc8e2-06df-413a-935a-b039ee4e4467");
    add(ml, "ctf_test", "6f7644a3-1a37-4cbb-ab01-128a1015b64f");
    add(ml, "ctf_vault_fwc", "dfeebeda-783e-4fe2-98fa-a76bf35b31b9");
    add(ml, "ctf_vault", "60c04653-a419-433f-a43b-63278d8bd339");
    add(ml, "dmn_armageddon", "e1129d41-42fd-4451-b729-4b7ff5f4e3e7");
    add(ml, "dmn_downtown", "0726fb42-1d41-7a43-97d1-4e0571124cd8");
    add(ml, "dmn_sirius", "70e29d78-bb85-4014-8989-ffeb9074d2bc");
    add(ml, "dmn_subzero", "d0af9f60-6402-11c4-8d37-8c89a553425b");
    add(ml, "dst_afghan", "d1f15bf2-1db4-42f4-b335-097a57b83ed4");
    add(ml, "dst_lighthouse", "ea0cbac2-5891-4d37-bb42-09e790730b34");
    add(ml, "ffa_bunker", "e6e96230-b938-11e5-b925-8c89a554425b");
    add(ml, "ffa_downtown", "cccea92f-612d-2850-1111-3e02a3a54d6c");
    add(ml, "ffa_forest_dawn", "fb53bbbb-f555-45aa-9144-8442446b1c4f");
    add(ml, "ffa_forest", "227123ef-b394-4321-b697-abed0fff9e38");
    add(ml, "ffa_motel", "dda0b8ac-de2e-9474-84fc-5b4ec51bf7ef");
    add(ml, "ffa_overpass", "52fa65a0-d182-4abb-97c2-54857c6b3bbc");
    add(ml, "ffa_train", "c61ea94f-505d-4860-9199-3e02a1a54d9c");
    add(ml, "ffa_widestreet", "45e182f3-d45c-4e5d-8054-fc4bbbfc2782");
    add(ml, "hnt_africa", "2124463b-0612-4882-a5de-b222b21bf0e6");
    add(ml, "hnt_night_motel", "959ddc2d-9bb4-4eab-9be8-8e04e76d94aa");
    add(ml, "hnt_winter", "d34538bc-f603-4f39-afe5-db27000564cd");
    add(ml, "hnt_winter_xmas", "bee96351-e973-4222-a145-5a68a6773bba");
    add(ml, "ptb_afghan", "123bec5b-a1c2-4bab-b105-3b5b89bab014");
    add(ml, "ptb_bridges", "3c3c5677-02df-4b87-91b1-8a61bcb16bcf");
    add(ml, "ptb_D17", "cdb2fe18-c99e-56b0-9e7f-ffc1d7733f53");
    add(ml, "ptb_destination", "543df61d-f277-4953-9e0f-3eb8c998fa3b");
    add(ml, "ptb_district", "8a0f1f90-c1b5-4235-baef-81318bb6e519");
    add(ml, "ptb_factory", "77e4f737-431c-4359-b27f-4800c5a5b780");
    add(ml, "ptb_mine", "e00c96b8-8185-4cbf-9bd5-e70520021575");
    add(ml, "ptb_overpass", "a2e98ff0-b602-44e5-b517-02518a7c19eb");
    add(ml, "ptb_palace", "afcf6636-b5a8-4eec-ac24-fa0915695a15");
    add(ml, "ptb_pyramid", "b1594650-15ad-43c0-850f-863f5ab85ef5");
    add(ml, "ptb_trailerpark", "3f57cdb0-010a-11e5-a726-8c99a553325b");
    add(ml, "stm_blackgold", "517d8a3f-91f8-4566-81a2-b48a70e44014");
    add(ml, "stm_blackmamba", "ee01cd3a-d214-48fa-843b-788427a6161c");
    add(ml, "stm_highhill", "2a8cbd01-52ae-4783-bdd6-d2f858b4fd74");
    add(ml, "stm_invasion", "18a2580c-5988-4890-96a9-b7b40c5487c4");
    add(ml, "stm_wharf", "cd54d2eb-f00e-4ccc-bbd4-d4c0f2cc935e");
    add(ml, "tdm_airbase", "e4f56e59-97fb-4451-a137-7b44026d96b0");
    add(ml, "tdm_aul_pts", "24531004-8e3f-48e3-a760-59f914c02c0d");
    add(ml, "tdm_aul", "bbb33b77-98db-463c-9a1a-d27655d90690");
    add(ml, "tdm_codenameD18", "f405283f-42f9-4d5e-b02a-618cacd10cf4");
    add(ml, "tdm_crossriver", "15144c77-4aaa-4d33-a88b-06eee80fd7f7");
    add(ml, "tdm_dock", "51a9ebbc-8519-4f5e-b83e-0e9911c6995c");
    add(ml, "tdm_downtown", "0726fb17-1d35-4a43-97d1-4e0563794cd8");
    add(ml, "tdm_farm_hw", "400e7fee-3fda-11e4-baf7-d43d7e9be5ba");
    add(ml, "tdm_farm_sunset", "90c7e3c1-6432-4745-8abc-40cbccaa4d25");
    add(ml, "tdm_farm", "30afabfd-032f-4ff3-a22e-8b889349aaa5");
    add(ml, "tdm_grand_bazaar", "3347d361-ef36-4c57-8bca-4f8051fc97e6");
    add(ml, "tdm_hangar_up", "e5981b6a-325d-42eb-a3fe-e6eed0bc4bf2");
    add(ml, "tdm_hangar_xmas", "953740a3-022e-4243-9044-eb1c07a2f680");
    add(ml, "tdm_hangar", "d42df33c-bbaa-49d9-bed2-8db61ef5b233");
    add(ml, "tdm_motel", "71e863e1-e28c-489d-b145-866987fbe031");
    add(ml, "tdm_oildepot_e16", "0575ee9b-db2a-4a8d-a376-c2ec4b021587");
    add(ml, "tdm_oildepotv3", "1d71c946-7ffc-4045-8035-14eea4072e02");
    add(ml, "tdm_oildepot_xmas", "24578605-b482-4dcf-af0a-7a6968732602");
    add(ml, "tdm_residential", "805b24e5-f5b6-447c-b7a5-c82c6b0134cb");
    add(ml, "tdm_shuttle", "8702f1ee-bac1-4bea-a92b-d5b76c5cd5af");
    add(ml, "tdm_sirius", "c090ccd6-dfd4-46aa-a5a6-929d974451e1");
    add(ml, "tdm_streetwars", "d2cc9728-2a3f-4387-a0ea-ea1e0f967495");
    add(ml, "tbs_hawkrock", "791f8fd3-41de-4a94-a5d0-945b461f691e");
    add(ml, "tbs_waterfalling", "da20dcc4-9672-4077-b877-1b04c6772b75");
}
