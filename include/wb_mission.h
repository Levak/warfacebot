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

#ifndef WB_MISSION_H
# define WB_MISSION_H

# include <wb_list.h>

struct mission
{
    char *mission_key;
    int no_team;
    char *name;
    char *setting;
    char *mode;
    char *mode_name;
    char *mode_icon;
    char *description;
    char *image;
    char *difficulty;
    char *type;
    char *time_of_day;
    unsigned int crown_reward_gold;
    unsigned int crown_time_gold;
    unsigned int crown_perf_gold;
};

struct list *mission_list_new(void);

#endif /* !WB_MISSION_H */
