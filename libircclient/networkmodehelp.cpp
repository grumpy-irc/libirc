//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2017

#include "networkmodehelp.h"

using namespace libircclient;

QHash<char, QString> NetworkModeHelp::GetUserModeHelp(QString ircd)
{
    QHash<char, QString> result;

    return result;
}

QHash<char, QString> ircd_seven(QHash<char, QString> result)
{
    result.insert('n', "No external messages.  Only channel members may talk in the channel.");
    result['t'] = "Ops Topic.  Only opped (+o) users may set the topic.";
    result.insert('s', "Secret.  Channel will not be shown in /whois and /list etc.");
    result.insert('p', "Private.  Disables /knock to the channel.");
    result['m'] = "Moderated.  Only opped/voiced users may talk in channel.";
    result['i'] = "Invite only.  Users need to be /invite'd or match a +I to join the channel.";
    result.insert('r', "Registered users only.  Only users identified to services may join.");
    result.insert('c', "No color.  All color codes in messages are stripped.");
    result.insert('g', "Free invite.  Everyone may invite users.  Significantly weakens +i control.");
    result.insert('z', "Op moderated.  Messages blocked by +m, +b and +q are instead sent to ops.");
    result.insert('L', "Large ban list.  Increase maximum number of +beIq entries. Only settable by opers.");
    result.insert('P', "Permanent.  Channel does not disappear when empty.  Only settable by opers.");
    result.insert('F', "Free target.  Anyone may set forwards to this (otherwise ops are necessary).");
    result.insert('Q', "Disable forward.  Users cannot be forwarded to the channel (however, new forwards can still be set subject to +F).");
    result.insert('C', "Disable CTCP. All CTCP messages to the channel, except ACTION, are disallowed.");
    return result;
}

QHash<char, QString> solanum(QHash<char, QString> result)
{
    result.insert('n', "No external messages.  Only channel members may talk in the channel.");
    result['t'] = "Ops Topic.  Only opped (+o) users may set the topic.";
    result.insert('s', "Secret.  Channel will not be shown in /whois and /list etc.");
    result.insert('p', "Private.  Disables /knock to the channel.");
    result['m'] = "Moderated.  Only opped/voiced users may talk in channel.";
    result['i'] = "Invite only.  Users need to be /invite'd or match a +I to join the channel.";
    result.insert('r', "Registered users only.  Only users identified to services may join.");
    result.insert('c', "No color.  All color codes in messages are stripped.");
    result.insert('g', "Free invite.  Everyone may invite users.  Significantly weakens +i control.");
    result.insert('z', "Op moderated.  Messages blocked by +m, +b and +q are instead sent to ops.");
    result.insert('L', "Large ban list.  Increase maximum number of +beIq entries. Only settable by opers.");
    result.insert('P', "Permanent.  Channel does not disappear when empty.  Only settable by opers.");
    result.insert('F', "Free target.  Anyone may set forwards to this (otherwise ops are necessary).");
    result.insert('Q', "Disable forward.  Users cannot be forwarded to the channel (however, new forwards can still be set subject to +F).");
    result.insert('C', "Disable CTCP. All CTCP messages to the channel, except ACTION, are disallowed.");
    result.insert('O', "IRC Operator only channel");
    result.insert('A', "IRC server administrator only channel");
    result.insert('T', "No NOTICEs allowed in the channel");
    result.insert('S', "Only users connected via SSL/TLS may join the channel while this mode is set. Users already in the channel are not affected.");
    return result;
}

QHash<char, QString> unrealircd4(QHash<char, QString> result)
{
    result.insert('c' ,"Block messages containing mIRC color codes [o]");
    result.insert('C', "No CTCPs allowed in the channel [h]");
    result.insert('D', "Delays someone's JOIN message until that person speaks. Chanops and higher, opers and ulines/services are exempt.  [o]");
    result.insert('G', "Filters out all Bad words in messages with <censored> [o]");
    result['i'] = "A user must be invited to join the channel [h]";
    result.insert('K', "/KNOCK is not allowed [o]");
    result['m'] = "Moderated channel (only +vhoaq users may speak) [h]";
    result.insert('M', "Must be using a registered nick (+r), or have voice access to talk [h]");
    result.insert('N', "No Nickname changes are permitted in the channel [h]");
    result.insert('n', "Users outside the channel can not send PRIVMSGs to the channel [h]");
    result.insert('O', "IRC Operator only channel (settable by IRCops)");
    result.insert('p', "Private channel [o]");
    result.insert('Q', "No kicks allowed [o]");
    result.insert('R', "Only registered (+r) users may join the channel [o]");
    result.insert('r', "The channel is registered (settable by services only)");
    result.insert('s', "Secret channel [o]");
    result.insert('S', "Strips mIRC color codes [o]");
    result.insert('T', "No NOTICEs allowed in the channel [o]");
    result.insert('t', "Only +hoaq may change the topic [h]");
    result.insert('V', "/INVITE is not allowed [o]");
    result.insert('Z', "All users on the channel are on a Secure connection (SSL) [server] (This mode is set/unset by the server. Only if the channel is also +z)");
    result.insert('z', "Only Clients on a Secure Connection (SSL) can join [o]");
    return result;
}

