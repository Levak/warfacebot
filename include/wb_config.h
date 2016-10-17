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

#ifndef WB_CONFIG_H
# define WB_CONFIG_H

struct rating_curve_step
{
  int points_required;
  int adjustment;
  char *icon;
  char *description;
};

struct rating_reward
{
  int rating_level;
  char *name;
};

struct abuse_report_type
{
  char *name;
  char *message;
};

struct login_reward
{
  unsigned int id;
  char *name;
};

struct login_streak
{
  unsigned int id;

  /** struct login_reward */
  struct list *rewards;
};

struct profile_progression_event
{
  unsigned int id;
  char silent;

  /* Event */
  char *type;
  char *tutorial_passed;
  int rank_reached;
  int max_value;
  int pass_value;

  /* Reward */
  char *unlock_type;
  char *unlock_class;
  char *special_reward;
};

enum special_reward_type
{
  SPECIAL_REWARD_ITEM,
  SPECIAL_REWARD_ACHIEVEMENT,
  SPECIAL_REWARD_MONEY
};

struct special_reward
{
  unsigned int id;
  enum special_reward_type type;
  char use_notification;
};


struct special_reward_item
{
  struct special_reward base;

  char *name;

  char *expiration;

  unsigned int max_amount;
  unsigned int amount;
};

struct special_reward_achievement
{
  struct special_reward base;

  unsigned int id;
  unsigned int progress;
};

struct special_reward_money
{
  struct special_reward base;

  char *currency;
  unsigned int amount;
};

struct special_reward_event
{
  char *name;

  /** struct special_reward */
  struct list *rewards;
};

struct game_config
{
  struct
  {
    char enabled;

    struct
    {
      unsigned int step;
      unsigned int leave_penalty;
      unsigned int top_rating_capacity;

      struct
      {
        char enabled;
        unsigned int bonus_amount;
        unsigned int start_from_streak;
        unsigned int apply_below_rating;
      } win_streak;

      /** struct rating_curve_step */
      struct list *steps;

    } curve;

    struct
    {
      char enabled;
      char *season_id_template;
      char *banner;
      char *description;
      char *rules;
      char *announcement_end_date;
      char *games_end_date;

      struct
      {
        /** struct rating_reward */
        struct list *rank;

        /** struct rating_reward */
        struct list *season;
      } rewards;
    } rule;

  } rating;

  struct
  {
    struct
    {
      int reports_per_day;
      int reports_per_player;
    } limits;

    /** struct abuse_report_type */
    struct list *report_types;

  } abuse;

  struct login_bonus_config
  {

    char enabled;
    int use_notification;
    char *schedule;
    char *expiration;

    /** struct login_streak */
    struct list *streaks;

  } consecutive_login_bonus, consecutive_login_bonus_holiday;

  struct
  {

    char enabled;

    /** struct profile_progression_event */
    struct list *events;

  } profile_progression;

  struct
  {

    /** struct special_reward_event */
    struct list *events;

  } special_rewards;

  struct
  {
    struct vote_config
    {
      unsigned int can_be_started_after_sec;
      unsigned int cooldown_sec;
      unsigned int timeout_sec;
      float success_threshold;
    } kick, surrender;
  } votes;

  struct
  {

  } regions;
};

#endif /* !WB_CONFIG_H */
