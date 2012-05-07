/*
    This file is part of SeeBorg.
	Copyright (C) 2003, 2006 Eugene Bujak.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

*/

#include "required.h"

#ifndef __SEEBORG_IRC_H__
#define __SEEBORG_IRC_H__

// Bot commands
// ---------------------------------------------------------------------------
wstring CMD_Shutup_f (class SeeBorg* self, const wstring command);
wstring CMD_Wakeup_f (class SeeBorg* self, const wstring command);

wstring CMD_Replyrate_f (class SeeBorg* self, const wstring command);
wstring CMD_Replynick_f (class SeeBorg* self, const wstring command);
wstring CMD_Replyword_f (class SeeBorg* self, const wstring command);

wstring CMD_Join_f (class SeeBorg* self, const wstring command);
wstring CMD_Part_f (class SeeBorg* self, const wstring command);
wstring CMD_Quit_f (class SeeBorg* self, const wstring command);
wstring CMD_Save_f (class SeeBorg* self, const wstring command);
wstring CMD_Stealth_f(class SeeBorg* self, const wstring command);
wstring CMD_Learning_f (class SeeBorg* self, const wstring command);
wstring CMD_ircHelp_f (class SeeBorg* self, const wstring command);

wstring ircParseCommands(const wstring &cmd, const char* who);

static botcommand_t ircbotcmds[] = {
  {L"help", L"Show this command list", CMD_ircHelp_f},
  {L"shutup", L"As the name says", CMD_Shutup_f},
  {L"wakeup", L"As the name says", CMD_Wakeup_f},
  {L"join", L"Join channel", CMD_Join_f},
  {L"part", L"Part channel", CMD_Part_f},

  {L"replyrate", L"Show/set reply rate", CMD_Replyrate_f},
  {L"replynick", L"Show/set nick reply rate", CMD_Replynick_f},
  {L"replymagic", L"Show/set magic word reply rate", CMD_Replyword_f},

  {L"quit", L"As the name implies", CMD_Quit_f},
  {L"save", L"Immediately save dictionary and settings", CMD_Save_f},
  
  {L"learning", L"Enable/disable bot's learning ability, should be enabled", CMD_Learning_f},
  {L"stealth", L"CTCP version stealth", CMD_Stealth_f},

  {NULL, NULL, NULL}
};
static const int numircbotcmds = sizeof(ircbotcmds) / sizeof(ircbotcmds[0]) - 1;

// Bot Settings
// ---------------------------------------------------------------------------
typedef struct ircbotowner_s {
	wstring nickname;
	wstring hostname;
} ircbotowner_t;

typedef vector<ircbotowner_t> ircbotowners_t;

typedef struct botsettings_s {
	botsettings_s();
	~botsettings_s();
	// IRC-specific
	wstring         server;
	int32_t         serverport;
	wstring         nickname;
	wstring         username;
	wstring         realname;
	vector<wstring> channels;
	ircbotowners_t  owners;
	wstring         quitmessage;
	wstring         ctcpversionstring;
	
	// Other settings
	float           replyrate;
	int32_t         learning;
	
	
	int             speaking;
	int             stealth;		// TODO
	vector<wstring> censored;		// TODO
	vector<wstring> ignorelist;		// TODO
	int             reply2ignored;	// TODO
	
	int             joininvites;	// TODO
	float           replyrate_mynick;
	float           replyrate_magic;
	vector<wstring> magicwords;
	vector<wstring> botakas;
	
	int             autosaveperiod;

	tokenizer_t*    tokenizer;
	
} botsettings_t;

extern botsettings_t botsettings;

// Bot Config File
// ---------------------------------------------------------------------------
typedef struct configsetting_s {
	const wchar_t* configline;
	const wchar_t* description;
	
	wstring* stringptr;
	float*   floatptr;
	int*     intptr;

	vector<wstring>* stringvectorptr;
} configsetting_t;

static const configsetting_t configsettings[] = {
	{L"server", L"Address of IRC server", &botsettings.server, NULL, NULL, NULL},
	{L"serverport", L"Server port", NULL, NULL, &botsettings.serverport, NULL},
	
	{L"nickname", L"Bot's nickname, most servers allow English only", &botsettings.nickname, NULL, NULL, NULL},
	{L"username", L"Bot's username (will show as ~<username>@your.host.com), most servers allow English only", &botsettings.username, NULL, NULL, NULL},
	{L"realname", L"Bot's realname (will show in whois)", &botsettings.realname, NULL, NULL, NULL},
	{L"quitmessage", L"Bot's quit message", &botsettings.quitmessage, NULL, NULL, NULL},
	{L"ctcpversion", L"Bot's CTCP version string (if stealth is enabled)", &botsettings.ctcpversionstring, NULL, NULL, NULL},
	
	{NULL, NULL, NULL, NULL, NULL},	// Newline in cfg
	
	{L"replyrate", L"Reply rate to all messages (in percent)", NULL, &botsettings.replyrate, NULL, NULL},
	{L"replynick", L"Reply rate to messages containing bot's nickname (in percent)", NULL, &botsettings.replyrate_mynick, NULL, NULL},
	{L"replymagic", L"Reply rate to messages containing magic words (in percent)", NULL, &botsettings.replyrate_magic, NULL, NULL},
	
	{NULL, NULL, NULL, NULL, NULL},	// Newline in cfg
	
	{L"speaking", L"Controls whether the bot speaks at all (boolean)", NULL, NULL, &botsettings.speaking, NULL},
	{L"learning", L"Does the bot learn, or just replies (boolean)", NULL, NULL, &botsettings.learning, NULL},
	{L"stealth", L"Try to emulate a popular IRC client's behaviour (TODO, boolean)", NULL, NULL, &botsettings.stealth, NULL},
	{L"joininvites", L"Join the channels the bot was invited to (0 - no, 1 - yes, 2 - only by owner)", NULL, NULL, &botsettings.joininvites, NULL},
	
	{NULL, NULL, NULL, NULL, NULL},	// Newline in cfg
	
	{L"autosaveperiod", L"Autosave period (in seconds)", NULL, NULL, &botsettings.autosaveperiod, NULL},

	{NULL, NULL, NULL, NULL, NULL},	// Newline in cfg

	{L"channels", L"Channel list to join to", NULL, NULL, NULL, &botsettings.channels},
	{L"magicwords", L"Magic word list", NULL, NULL, NULL, &botsettings.magicwords},
	{L"botakas", L"The words that bot will identify as itself", NULL, NULL, NULL, &botsettings.botakas},
	{L"censorlist", L"Censor word list (will prevent bot from speaking those words)", NULL, NULL, NULL, &botsettings.censored},
	{L"ignorelist", L"Ignore word list (will prevent bot from reacting to those words)", NULL, NULL, NULL, &botsettings.ignorelist},
	
	{NULL, NULL, NULL, NULL, NULL}
};

static const int numconfigsettings = sizeof(configsettings) / sizeof(configsettings[0]) - 1;

void LoadBotSettings();
void SaveBotSettings();


#endif