QHash<char, QString> NetworkModeHelp::GetChannelModeHelp(QString ircd)
{
    QHash<char, QString> result;
    result.insert('m', "Moderated - will suppress all messages from people who don't have voice (+v) or higher.");
    result.insert('t', "Topic changes restricted - only allow privileged users to change the topic.");
    result.insert('i', "A user must be invited to join the channel");
    result.insert('n', "Users cannot send messages to the channel externally");
    result.insert('s', "Secret channel—not shown in channel list or user whois except to users already on the channel");
    result.insert('p', "Private channel—listed in channel list as prv according to RFC 1459");

    if (ircd.startsWith("Unreal3"))
    {
            /*
                (Tue Nov 28 16:50:00 2017) :hub.tm-irc.org 292 petan : b <nick!ident@host> = Bans the nick!ident@host from the channel [h]
                (Tue Nov 28 16:50:00 2017) :hub.tm-irc.org 292 petan :                       (For more info on extended bantypes, see /HELPOP EXTBANS)
                (Tue Nov 28 16:50:00 2017) :hub.tm-irc.org 292 petan : e <nick!ident@host> = Overrides a ban for matching users [h]
                (Tue Nov 28 16:50:00 2017) :hub.tm-irc.org 292 petan : f <floodparams> = Flood protection (for more info see /HELPOP CHMODEF) [o]
                (Tue Nov 28 16:50:00 2017) :hub.tm-irc.org 292 petan : I <nick!ident@host> = Overrides +i for matching users [h]
                (Tue Nov 28 16:50:00 2017) :hub.tm-irc.org 292 petan : j <joins:sec> = Throttle joins per-user to 'joins' per 'sec' seconds [o]
                (Tue Nov 28 16:50:00 2017) :hub.tm-irc.org 292 petan : k <key> = Users must specify <key> to join [h]
                (Tue Nov 28 16:50:00 2017) :hub.tm-irc.org 292 petan : L <chan2> = Channel link (If +l is full, the next user will auto-join <chan2>) [q]
                (Tue Nov 28 16:50:00 2017) :hub.tm-irc.org 292 petan : l <number of max users> = Channel may hold at most <number> of users [o]
                (Tue Nov 28 16:50:01 2017) :hub.tm-irc.org 292 petan : z = Only Clients on a Secure Connection (SSL) can join [o]
                (Tue Nov 28 16:50:01 2017) :hub.tm-irc.org 292 petan : Z = All users on the channel are on a Secure connection (SSL) [server]
                (Tue Nov 28 16:50:01 2017) :hub.tm-irc.org 292 petan :     (This mode is set/unset by the server. Only if the channel is also +z)
             *
             */
        result.insert('A', "Server/Net Admin only channel (settable by Admins)");
        result.insert('c' ,"Block messages containing mIRC color codes [o]");
        result.insert('C', "No CTCPs allowed in the channel [o]");
        result.insert('G', "Filters out all Bad words in messages with <censored> [o]");
        result['i'] = "A user must be invited to join the channel [h]";
        result.insert('K', "/KNOCK is not allowed [o]");
        result['m'] = "Moderated channel (only +vhoaq users may speak) [h]";
        result.insert('M', "Must be using a registered nick (+r), or have voice access to talk [o]");
        result.insert('N', "No Nickname changes are permitted in the channel [o]");
        result.insert('n', "Users outside the channel can not send PRIVMSGs to the channel [h]");
        result.insert('O', "IRC Operator only channel (settable by IRCops)");
        result.insert('p', "Private channel [o]");
        result.insert('Q', "No kicks allowed [o]");
        result.insert('R', "Only registered (+r) users may join the channel [o]");
        result.insert('r', "The channel is registered (settable by services only)");
        result.insert('s', "Secret channel [o]");
        result.insert('S', "Strips mIRC color codes [o]");
        result.insert('T', "No NOTICEs allowed in the channel [o]");
        result.insert('t', "Only +hoaq may change the topic [h]");
        result.insert('u', "Auditorium mode (/names and /who #channel only show channel ops) [q]");
        result.insert('V', "/INVITE is not allowed [o]");
        result.insert('z', "Only Clients on a Secure Connection (SSL) can join [o]");

    } else if (ircd.startsWith("UnrealIRCd-4"))
    {
        result = unrealircd4(result);
    } else if (ircd.startsWith("ircd-seven-1"))
    {
        result = ircd_seven(result);
    } else if (ircd.startsWith("solanum-"))
    {
        result = solanum(result);
    }

    return result;
}
