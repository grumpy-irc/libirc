//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef IRC_NUMERICS_H
#define IRC_NUMERICS_H

// Definitions of IRC numerics, lot of them taken from https://www.alien.net.au/irc/irc2numerics.html

// These are non-standard specific for libirc only
#define IRC_NUMERIC_INVALID            -200
#define IRC_NUMERIC_RAW_PING           -1
#define IRC_NUMERIC_RAW_PRIVMSG        -2
#define IRC_NUMERIC_RAW_NOTICE         -3
#define IRC_NUMERIC_RAW_KICK           -4
#define IRC_NUMERIC_RAW_QUIT           -5
#define IRC_NUMERIC_RAW_PART           -6
#define IRC_NUMERIC_RAW_JOIN           -7
#define IRC_NUMERIC_RAW_NICK           -8
#define IRC_NUMERIC_RAW_MODE           -9
#define IRC_NUMERIC_RAW_TOPIC          -10
#define IRC_NUMERIC_RAW_CTCP           -11
#define IRC_NUMERIC_RAW_CAP            -12
#define IRC_NUMERIC_RAW_AWAY           -13
#define IRC_NUMERIC_RAW_METADATA       -14
#define IRC_NUMERIC_RAW_INVITE         -15
#define IRC_NUMERIC_RAW_CHGHOST        -16 // CAP https://ircv3.net/specs/extensions/chghost-3.2.html

