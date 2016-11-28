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

#include <wb_tools.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <wb_config.h>
#include <wb_querycache.h>
#include <wb_list.h>
#include <wb_log.h>
#include <wb_cvar.h>

static int rating_curve_step_cmp(struct rating_curve_step *step, int points_required)
{
  return step->points_required - points_required;
}

static void rating_curve_step_free(struct rating_curve_step *step)
{
  free(step->icon);
  step->icon = NULL;
  free(step->description);
  step->description = NULL;

  free(step);
}

static void _parse_rating_curve(struct game_config *config, const char *elt)
{
  config->rating.curve.step =
    get_info_int(elt, "step='", "'", NULL);
  config->rating.curve.leave_penalty =
    get_info_int(elt, "leave_penalty='", "'", NULL);
  config->rating.curve.top_rating_capacity =
    get_info_int(elt, "top_rating_capacity='", "'", NULL);

  {
    char *win_streak_node = get_info(elt, "<win_streak", "/>", NULL);

    if (win_streak_node != NULL)
    {
      config->rating.curve.win_streak.enabled =
        get_info_int(win_streak_node, "enabled='", "'", NULL);
      config->rating.curve.win_streak.bonus_amount =
        get_info_int(win_streak_node, "bonus_amount='", "'", NULL);
      config->rating.curve.win_streak.start_from_streak =
        get_info_int(win_streak_node, "start_from_streak='", "'", NULL);
      config->rating.curve.win_streak.apply_below_rating =
        get_info_int(win_streak_node, "apply_below_rating='", "'", NULL);
    }

    free(win_streak_node);
  }

  config->rating.curve.steps = list_new((f_list_cmp) rating_curve_step_cmp,
                                        (f_list_free) rating_curve_step_free);

  const char *m = elt;
  while ((m = strstr(m, "<rating ")))
  {
    char *rating = get_info(m, "<rating ", "/>", NULL);

    struct rating_curve_step *step =
      calloc(1, sizeof (struct rating_curve_step));

    step->points_required =
      get_info_int(rating, "points_required='", "'", NULL);
    step->adjustment =
      get_info_int(rating, "adjustment='", "'", NULL);

    step->icon =
      get_info(rating, "icon='", "'", NULL);
    step->description =
      get_info(rating, "description='", "'", NULL);

    list_add(config->rating.curve.steps, step);

    free(rating);
    ++m;
  }
}


static int rating_reward_cmp(struct rating_reward *reward, int rating_level)
{
  return reward->rating_level - rating_level;
}

static void rating_reward_free(struct rating_reward *reward)
{
  free(reward->name);
  reward->name = NULL;

  free(reward);
}

static void _parse_rating_season_rule(struct game_config *config, const char *elt)
{
  config->rating.rule.enabled =
    get_info_int(elt, "enabled='", "'", NULL);

  config->rating.rule.season_id_template =
    get_info(elt, "season_id_template='", "'", NULL);
  config->rating.rule.banner =
    get_info(elt, "banner='", "'", NULL);
  config->rating.rule.description =
    get_info(elt, "description='", "'", NULL);
  config->rating.rule.rules =
    get_info(elt, "rules='", "'", NULL);
  config->rating.rule.announcement_end_date =
    get_info(elt, "announcement_end_date='", "'", NULL);
  config->rating.rule.games_end_date =
    get_info(elt, "games_end_date='", "'", NULL);

  {
    char *rating_achieved_rewards =
      get_info(elt,
               "<rating_achieved_rewards>",
               "</rating_achieved_rewards>",
               NULL);

    config->rating.rule.rewards.rank = list_new((f_list_cmp) rating_reward_cmp,
                                           (f_list_free) rating_reward_free);

    if (rating_achieved_rewards != NULL)
    {

      const char *m = rating_achieved_rewards;
      while ((m = strstr(m, "<reward ")))
      {
        char *rank = get_info(m, "<reward ", "/>", NULL);

        struct rating_reward *reward =
          calloc(1, sizeof (struct rating_reward));

        reward->rating_level =
          get_info_int(rank, "rating_level='", "'", NULL);
        reward->name =
          get_info(rank, "name='", "'", NULL);

        list_add(config->rating.rule.rewards.rank, reward);

        free(rank);
        ++m;
      }
    }

    free(rating_achieved_rewards);
  }

  {
    char *season_result_rewards =
      get_info(elt,
               "<season_result_rewards>",
               "</season_result_rewards>",
               NULL);

    config->rating.rule.rewards.season = list_new((f_list_cmp) rating_reward_cmp,
                                             (f_list_free) rating_reward_free);

    if (season_result_rewards != NULL)
    {

      const char *m = season_result_rewards;
      while ((m = strstr(m, "<reward ")))
      {
        char *season = get_info(m, "<reward ", "/>", NULL);

        struct rating_reward *reward =
          calloc(1, sizeof (struct rating_reward));

        reward->rating_level =
          get_info_int(season, "rating_level='", "'", NULL);
        reward->name =
          get_info(season, "name='", "'", NULL);

        list_add(config->rating.rule.rewards.season, reward);

        free(season);
        ++m;
      }
    }

    free(season_result_rewards);
  }
}


