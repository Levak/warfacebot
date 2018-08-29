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

#include <cust_api.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <time.h>

void initAPI()
{
    mkdir("./data/", 0777);
    mkdir("./data/clanmates/", 0777);
    mkdir("./data/clanmates/lastseen/", 0777);
    setenv("TZ", "America/Sao_Paulo", 1);
    tzset();
}

void writeTsFile()
{
    FILE *f = fopen("./data/.ts", "w");
    if (f == NULL){
        printf("Error opening ./data/.ts\n");
        return;
    }
    fprintf(f, "%d", (int)time(NULL));
    fclose(f);
}

void writeClanList(const char *query)
{
    writeTsFile();
    FILE *f = fopen("./data/clan_list.xml", "w");
    if (f == NULL){
        printf("Error opening ./data/clan_list.xml\n");
        return;
    }
    fprintf(f, "<?xml version='1.0' encoding='UTF-8'?>%s", query);
    fclose(f);
}

void writeClanmateInfo(const char *nickname,const char *query)
{
    char filename[48];
    snprintf(filename, sizeof(char) * 48, "./data/clanmates/%s.xml", nickname);
    FILE *f = fopen(filename, "w");
    if (f == NULL){
        printf("Error opening %s\n",filename);
        return;
    }
    fprintf(f, "<?xml version='1.0' encoding='UTF-8'?>%s", query);
    fclose(f);
}

void writeClanmateLastSeen(const char *profile_id,int last_seen)
{
    char filename[48];
    snprintf(filename, sizeof(char) * 48, "./data/clanmates/lastseen/%s", profile_id);
    FILE *f = fopen(filename, "w");
    if (f == NULL){
        printf("Error opening %s\n",filename);
        return;
    }
    time_t t = last_seen;
    struct tm ts;
    char buf[64];
    localtime_r(&t, &ts);
    strftime(buf, sizeof (buf), "%d/%m/%Y %H:%M", &ts);
    fprintf(f, "%s", buf);
    fclose(f);
}
