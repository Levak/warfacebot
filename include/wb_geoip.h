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

#ifndef WB_GEOIP_H
# define WB_GEOIP_H

struct geoip
{
    char *ip;
    char *country_code;
    char *country_name;
    char *region_code;
    char *region_name;
    char *city;
    char *zip_code;
    char *time_zone;
    char *latitude;
    char *longitude;
    char *metro_code;
    char *isp;
};

struct geoip *geoip_get_info(const char *ip, int full);
void geoip_free(struct geoip *g);


#endif /* !WB_GEOIP_H */
