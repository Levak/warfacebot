# WarfaceBot
... **a blind XMPP client for Warface (FPS game).**

## Why

After my [analysis of Warface in-game protocol][1], I've decided to write a
blind client that will perform various statistical tasks. While developing it,
I've found it could be used for other means, such as **create solo games**,
which could be the opportunity for legit players to face new difficulties, or
for hack-users to cheat alone.

[1]: http://wf.comuv.com

## Prerequisite

It is mandatory to understand the concept of licensing and _free software_
that yields in the GNU world. Indeed, this program is distributed under the
terms of GPLv3. Please take the time to read and understand the file
**LICENSE** shipped within this repository.

## About this program

This is a XMPP client for Warface only, as the latter uses a special overlay
protocol to hide the fact that it's using XMPP. It's a blind client because it
performs lobby actions without the need of launching a resource-needing game
and can be ran on a dedicated server.

This program is shipped fully functionnal but is intended to be customized
(such as adding support to Cleverbot, adding stats, or custom commands).

It also features a little XMPP console that lets you (_at your own risks !_)
send XMPP queries. It can be also used to enter owner-only commands.

## How does it work?

This program acts like a normal game client. It's basically a dummy
implementation of both Warface overlay protocol, XMPP, and the custom queries
used by Warface. XMPP is a standard _presence_ protocol. All the lobby and
shop actions are made using custom XMPP queries.

It is composed of two parts, similary to the game:
 - `wb.sh` - The launcher. It is responsible to log you in to the
   authentication server which will provide us a "session token". The launcher
   is made seperate in order to be scriptable and customizable (for instance,
   you can make it work for different Warface servers such as VN, RU, or BR);
 - `wb` - The actual program. It uses the "session token" given by the
   launcher to login to the XMPP service. Once there, this program acts like
   the real game does.

## Important notices

This program lacks of several features that would make it totally
undetectable. Firstly, if not hooked up to a Cleverbot (or others), a really
easy Turing Test would tell if you are dealing with a bot or a human.

On a more serious level, here is the TODO list:
 - Complete the `peer_player_info` handler. Currently, all the stats (that are not
   worth storing) are random-based;
 - Add an AFK trigger. Currently, the bot will never suffer from AFK-like
   features the game has, since this is a client-side mechanism. It can stay
   forever in a room until you tell him to leave or someone kicks him;
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
  Storm if you're a legit player. This represents **1 day of game per
  account**. If you use cheats, use them in PvE with your other bots.

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
 - `switch`: If in PvP, try to switch team.

### Owner-only commands

You can enter owner-only commands directly in the terminal, such as:
 - `add <nickname>`: Send to `nickname` a friend request;
 - `remove <nickname>`: Remove `nickname` from the buddy-list;
 - `say <msg>`: If in a room, make him speak;
 - `open <map/mission>`: open a game room with _'map'_ (PvP) or _'mission'_
   (PvE). The _'map'_ list is available in the file `src/pvp_maps.c`. The
   _'mission'_ is either 'trainingmission', 'easymission', 'normalmission',
   'hardmission', 'survivalmission' or 'campaingnsections';
 - `name <roomname>`: Change the name of the PvP room;
 - `change <map/mission>`: Change _'map'_ or _'mission'_. See `open`;
 - `safe <map>`: Create a blacklist-based safe-room (**Need customization**, see
   the file `src/cmds/cmd_safe.c`). Notice the bot won't respond to any
   invite-requests while in safe-mode. To make him leave the safe-mode, use
   the command `leave`.
 - `channel <channel>`: Switch to _'channel'_.
