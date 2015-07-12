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

Since this program targets Warface gamers (which is a _Windows-only_ game) but
is written in C for GNU/Linux, it is mandatory to understand the concept of
licensing and _free software_ that yields in the GNU world. Indeed, this
program is distributed under the terms of GPLv3. Please take the time to read
and understand the file **LICENSE** shipped within the repository.

## About this program

This is a blind XMPP client for Warface only, has the latter uses a special
overlay protocol to hide the fact that it's using XMPP. Blind, because it
performs lobby actions without the need of launching a resource-needing game
and can be ran on a dedicated server.

It features a little XMPP console that lets you (_at your own risks !_) send
XMPP queries.

## Preparing the accounts

If you wish to use your bots for helping you start solo games, you need to
train them *before* using this program as they wont be able to join Skilled,
Hardcore or survival rooms. Else, skip this section.

**Skilled and Hardcore rooms** can be unlocked starting from **level
  4**. Thus, you'll need to complete the class tutorials, the initiation and
  regular maps, and some other PvP games to access this level. It represents
  **30 minutes of game per account**.

**Survival rooms** are available starting from **level 10**. You'll need to
  either spend some time in PvP - Storm if you're a legit player. This
  represents **1 day of game per account**. If you use cheats, use them in PvE
  with your other bots.

**To link your main account with your bot**:

1. **Create** a bot account
2. Launch warfacebot your **bot account**
3. Launch your **main account** with the official game
4. **Send a friend request** to your bot account
5. The bot should automatically accept the friend invitation request

**Optional** (for Skilled/Hardcore and Survival):

1. Launch the **bot account** with the official game
2. **Accept the friend request** if it didn't accept it
3. Complete the tutorials and **reach level 4**.

## How to use

1. Clone this repository :
   ```
   $ git clone https://github.com/Levak/warfacebot.git
   ```

2. Compile the application :
   ```
   $ cd warfacebot
   $ make
   ```

3. Log in to GFace and obtain an identification token with that simple script
`wb.sh` (or use the one provided) :
   ```
   #! /usr/bin/env bash
   read -p "Email: " email
   read -s -p "Password: " psswd

   res=$(curl -s --data "email=${email}&pwd=${psswd}" \
         'https://gface.com/api/-text/auth/login.json')

   token=$(sed 's/^.*token":"\([-0-9a-f]*\).*$/\1/' <<< ${res})
   userid=$(sed 's/^.*userid":\([0-9]*\).*$/\1/' <<< ${res})

   ./wb ${token} ${userid} eu # Europe server (others: na/tr)
   ```

4. Run the application :
   ```
   $ ./wb.sh
   Email: mybot@trashmail.com
   Password: 
   ...
   ```

5. Repeat the step 4 for every bot you wish to boot up in another terminal.

6. The bot replies to any **follow** requests sent from the game to that
account.

7. You can **whisper** commands to the bot such as :
   - `leave`: Asks the account to leave the current game room.
   - `master`: Asks the account to give you the room master rights.
   - `ready`: Asks the account to give set its state to *ready*.
   - `take <class>`: Alias to `ready <class>`. Force the bot to take a
      specific class (medic, sniper, engineer, rifleman).
   - `invite`: Will send you an invitation request to the room the account is
      in (in case you left the room).
   - `whois <nickname>`: Retrieves the user IP and his status (such as AFK).
