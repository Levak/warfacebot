# WarfaceBot [![Build Status](https://travis-ci.org/Levak/warfacebot.svg?branch=master)](https://travis-ci.org/Levak/warfacebot) [![Coverity](https://img.shields.io/coverity/scan/9461.svg)](https://scan.coverity.com/projects/levak-warfacebot)
... **a headless XMPP client for Warface (FPS game).**

## Why

After my [analysis of Warface in-game protocol][1], I've decided to write an
headless client that will perform various statistical tasks. While developing
it, I've found it could be used for other means, such as **create solo
games**, which could be the opportunity for legit players to face new
difficulties as the official game doesn't have support for single player.

[1]: https://stackedit.io/viewer#!provider=gist&gistId=b9a1852a0a17e334f041&filename=wfre

## License

It is mandatory to understand the concept of licensing and _free software_
that yields in the GNU world. Indeed, this program is distributed under the
terms of AGPLv3. Please take the time to read and understand the file
**LICENSE** shipped within this repository.

## About this program

This is a XMPP client for Warface only, as the latter uses a special overlay
protocol to hide the fact that it's using XMPP. It's a headless client because
it performs lobby actions without the need of launching a resource-needing
game and can be ran on a dedicated server.

This program is shipped fully functionnal but is intended to be customized
(such as adding support to Cleverbot, adding stats, or custom commands).

It also features a little console that lets you enter owner-only commands. It
can also be used to send XMPP queries (_at your own risks !_).

## How does it work?

This program acts like a normal game client. It's basically a dummy
implementation of both Warface overlay protocol, XMPP, and the custom queries
used by Warface. XMPP is a standard _presence_ protocol. All the lobby and
shop actions are made using custom XMPP queries.

It is composed of two parts, similary to the game:

 - **Launcher**: It is responsible to log you in to the
   authentication server which will provide us a "session token". The launcher
   is made seperate in order to be scriptable and customizable (for instance,
   you can make it work for different Warface servers such as VN, RU, or
   BR). There are several launchers:
   + `wb.sh`: Bash launcher for GNU/Linux users;
   + `wb_launcher.hta`: GUI launcher for Windows users;
   + `wbd_launcher`: Bash launcher when warfacebot is compiled DBUS mode;
   + `wbm_launcher`: Bash launcher for the DBUS manager;
 - **Client** - `wb` - The actual program. It uses the "session token" given
   by the launcher to login to the XMPP service. Once there, this program acts
   like the real game does.

## Important notices