static void _parse_ratingseason(struct game_config *config, const char *elt)
{
  config->rating.enabled =
    get_info_int(elt, "enabled='", "'", NULL);
}


static int abuse_report_type_cmp(struct abuse_report_type *report, const char *name)
{
  return strcmp(report->name, name);
}

static void abuse_report_type_free(struct abuse_report_type *report)
{
  free(report->name);
  report->name = NULL;
  free(report->message);
  report->message = NULL;

  free(report);
}

static void _parse_abuse_manager_config(struct game_config *config, const char *elt)
{
  config->abuse.limits.reports_per_day =
    get_info_int(elt, "reports_per_day='", "'", NULL);
  config->abuse.limits.reports_per_player =
    get_info_int(elt, "reports_per_player='", "'", NULL);

  config->abuse.report_types = list_new((f_list_cmp) abuse_report_type_cmp,
                                        (f_list_free) abuse_report_type_free);

  const char *m = elt;
  while ((m = strstr(m, "<report_type ")))
  {
    char *report_type = get_info(m, "<report_type ", "/>", NULL);

    struct abuse_report_type *type =
      calloc(1, sizeof (struct abuse_report_type));

    type->name =
      get_info(report_type, "name='", "'", NULL);
    type->message =
      get_info(report_type, "message='", "'", NULL);

    list_add(config->abuse.report_types, type);

    free(report_type);
    ++m;
  }
}


static int login_reward_cmp(struct login_reward *reward, int id)
{
  return reward->id - id;
}

static void login_reward_free(struct login_reward *reward)
{
  free(reward->name);
  reward->name = NULL;

  free(reward);
}

static int login_streak_cmp(struct login_streak *streak, int id)
{
  return streak->id - id;
}

static void login_streak_free(struct login_streak *streak)
{
  if (streak->rewards != NULL)
    list_free(streak->rewards);
  streak->rewards = NULL;

  free(streak);
}

static void _parse_login_bonus(struct login_bonus_config *login_bonus, const char *elt)
{
  login_bonus->enabled =
    get_info_int(elt, "enabled='", "'", NULL);
  login_bonus->use_notification =
    get_info_int(elt, "use_notification='", "'", NULL);

  login_bonus->schedule =
    get_info(elt, "schedule='", "'", NULL);
  login_bonus->expiration =
    get_info(elt, "expiration='", "'", NULL);

  login_bonus->streaks =
    list_new((f_list_cmp) login_streak_cmp,
             (f_list_free) login_streak_free);

  unsigned int j = 0;
  const char *m2 = elt;
  while ((m2 = strstr(m2, "<streak>")))
  {
    char *streak_node =
      get_info(m2,
               "<streak>",
               "</streak>",
               NULL);

    struct login_streak *streak =
      calloc(1, sizeof (struct login_streak));

    streak->id = j++;
    streak->rewards = list_new((f_list_cmp) login_reward_cmp,
                               (f_list_free) login_reward_free);

    unsigned int i = 0;
    const char *m = streak_node;
    while ((m = strstr(m, "<reward ")))
    {
      char *reward_node = get_info(m, "<reward ", "/>", NULL);

      struct login_reward *reward =
        calloc(1, sizeof (struct login_reward));

      reward->id = i++;
      reward->name =
        get_info(reward_node, "name='", "'", NULL);

      list_add(streak->rewards, reward);

      free(reward_node);
      ++m;
    }

    list_add(login_bonus->streaks, streak);

    free(streak_node);
    ++m2;
  }
}