// Both RFC standard and not standard
#define IRC_NUMERIC_RAW_PONG           0
#define IRC_NUMERIC_WELCOME            1 // The first message sent after client registration. The text used varies widely
#define IRC_NUMERIC_YOURHOST           2 // Part of the post-registration greeting. Text varies widely
#define IRC_NUMERIC_CREATED            3 // Part of the post-registration greeting. Text varies widely
#define IRC_NUMERIC_MYINFO             4 // Part of the post-registration greeting
#define IRC_NUMERIC_BOUNCE             5 // Sent by the server to a user to suggest an alternative server, sometimes used when the connection is refused because the server is already full.
//                                          Also known as RPL_SLINE (AustHex), and RPL_REDIR Also see #010.
#define IRC_NUMERIC_ISUPPORT           5 // Also known as RPL_PROTOCTL (Bahamut, Unreal, Ultimate)
#define IRC_NUMERIC_MAP                6
#define IRC_NUMERIC_MAPEND             7
#define IRC_NUMERIC_SNOMASK            8
#define IRC_NUMERIC_STATMEMTOT         9
//#define IRC_NUMERIC_BOUNCE             10
#define IRC_NUMERIC_YOURCOOKIE         14
#define IRC_NUMERIC_YOURID             42
#define IRC_NUMERIC_SAVENICK           43 // Sent to the client when their nickname was forced to change due to a collision
#define IRC_NUMERIC_TRACELINK          200
#define IRC_NUMERIC_STATS              210
#define IRC_NUMERIC_RPL_TRYAGAIN       263 // When a server drops a command without processing it, it MUST use this reply. Also known as RPL_LOAD_THROTTLED and RPL_LOAD2HI, I'm presuming they do the same thing.
#define IRC_NUMERIC_NONE               300 // Dummy reply, supposedly only used for debugging/testing new features, however has appeared in production daemons.
#define IRC_NUMERIC_AWAY               301
#define IRC_NUMERIC_UNAWAY             305 // Reply from AWAY when no longer marked as away
#define IRC_NUMERIC_NOWAWAY            306 // Reply from AWAY when marked away
#define IRC_NUMERIC_WHOISREGNICK       307 // Reply to WHOIS - Registered nick
#define IRC_NUMERIC_WHOISUSER          311 // Reply to WHOIS - Information about the user
#define IRC_NUMERIC_WHOISSERVER        312 // Reply to WHOIS - What server they're on
#define IRC_NUMERIC_WHOISOPERATOR      313 // Reply to WHOIS - User has IRC Operator privileges
#define IRC_NUMERIC_WHOWASUSER         314 // Reply to WHOWAS - Information about the user
#define IRC_NUMERIC_ENDOFWHO           315 // Used to terminate a list of RPL_WHOREPLY replies
#define IRC_NUMERIC_WHOISCHANOP        316 // Deprecated
#define IRC_NUMERIC_WHOISIDLE          317 // Reply to WHOIS - Idle information
#define IRC_NUMERIC_ENDOFWHOIS         318 // Reply to WHOIS - End of list
#define IRC_NUMERIC_WHOISCHANNELS      319 // Reply to WHOIS - Channel list for user (See RFC)
#define IRC_NUMERIC_WHOISSPECIAL       320 // Unreal specific
#define IRC_NUMERIC_LISTSTART          321
#define IRC_NUMERIC_LIST               322
#define IRC_NUMERIC_LISTEND            323
#define IRC_NUMERIC_MODEINFO           324 // Response to MODE if no parametres provided
#define IRC_NUMERIC_CHANNELURL         328
#define IRC_NUMERIC_CREATIONTIME       329
#define IRC_NUMERIC_WHOISACCOUNT       330 // Unreal specific
#define IRC_NUMERIC_NOTOPIC            331
#define IRC_NUMERIC_TOPICINFO          332
#define IRC_NUMERIC_TOPICWHOTIME       333
#define IRC_NUMERIC_BADCHANPASS        339
#define IRC_NUMERIC_EXCEPTION          348
#define IRC_NUMERIC_ENDOFEX            349
#define IRC_NUMERIC_WHOREPLY           352
#define IRC_NUMERIC_NAMREPLY           353
#define IRC_NUMERIC_WHOSPCRPL          354
#define IRC_NUMERIC_ENDOFNAMES         366
#define IRC_NUMERIC_BAN                367
#define IRC_NUMERIC_ENDOFBANS          368
#define IRC_NUMERIC_ENDOFWHOWAS        369
#define IRC_NUMERIC_MOTD               372
#define IRC_NUMERIC_MOTDBEGIN          375
#define IRC_NUMERIC_MOTDEND            376
#define IRC_NUMERIC_WHOISHOST          378 // Unreal specific host from which user is connected
#define IRC_NUMERIC_WHOISMODES         379 // Unreal specific list of modes "target nick :is using modes +iwx"
#define IRC_NUMERIC_YOUREOPER          381
#define IRC_NUMERIC_ERR_UNKNOWNERROR   400
#define IRC_NUMERIC_ERR_NOSUCHNICK     401
#define IRC_NUMERIC_ERR_NOSUCHSERVER   402
#define IRC_NUMERIC_ERR_NOSUCHCHANNEL  403
#define IRC_NUMERIC_ERR_CANNOTSENDTOCHAN 404
#define IRC_NUMERIC_ERR_TOOMANYCHANNELS  405
#define IRC_NUMERIC_ERR_WASNOSUCHNICK  406
#define IRC_NUMERIC_ERR_TOOMANYTARGETS 407
#define IRC_NUMERIC_ERR_NOCOLORSONCHAN 408
#define IRC_NUMERIC_ERR_NOORIGIN       409
#define IRC_NUMERIC_ERR_INVALIDCAPCMD  410
#define IRC_NUMERIC_ERR_NORECIPIENT    411
#define IRC_NUMERIC_ERR_NOTEXTTOSEND   412
#define IRC_NUMERIC_UNKNOWN            421
#define IRC_NUMERIC_NICKUSED           433
#define IRC_NUMERIC_NICKISNOTAVAILABLE 437
#define IRC_NUMERIC_WHOISSECURE        671 // Reply to WHOIS command - Returned if the target is connected securely, eg. type
//                                            may be TLSv1, or SSLv2 etc. If the type is unknown, a '*' may be used.

#endif // IRC_NUMERICS_H