This program lacks of several features that would make it totally
undetectable. Here is the TODO list:

 - HWID is not implemented. Some servers ban/kick HWIDs that are not
   valid. Use the CVar `game_hwid` on launch to change it;
 - Use a real XML parser (or use a XMPP library). Currently, every query is
   hand-crafted due to historical testings. Besides the untrusty code,
   everything seems to work flawlessly (until the day the server will start
   sending some advanced XML I don't handle correctly).

## How to use

### Preparing the accounts

If you wish to use your bots for helping you starting solo games, you need to
train them *before* using this program as they wont be able to join Skilled,
Hardcore or survival rooms. Else, skip this section.

**Skilled and Hardcore rooms** can be unlocked starting from **level
  4**. Thus, you'll need to complete the class tutorials, the initiation and
  regular maps, and some other PvP games to access this level. It represents
  **30 minutes of game per account**.

**Survival rooms** are available starting from **level 10** (ColdPeak is
  unlocked at level 25). You'll need to either spend some time in PvP -
  Storm. This represents **2 days of game per account**.

**To link your main account with your bot**:

1. **Create** a bot account
2. Launch your **bot account** with warfacebot
3. Launch your **main account** with the official game
4. **Send a friend request** to your bot account from your **main account**
5. The bot should automatically accept the friend invitation request

**Optional** (for Skilled/Hardcore and Survival):

6. Launch the **bot account** with the official game
7. Complete the tutorials and **reach level 4**.

### Step by step

1. Clone this repository (if you didn't download it already) :
   ```
   $ git clone https://github.com/Levak/warfacebot.git
   ```

   *Note*: For Windows users, you can also download it from binary
    [releases](https://github.com/Levak/warfacebot/releases).

2. Compile the application :
   ```
   $ cd warfacebot
   $ make
   ```

3. Run the application by using the provided launcher `wb.sh`:
   ```
   $ ./wb.sh eu
   Email: mybot@trashmail.com
   Password: 
   ...
   ```

4. Repeat the step 3. for every bot you wish to boot up in another terminal.

5. The bot replies to any **follow** requests sent from the game to that
   account. Just ask him to join your room like you would do with any other
   player.

### Whisper commands

You can **whisper** commands to the bot such as:

 - `leave`: Tell him to leave the current game room;
 - `master`: Ask him to give you the room master permissions;
 - `ready`: Set his to lobby state to *ready*;
 - `take <class>`: Alias to `ready <class>`. Force him to take a
    specific class (medic, sniper, engineer, rifleman);
 - `follow [nickname]`: Tell him to follow you or somebody;
 - `invite [nickname]`: Send you an invitation request to the room he's in (in
    case you left him in a room);
 - `whois <nickname>`: Retreive any connected player's country and lobby status;
 - `missions`: Crown challenge objectives;
 - `start`: If he's master, try to start the room;
 - `stay`: If in a room, make sure to stay even if the room started (for 1h,
   until it leaves the current room);
 - `switch`: If in PvP, try to switch team;
 - `unready`: If in a room, keep to unready, until it either receives `ready`
   or leaves the current room.

### Owner-only commands

Additionally, you can enter owner-only commands directly in the terminal, such as:

 - `add <nickname>`: Send to `nickname` a friend request;
 - `remove <nickname>`: Remove `nickname` from the buddy-list;
 - `say <msg>`: If in a room, make him speak;
 - `open <map/mission>`: open a game room with _'map'_ (PvP) or _'mission'_
   (PvE). The _'map'_ list is available in the file `src/pvp_maps.c`. The
   _'mission'_ is either 'training', 'easy', 'normal', 'hard', 'survival',
   'zombie<diff>', 'zombietower<diff>', 'volcano<diff>', 'anubis<diff>', or
   'campaingnsections';
 - `name <roomname>`: Change the name of the PvP room;
 - `change <map/mission>`: Change to _'map'_ or _'mission'_. See `open`;
 - `safe <map>`: Create a blacklist-based safe-room (**Need customization**,
   see the file `src/cmds/cmd_safe.c`). Notice the bot won't respond to any
   invite-requests while in safe-mode. To make him leave the safe-mode, use
   the command `leave`;
 - `channel <channel>`: Switch to _'channel'_;
 - `whisper <nickname> <message>`: Send a private message to a friend of clan
   mate;
 - `friends`: List friends and clanmates;
 - `sleep [n]`: Hang the readline thread for _'n'_ seconds (1 second by
   default);
 - `stats`: List all channel load statistics;
 - `stay <count> [unit]`: If in a room, make sure to stay even if the room
   started. _'count'_ is the number of time _'unit'_ to stay (if _'unit'_ is
   not given, then default to seconds);
 - `randombox [<name> <count>]`: Open _count_ boxes from the randombox called
   _name_. If neither _name_ nor _count_ is given, display the list of
   available randomboxes and their price;
 - `last <nickname>`: Display the last seen date of a friend or a clanmate;
 - `quit`: Exit warfacebot;
 - `quickplay <cmd> [arg1]`: Quickplay matchmaking. _'cmd'_ should be one of
   `open`, `invite`, `cancel` or `start`. _'arg1'_ depends on the command
   used:
   + `open`: _'arg1'_ is a PvE mission or a PvP quickplay map;
   + `invite`: _'arg1'_ is a buddy nickname;
   + `cancel`: _arguments ignored_;
   + `start`: _arguments ignored_.


### Console variables

In order to dynamically affect the behavior of the bot without the need of
recompiling it (either to configure it while running or on launch), couple
CVars have been added.

Config files are single-token-separated files. The token is either a space, an
equal sign, or both. Extra tokens after the cvar name are ignored. Below is an
example of a valid config file (assuming `cvar_name` is a valid cvar):

   ```
   cvar_name 1
   cvar_name=1
   cvar_name   1
   cvar_name = 1
   ```

**Note**: When using `wb.sh <server>` to launch warfacebot, the config file
          `./cfg/server/<server>.cfg` is used in order to determine the game
          version and the server host. If the game version changed, you need
          to update this file accordingly.

#### How to use CVars

CVars can be defined in 4 different ways:

 - From the default config file `wb.cfg`;

 - From a config file given at launch;
   ```
   $ ./wb.sh eu -f <config.cfg>
   ```

 - From a variable defined at launch;
   ```
   $ ./wb.sh eu -d <cvar_name=value>
   ```

 - From the readline prompt.
   ```
   $ ./wb.sh eu
   [...]
   CMD# cvar_name = value
   cvar_name = value
   ```

#### Game-related variables

 - `game_version`: Game version used at login (Mandatory);
 - `game_server_name`: Game server identifier (Mandatory);
 - `game_crypt_key`: Optional server encryption key override (default: NULL);
 - `game_hwid`: HWID used at login (default: 0).
 - `g_language`: Language to use. Used only at startup (see file `wb.cfg`) in
   order to read `cfg/lang/<g_language>.cfg` (default: english);

#### CryOnline-related variables

 - `online_server`: Game server host to connect to (default: NULL);
 - `online_server_port`: Game server port (default: 5222);
 - `online_channel_type`: Default channel type to connect to (default: pve);
 - `online_host`: XMPP server name (default: warface);
 - `online_bootstrap`: Prefix for `online_host` (default: NULL);
 - `online_region_id`: Region to use (default: global);
 - `online_use_protect`: Use an additional encryption layer (default: TRUE);
 - `online_use_tls`: Use TLS encryption(default: TRUE).

#### DBus-related variables

 - `dbus_id`: In DBUS mode, unique DBUS identifier used to construct bot
   busname (Mandatory);

#### Query-related variables

 - `query_dump_to_file`: In DEBUG mode, enable logging queries to a file
   (default: FALSE);
 - `query_dump_location`: In DEBUG mode, location of logged queries (default:
   `./Logs/`);
 - `query_debug`: In DEBUG mode, output queries to stdout (default: TRUE);
 - `query_cache`: Enable query cache system (default: TRUE);
 - `query_cache_location`: Filesystem location where to store query cache
   (default: `./QueryCache/`);
 - `query_disable_items`: Disable fetch of items (default: FALSE);
 - `query_disable_shop_get_offers`: Disable fetch of shop offers, such as
   randomboxes (default: FALSE);
 - `query_disable_quickplay_maplist`: Disable fetch of quickplay map list for
   PvP (default: FALSE);
 - `query_disable_get_configs`: Disable fetch of game configs (default: FALSE);

  **Note:** In order to improve the bot performances (at launch and on channel
  switch), it is recommanded to disable all the above options. In a casual
  usage of bots, there is no need to fetch items, quickplay map list nor shop
  offers. These are enabled by default in order to provide additional
  features, such as random boxes, quickplay pre-made matches and more.

#### Warfacebot-related variables

 - `wb_safemaster`: Setup the bot as a safemaster (default: FALSE);
 - `wb_safemaster_room_name`: Safemaster default room name;
 - `wb_safemaster_channel`: Safemaster default room channel (default:
   pvp_pro_1);
 - `wb_accept_friend_requests`: Accept any friends requests (default: TRUE);
 - `wb_postpone_friend_requests`: Do not treat friends requests at all
    (default: FALSE);
 - `wb_accept_clan_invites`: Accept any clan invitations (default: TRUE);
 - `wb_postpone_clan_invites`: Do not treat clan invitations at all (default:
    FALSE);
 - `wb_enable_whisper_commands`: Proceed whisper commands (default: TRUE);
 - `wb_leave_on_start`: Automatically leave when the room starts (default:
   TRUE);
 - `wb_accept_room_invitations`: Whether to accept buddy room inviations or
   not (default: TRUE);
 - `wb_postpone_room_invitations`: Ignore buddy room inviations (default:
   FALSE);
 - `wb_enable_invite`: Whether to enable or not the `invite` whisper command
   (default: TRUE);
 - `wb_auto_start`: Whether to auto start or not when room master (default:
   TRUE);
 - `wb_auto_afk`: Whether to enble AFK status on status idle (default: FALSE);
 - `wb_ping_unit`: Period used to throttle the ping thread (default: 60 sec.);
 - `wb_ping_count_is_afk`: If `wb_auto_afk` is TRUE, number of ping units
   before sending an AFK status (default: 1);
 - `wb_ping_count_is_stall`: Assuming the scheduler did not receive any query
   yet, number of ping units before sending a ping request (default: 3);
 - `wb_ping_count_is_over`: Number of ping units before considering the
   connection lost (default: 4);
 - `wb_ping_count_is_outdated`: Number of ping units before forcing a profile
   status update (default: 5);
 - `wb_qp_search_started`: When receiving a quickplay offer, accept the first one
   which room has already started (default: TRUE);
 - `wb_qp_search_non_started`: When receiving a quickplay offer, accept the
   first one which room has not started yet (default: TRUE).