static void _parse_consecutive_login_bonus(struct game_config *config, const char *elt)
{
  _parse_login_bonus(&config->consecutive_login_bonus, elt);
}

static void _parse_consecutive_login_bonus_holiday(struct game_config *config, const char *elt)
{
  _parse_login_bonus(&config->consecutive_login_bonus_holiday, elt);
}



static int profile_progression_event_cmp(struct profile_progression_event *event, int id)
{
  return event->id - id;
}

static void profile_progression_event_free(struct profile_progression_event *event)
{
  free(event->type);
  event->type = NULL;
  free(event->tutorial_passed);
  event->tutorial_passed = NULL;
  free(event->unlock_type);
  event->unlock_type = NULL;
  free(event->unlock_class);
  event->unlock_class = NULL;
  free(event->special_reward);
  event->special_reward = NULL;

  free(event);
}

static void _parse_profile_progression_config(struct game_config *config, const char *elt)
{
  config->profile_progression.enabled =
    get_info_int(elt, "enabled='", "'", NULL);

  config->profile_progression.events =
    list_new((f_list_cmp) profile_progression_event_cmp,
             (f_list_free) profile_progression_event_free);

  unsigned int i = 0;
  const char *end = strstr(elt, "</profile_progression_config>");
  const char *m = elt + 1;
  while ((m = strstr(m, "<")) && m < end)
  {
    char *event_node = get_info(m, "<", "/>", NULL);

    struct profile_progression_event *event =
        calloc(1, sizeof (struct profile_progression_event));

    event->id = i++;
    event->silent =
      get_info_int(event_node, "silent='", "'", NULL);
    event->rank_reached =
      get_info_int(event_node, "rank_reached='", "'", NULL);
    event->max_value =
      get_info_int(event_node, "max_value='", "'", NULL);
    event->pass_value =
      get_info_int(event_node, "pass_value='", "'", NULL);

    event->type =
      get_info(event_node, "type='", "'", NULL);
    event->tutorial_passed =
      get_info(event_node, "tutorial_passed='", "'", NULL);
    event->unlock_class =
      get_info(event_node, "unlock_class='", "'", NULL);
    event->special_reward =
      get_info(event_node, "special_reward='", "'", NULL);

    list_add(config->profile_progression.events, event);

    m += strlen(event_node);
    free(event_node);
  }
}


static int special_reward_cmp(struct special_reward *reward, int id)
{
  return reward->id - id;
}

static void special_reward_free(struct special_reward *reward)
{
  switch (reward->type)
  {
    case SPECIAL_REWARD_ITEM:
    {
      struct special_reward_item *item =
        (struct special_reward_item *) reward;

      free(item->name);
      item->name = NULL;
      free(item->expiration);
      item->expiration = NULL;
      break;
    }

    case SPECIAL_REWARD_MONEY:
    {
      struct special_reward_money *money =
        (struct special_reward_money *) reward;

      free(money->currency);
      money->currency = NULL;
      break;
    }

    case SPECIAL_REWARD_ACHIEVEMENT:
    default:
      break;
  }

  free(reward);
}

static int special_reward_event_cmp(struct special_reward_event *event, const char *name)
{
  return strcmp(event->name, name);
}

static void special_reward_event_free(struct special_reward_event *event)
{
  if (event->rewards != NULL)
    list_free(event->rewards);
  event->rewards = NULL;

  free(event);
}

