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

#ifndef WB_SHOP_H
# define WB_SHOP_H

# include <wb_list.h>

enum offer_status
{
    OFFER_NORMAL,
    OFFER_SALE,
    OFFER_NEW,
};

struct repair_cost_item
{
    char *name;
    unsigned int repair_cost;
    unsigned int durability;
};

struct shop_offer
{
    unsigned int id;
    char *name;
    unsigned int supplier_id;
    enum offer_status offer_status;

    struct list *repairs;

    unsigned int quantity;
    char *expirationTime;

    unsigned int durabilityPoints;
    unsigned int discount_percent;

    unsigned int rank;

    struct {
        struct {
            unsigned int orig;
            unsigned int curr;
        } cry;
        struct {
            unsigned int orig;
            unsigned int curr;
        } crown;
        struct {
            unsigned int orig;
            unsigned int curr;
        } game;
        struct {
            unsigned int curr;
        } key;
    } price;
};

struct shop_offer *offer_list_get(const char *name);
struct shop_offer *offer_list_get_by_id(unsigned int id);
struct list *offer_list_new(void);
struct list *repair_list_new(void);

#endif /* !WB_SHOP_H */