static void _parse_special_reward_configuration(struct game_config *config, const char *elt)
{

  config->special_rewards.events =
    list_new((f_list_cmp) special_reward_event_cmp,
             (f_list_free) special_reward_event_free);

  const char *m2 = elt;
  while ((m2 = strstr(m2, "<event>")))
  {
    char *event_node =
      get_info(m2,
               "<event ",
               "</event>",
               NULL);

    char *event_head =
      get_info(m2,
               "<event ",
               ">",
               NULL);

    struct special_reward_event *event =
      calloc(1, sizeof (struct special_reward_event));

    char use_notification = 1;
    if (NULL != strstr(event_head, "use_notification="))
      use_notification =
        get_info_int(event_head, "use_notification='", "'", NULL);
    else
      use_notification = 1;

    event->name =
      get_info(event_head, "name='", "'", NULL);

    event->rewards = list_new((f_list_cmp) special_reward_cmp,
                              (f_list_free) special_reward_free);

    unsigned int i = 0;
    const char *m = event_node;
    while ((m = strstr(m, "<")))
    {
      char *node = get_info(m, "<", "/>", NULL);
      char *node_name = get_info_first(m, "<", " />", NULL);

      char node_use_notification = 1;
      if (NULL != strstr(node, "use_notification="))
        node_use_notification =
          get_info_int(node, "use_notification='", "'", NULL);
      else
        node_use_notification = use_notification;

      struct special_reward *reward = NULL;

      if (0 == strcmp(node_name, "item"))
      {
        struct special_reward_item *item =
          calloc(1, sizeof (struct special_reward_item));

        reward = &item->base;
        item->base.type = SPECIAL_REWARD_ITEM;

        item->name = get_info(node, "name='", "'", NULL);
        item->expiration = get_info(node, "expiration='", "'", NULL);

        item->max_amount = get_info_int(node, "max_amount='", "'", NULL);
        item->amount = get_info_int(node, "amount='", "'", NULL);
      }
      else if (0 == strcmp(node_name, "money"))
      {
        struct special_reward_money *money =
          calloc(1, sizeof (struct special_reward_money));

        reward = &money->base;
        money->base.type = SPECIAL_REWARD_MONEY;

        money->currency = get_info(node, "currency='", "'", NULL);

        money->amount = get_info_int(node, "amount='", "'", NULL);
      }
      else if (0 == strcmp(node_name, "achievement"))
      {
        struct special_reward_achievement *achievement =
          calloc(1, sizeof (struct special_reward_achievement));

        reward = &achievement->base;
        achievement->base.type = SPECIAL_REWARD_ACHIEVEMENT;

        achievement->id = get_info_int(node, "id='", "'", NULL);

        achievement->progress = get_info_int(node, "progress='", "'", NULL);
      }
      else
      {
#ifdef DEBUG
        xprintf("Unhandled special reward '%s'\n'", node_name);
#endif /* DEBUG */
      }

      reward->use_notification = node_use_notification;
      reward->id = i++;

      list_add(event->rewards, reward);

      m += strlen(node);
      free(node);
      free(node_name);
    }

    list_add(config->special_rewards.events, event);

    m2 += strlen(event_node);
    free(event_node);
    free(event_head);
  }
}

static void _parse_vote(struct vote_config *vote, const char *elt)
{
  vote->can_be_started_after_sec =
    get_info_int(elt, "can_be_started_after_sec='", "'", NULL);
  vote->cooldown_sec =
    get_info_int(elt, "cooldown_sec='", "'", NULL);
  vote->timeout_sec =
    get_info_int(elt, "timeout_sec='", "'", NULL);
  vote->success_threshold =
    get_info_float(elt, "success_threshold='", "'", NULL);
}

static void _parse_votes(struct game_config *config, const char *elt)
{
  {
    char *kickvote_node = get_info(elt, "<kickvote", "/>", NULL);

    if (kickvote_node != NULL)
    {
      _parse_vote(&config->votes.kick, kickvote_node);
    }

    free(kickvote_node);
  }

  {
    char *surrender_node = get_info(elt, "<surrendervote", "/>", NULL);

    if (surrender_node != NULL)
    {
      _parse_vote(&config->votes.surrender, surrender_node);
    }

    free(surrender_node);
  }
}

static void _parse_regions(struct game_config *config, const char *elt)
{
  /* Nothing to do */

#ifdef DEBUG
  if (strlen(elt) > 0)
  {
    xprintf("FIXME: Unhandled config 'regions':\n%s\n", elt);
  }
#endif /* DEBUG */
}

static void _parse_config(struct querycache *cache,
                          const char *elt)
{
    struct game_config *config = (struct game_config *) cache->container;

    char *tag_name = get_info_first(elt, "<", " />", NULL);

    if (0 == strcmp(tag_name, "rating_curve"))
    {
      _parse_rating_curve(config, elt);
    }
    else if (0 == strcmp(tag_name, "rating_season_rule"))
    {
      _parse_rating_season_rule(config, elt);
    }
    else if (0 == strcmp(tag_name, "ratingseason"))
    {
      _parse_ratingseason(config, elt);
    }
    else if (0 == strcmp(tag_name, "abuse_manager_config"))
    {
      _parse_abuse_manager_config(config, elt);
    }
    else if (0 == strcmp(tag_name, "consecutive_login_bonus"))
    {
      _parse_consecutive_login_bonus(config, elt);
    }
    else if (0 == strcmp(tag_name, "consecutive_login_bonus_holiday"))
    {
      _parse_consecutive_login_bonus_holiday(config, elt);
    }
    else if (0 == strcmp(tag_name, "profile_progression_config"))
    {
      _parse_profile_progression_config(config, elt);
    }
    else if (0 == strcmp(tag_name, "special_reward_configuration"))
    {
      _parse_special_reward_configuration(config, elt);
    }
    else if (0 == strcmp(tag_name, "votes"))
    {
      _parse_votes(config, elt);
    }
    else if (0 == strcmp(tag_name, "regions"))
    {
      _parse_regions(config, elt);
    }
    else
    {
#ifdef DEBUG
      xprintf("FIXME: Unhandled config '%s'\n", tag_name);
#endif /* DEBUG */
    }

    free(tag_name);
}

void _reset_configs(void)
{
  struct game_config *config = session.wf.config.game;

  if (config != NULL)
  {
    free(config->rating.rule.season_id_template);
    config->rating.rule.season_id_template = NULL;
    free(config->rating.rule.banner);
    config->rating.rule.banner = NULL;
    free(config->rating.rule.description);
    config->rating.rule.description = NULL;
    free(config->rating.rule.rules);
    config->rating.rule.rules = NULL;
    free(config->rating.rule.announcement_end_date);
    config->rating.rule.announcement_end_date = NULL;
    free(config->rating.rule.games_end_date);
    config->rating.rule.games_end_date = NULL;

    if (config->rating.rule.rewards.rank != NULL)
      list_free(config->rating.rule.rewards.rank);
    config->rating.rule.rewards.rank = NULL;

    if (config->rating.rule.rewards.season != NULL)
      list_free(config->rating.rule.rewards.season);
    config->rating.rule.rewards.season = NULL;

    if (config->abuse.report_types != NULL)
      list_free(config->abuse.report_types);
    config->abuse.report_types = NULL;

    if (config->profile_progression.events != NULL)
      list_free(config->profile_progression.events);
    config->profile_progression.events = NULL;

    if (config->consecutive_login_bonus.streaks != NULL)
      list_free(config->consecutive_login_bonus.streaks);
    config->consecutive_login_bonus.streaks = NULL;

    if (config->special_rewards.events != NULL)
      list_free(config->special_rewards.events);
    config->special_rewards.events = NULL;
  }

  free(config);
  session.wf.config.game = calloc(1, sizeof (struct game_config));
}

void querycache_get_configs_init(void)
{
    if (cvar.query_disable_get_configs)
        return;

    querycache_init((struct querycache *) &session.wf.config,
                    "get_configs",
                    (f_querycache_parser) _parse_config,
                    (f_querycache_reset) _reset_configs);
}

void querycache_get_configs_free(void)
{
    free(session.wf.config.game);
    session.wf.config.game = NULL;

    querycache_free((struct querycache *) &session.wf.config);
}

void xmpp_iq_get_configs(f_get_configs_cb cb, void *args)
{
    if (cvar.query_disable_get_configs)
        return;

    querycache_request((struct querycache *) &session.wf.config,
                       QUERYCACHE_ANY_CHANNEL,
                       cb,
                       args);
}
