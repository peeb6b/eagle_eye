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

#ifdef __unix__
#include <unistd.h>
#include <sys/wait.h>
#else
#include <time.h>
#ifdef _MSC_VER
#pragma comment (lib, "wsock32.lib")
#endif // _MSC_VER
#endif // __unix__

#include <stdlib.h>
#include <signal.h>
#include <locale.h>
#include <ctype.h>
#include <string.h>

#include "seeborg.h"
#include "seeborg-irc.h"
#include "seeutil.h"


extern "C" {
#include "botnet/botnet.h"
}


// Variables
// ---------------------------------------------------------------------------

BN_TInfo  Info;
bool	  initialized = false;
bool	  connected = false;

#define ABORT_HACK L"#abort#"


// Bot Settings
// ---------------------------------------------------------------------------
botsettings_s::botsettings_s() {
  nickname = L"SeeBorg";
  username = L"SeeBorg";
  utf8_mbstowstring("I am SeeBorg v" SEEBORGVERSIONSTRING " by Eugene Bujak (Буяк Евгений)", realname);
  quitmessage = L"Byebye...";
  ctcpversionstring = L"mIRC v6.2 Khaled Mardam-Bey";
  replyrate = 1;
  replyrate_magic = 33;
  replyrate_mynick = 90;
  learning = true;
  speaking = true;
  joininvites = 2;	// 2: Only react to owners, 1: react to anyone
  
  // These are default channels
  channels.push_back(L"#seeborg");
  channels.push_back(L"#test");
  
  serverport = 6667;
  autosaveperiod = 600;

  tokenizer = tokenizer_init();
}
botsettings_s::~botsettings_s() {
  tokenizer_free(tokenizer);
}

botsettings_t botsettings;

// Bot Config File
// ---------------------------------------------------------------------------
void LoadBotSettings() {
  wstring str;
  FILE* f = fopen ("seeborg-irc.cfg", "r");
  if (f == NULL) return;
  
  while (fReadStringLine (f, str)) {
	  trimString(str, false);
	  // skip comments
    if (!str.empty()) //Avoid accessing an empty str
    { 
      if ( (str[0] != L';') && (str[0] != L'#') ) //Ignore comments in .cfg
      {
	      vector<wstring> cursetting;
	      if (splitString (str, cursetting, L"=") < 2) continue;
	      trimString(cursetting[0], false);
	      trimString(cursetting[1], false);
	
	      // Owner list, special case for parsing
	      if (!wcscasecmp(cursetting[0].c_str(), L"owners")) {
	        vector<wstring> cursplit;
	        splitString(cursetting[1], cursplit);
	        botsettings.owners.clear();
	        for (int i = 0, sz = cursplit.size(); i < sz; i++) {
		      ircbotowner_t ircbotowner;
		      ircbotowner.nickname = cursplit[i];
		      botsettings.owners.push_back(ircbotowner);
	        }
	        continue;
	      }

	      // Go through non-special entries
	      for (int i = 0; i < numconfigsettings; i++) 
        {
	        const configsetting_t* s = &configsettings[i];
	        if (s->configline == NULL) continue;
	        if (!wcscasecmp(s->configline, cursetting[0].c_str()))
          {
		        if (s->stringptr != NULL)
            {
		          *s->stringptr = cursetting[1];
		        }
		        if (s->floatptr != NULL)
            {
		          *s->floatptr = (float)wcstod(cursetting[1].c_str(), NULL);
		        }
		        if (s->intptr != NULL) 
            {
		          *s->intptr = wcstol(cursetting[1].c_str(), NULL, 10);
		        }
		        if (s->stringvectorptr != NULL)
            {
		          s->stringvectorptr->clear();
		          splitString(cursetting[1], *(s->stringvectorptr));
		        }
		        break;
	        }
	      }
    }
  }
  }
  fclose(f);
}

void SaveBotSettings() {
  FILE* f;
  int i, sz;
  utf8writer_t *utf8writer = utf8writer_init();
  if (utf8writer == NULL) {
	// TODO: write an error and warning that settings couldn't be saved
	return;
  }
  
  f = fopen ("seeborg-irc.cfg", "w");
  if (f == NULL) {
	// TODO: write an error and warning that settings couldn't be saved
	return; 
  }
  
  fprintf (f, 
	"; SeeBorg " SEEBORGVERSIONSTRING " settings file"
	"; Lines beginning with ; or # are treated as comments\n\n\n");

  // Go through single-entry settings
  for (i = 0; i < numconfigsettings; i++) {
	const configsetting_t* s = &configsettings[i];
	if (s->configline == NULL) {
	  fprintf (f, "\n\n");
	  continue;
	}
	
	utf8writer_write(utf8writer, f, s->description, "; %s\n");
	utf8writer_write(utf8writer, f, s->configline, "%s = ");
	
	if (s->stringptr != NULL) {
	  utf8writer_write(utf8writer, f, (*s->stringptr).c_str(), "%s\n");
	} else if (s->floatptr != NULL) {
	  fprintf (f, "%.2f\n", *s->floatptr);
	} else if (s->intptr != NULL) {
	  fprintf (f, "%i\n", *s->intptr);
	} else if (s->stringvectorptr != NULL) {
	  wstring jointstring = joinString(*(s->stringvectorptr));
	  utf8writer_write(utf8writer, f, jointstring.c_str(), "%s\n");
	}
  }
  
  // Write owner list
  fprintf (f, "; Owner list (nicknames)\n");
  fprintf (f, "owners =");
  for (i = 0, sz = botsettings.owners.size(); i < sz; i++) {
	utf8writer_write(utf8writer, f, botsettings.owners[i].nickname.c_str(), " %s");
  }
  fprintf (f, "\n");
  
  
  fclose(f);
}

// Message processing
// ---------------------------------------------------------------------------
void checkOwners(const char who[]) {
  char	hostname[4096];
  char	nickname[4096];
  wstring hostnamewc;
  wstring nicknamewc;
  
  BN_ExtractHost(who, hostname, sizeof(hostname));
  BN_ExtractNick(who, nickname, sizeof(nickname));
  utf8_mbstowstring(hostname, hostnamewc);
  utf8_mbstowstring(nickname, nicknamewc);
  
  for (int i = 0, sz = botsettings.owners.size(); i < sz; i++) {
	if (botsettings.owners[i].hostname.empty()) {
	  if (!wcscasecmp(nicknamewc.c_str(), botsettings.owners[i].nickname.c_str())) {
		botsettings.owners[i].hostname = hostnamewc;
		see_printstring(stdout, L"Locked owner '%ls' to '%ls'\n", nicknamewc.c_str(), hostnamewc.c_str());
		return;
	  }
	}
  }
}

bool isOwner(const char who[]) {
  if (botsettings.owners.empty()) return false;
  char	hostname[4096];
  char	nickname[4096];
  wstring hostnamewc;
  wstring nicknamewc;
  
  BN_ExtractHost(who, hostname, sizeof(hostname));
  BN_ExtractNick(who, nickname, sizeof(nickname));
  utf8_mbstowstring(hostname, hostnamewc);
  utf8_mbstowstring(nickname, nicknamewc);
  
  for (int i = 0, sz = botsettings.owners.size(); i < sz; i++) {
	if (!wcscasecmp(hostnamewc.c_str(), botsettings.owners[i].hostname.c_str())) {
	  if (!wcscasecmp(nicknamewc.c_str(), botsettings.owners[i].nickname.c_str())) {
		return true;
	  }
	}
  }
  return false;
}

// TODO: make sure msg will always be lowercase to avoid copy operation inside function
bool checkReplying(const vector<wstring> *keylist, const float chance, const wstring &msg) {
  wstring message = msg;
  lowerString(message);
  bool returnval = false;
  bool throwdice = false;
  if (chance <= 0) return returnval;

  if (keylist == NULL) throwdice = true;
  else {
	size_t sz = keylist->size();
	for (size_t i = 0; i < sz; i++) {
	  wstring curword = (*keylist)[i];
	  lowerString(curword);
	  size_t pos = message.find(curword);
	  if (pos != message.npos) {
		throwdice = true;
		break;
	  }
	}
  }

  if (throwdice) {
	if (randFloat(0, 100) < chance) returnval = true;
  }


  return returnval;
}

wstring ProcessMessage(BN_PInfo I, const char who[], const wstring &msg, bool replying = false) {
  wstring replystring;
  checkOwners(who);
  wstring message = msg;
  lowerString(message);

  if (message[0] == L'!') {
	// Parse commands
	replystring = ircParseCommands(msg, who);
	if (replystring == ABORT_HACK) return L"";

	if (replystring != L"") {
	  return replystring;
	} else {
	  // Try seeborg's core commands
	  replystring = gSeeBorg.ParseCommands(msg);
	  if (replystring == ABORT_HACK) return L"";
	  if (replystring != L"") return replystring;
	}

	return L"";
  }

  // Ignore quotes
  if (message[0] == L'<') return L"";
  if (message[0] == L'[') return L"";

  if (botsettings.speaking) {
	if (!replying) replying = checkReplying(NULL, botsettings.replyrate, message);
	if (!replying) replying = checkReplying(&botsettings.magicwords, botsettings.replyrate_magic, message);
	if (!replying) {
	  vector<wstring> nicknames = botsettings.botakas;
	  wstring nickname;
	  utf8_mbstowstring(I->Nick, nickname);
	  nicknames.push_back(nickname);
	  
	  replying = checkReplying(&nicknames, botsettings.replyrate_mynick, message);
	}
  }
	
  if (replying) {
	replystring = gSeeBorg.Reply(message);
  }

  if (botsettings.learning) gSeeBorg.Learn(message);
  return replystring;
}


// BotNet callback functions
// ---------------------------------------------------------------------------
void ProcOnError(BN_PInfo I, int errnum) {
  see_printstring(stderr, L"Error from botnet: %i\n", errnum);
//  perror("Error from botnet");
}
void ProcOnConnected(BN_PInfo I, const char HostName[]) {
  char* utf8nickname = utf8_wstringtombs(botsettings.nickname);
  char* utf8username = utf8_wstringtombs(botsettings.username);
  char* utf8realname = utf8_wstringtombs(botsettings.realname);

  see_printstring(stdout, L"Connected to %hs...\n", HostName);
  BN_EnableFloodProtection(I, 1000, 1000, 60);
  connected = true;
  BN_Register(I, utf8nickname, utf8username, utf8realname);

  safe_free (utf8realname);
  safe_free (utf8username);
  safe_free (utf8nickname);
}


void ProcOnRegistered(BN_PInfo I) {
  see_printstring(stdout, L"Registered...\n");
  vector<wstring>::iterator it = botsettings.channels.begin();
  for (; it != botsettings.channels.end(); ++it) {
	char* utf8channel = utf8_wstringtombs(*it);
	see_printstring(stdout, L"Joining %ls...\n", (*it).c_str());
	BN_SendJoinMessage (I, utf8channel, NULL);
	safe_free(utf8channel);
  }
}
void ProcOnPingPong(BN_PInfo I) {
  static time_t oldtime = time(NULL);
  if (oldtime + botsettings.autosaveperiod < time(NULL)) {
	oldtime = time(NULL);
	SaveBotSettings();
	gSeeBorg.SaveSettings();
  }
}
// ---
void ProcOnInvite(BN_PInfo I,const char Chan[],const char Who[],const char Whom[]) {
  char  tempnick[4096];
  BN_ExtractNick(Who, tempnick, sizeof(tempnick));
  wstring nickname, channel;
  utf8_mbstowstring(tempnick, nickname);
  utf8_mbstowstring(Chan, channel);

  see_printstring(stdout, L"Received invitation to %ls by %ls\n", channel.c_str(), nickname.c_str());

  bool isowner = isOwner(Who);
  if (botsettings.joininvites == 0) return;
  if ((botsettings.joininvites > 1) && (!isowner)) return;

  BN_SendJoinMessage (I, Chan, NULL);
}

void ProcOnKick(BN_PInfo I, const char Chan[], const char Who[], const char Whom[], const char Msg[]) {
  char  tempnick[4096];
  BN_ExtractNick(Who, tempnick, sizeof(tempnick));
  wstring nickname, channel, whom, message;
  utf8_mbstowstring(tempnick, nickname);
  utf8_mbstowstring(Chan, channel);
  utf8_mbstowstring(Whom, whom);
  utf8_mbstowstring(Msg, message);
  see_printstring(stdout, L"(%ls) * %ls has been kicked from %ls by %ls [%ls]\n", 
	channel.c_str(), whom.c_str(), channel.c_str(), nickname.c_str(), message.c_str());
  
  if (strstr(Whom, I->Nick) != NULL) {
	// we were kicked, try to rejoin
	BN_SendJoinMessage (I, Chan, NULL);
  }
}


void ProcOnPrivateTalk(BN_PInfo I, const char Who[], const char Whom[], const char Msg[]) {
  char  tempnick[4096];
  BN_ExtractNick(Who, tempnick, sizeof(tempnick));
  wstring nickname, whom, message;
  utf8_mbstowstring(tempnick, nickname);
  utf8_mbstowstring(Whom, whom);
  utf8_mbstowstring(Msg, message);
  see_printstring(stdout, L"%ls: %ls\n", nickname.c_str(), message.c_str());

  wstring reply = ProcessMessage(I, Who, message, true);

  if (!reply.empty()) {
	vector<wstring> curlines;
	splitString(reply, curlines, L"\n");
	for (size_t i = 0, sz = curlines.size(); i < sz; i++) {
	  char* utf8line = utf8_wstringtombs(curlines[i]);
	  see_printstring(stdout, L"%ls -> %ls: %ls\n", whom.c_str(), nickname.c_str(), curlines[i].c_str());
	  BN_SendPrivateMessage(I, tempnick, utf8line);
	  safe_free (utf8line);
	}
  }
}


void ProcOnChannelTalk(BN_PInfo I,const char Chan[],const char Who[],const char Msg[]) {
  char  tempnick[4096];
  BN_ExtractNick(Who, tempnick, sizeof(tempnick));
  wstring nickname, channel, mynick, message;
  utf8_mbstowstring(tempnick, nickname);
  utf8_mbstowstring(Chan, channel);
  utf8_mbstowstring(Msg, message);

  see_printstring(stdout, L"(%ls) <%ls> %ls\n", channel.c_str(), nickname.c_str(), message.c_str());
  
  // Process incoming message, acquire reply
  wstring reply = ProcessMessage(I, Who, message);
  
  // if there's reply, send it
  if (!reply.empty()) {
	vector<wstring> curlines;
	utf8_mbstowstring(I->Nick, mynick);
	splitString(reply, curlines, L"\n");
	for (size_t i = 0, sz = curlines.size(); i < sz; i++) {
	  bool beginswithnickname = false;
	  vector<wstring> nicknames = botsettings.botakas;
	  nicknames.push_back(mynick);
	  size_t nnsz = nicknames.size();
	  for (size_t j = 0; j < nnsz; j++) {
		if (!wcsncasecmp(nicknames[j].c_str(), curlines[i].c_str(), nicknames[j].length())) {
		  beginswithnickname = true;
		  break;
		}
	  }
	  
	  // TODO: what a mess, clean it up!
	  if (!beginswithnickname) {
		char* utf8line = utf8_wstringtombs(curlines[i]);
		see_printstring(stdout, L"(%ls) <%ls> %ls\n", channel.c_str(), mynick.c_str(), curlines[i].c_str());
		BN_SendChannelMessage(I, Chan, utf8line);
		safe_free (utf8line);
	  } else {
		vector<wstring> words;
		splitString(curlines[i], words);
		words[0] = nickname;
		lowerString(words[0]);
		curlines[i] = joinString(words);
		trimString(curlines[i], false);
		char* utf8line = utf8_wstringtombs(curlines[i]);
		see_printstring(stdout, L"(%ls) <%ls> %ls\n", channel.c_str(), mynick.c_str(), curlines[i].c_str());
		BN_SendChannelMessage(I, Chan, utf8line);
		safe_free (utf8line);
	  }
	}
  }
}


void ProcOnAction(BN_PInfo I,const char Chan[],const char Who[],const char Msg[]) {
  char  tempnick[4096];
  BN_ExtractNick(Who, tempnick, sizeof(tempnick));
  wstring nickname, channel, action, mynick;
  utf8_mbstowstring(tempnick, nickname);
  utf8_mbstowstring(Chan, channel);
  utf8_mbstowstring(Msg, action);
  see_printstring(stdout, L"(%ls) * %ls %ls\n", channel.c_str(), nickname.c_str(), action.c_str());

  wstring message = nickname;
  message += L" ";
  message += action;

  wstring reply = ProcessMessage(I, Who, message);

  if (!reply.empty()) {
	vector<wstring> curlines;
	utf8_mbstowstring(I->Nick, mynick);
	splitString(reply, curlines, L"\n");
	for (size_t i = 0, sz = curlines.size(); i < sz; i++) {
	  bool beginswithnickname = false;
	  vector<wstring> nicknames = botsettings.botakas;
	  nicknames.push_back(mynick);
	  size_t nnsz = nicknames.size();
	  for (size_t j = 0; j < nnsz; j++) {
		if (!wcsncasecmp(nicknames[j].c_str(), curlines[i].c_str(), nicknames[j].length())) {
		  beginswithnickname = true;
		  break;
		}
	  }
	  
	  // TODO: what a mess, clean it up!
	  if (!beginswithnickname) {
		char* utf8line = utf8_wstringtombs(curlines[i]);
		see_printstring(stdout, L"(%ls) <%ls> %ls\n", channel.c_str(), mynick.c_str(), curlines[i].c_str());
		BN_SendChannelMessage(I, Chan, utf8line);
		safe_free (utf8line);
	  } else {
		vector<wstring> words;
		splitString(curlines[i], words);
		words[0] = nickname;
		lowerString(words[0]);
		curlines[i] = joinString(words);
		trimString(curlines[i], false);
		char* utf8line = utf8_wstringtombs(curlines[i]);
		see_printstring(stdout, L"(%ls) <%ls> %ls\n", channel.c_str(), mynick.c_str(), curlines[i].c_str());
		BN_SendChannelMessage(I, Chan, utf8line);
		safe_free (utf8line);
	  }
	}
  }
}


void ProcOnJoin (BN_PInfo I, const char Chan[],const char Who[]) {
  char  tempnick[4096];
  char  temphost[4096];
  char  tempuser[4096];
  BN_ExtractNick(Who, tempnick, sizeof(tempnick));
  BN_ExtractHost(Who, temphost, sizeof(temphost));
  BN_ExtractExactUserName(Who, tempuser, sizeof(tempuser));

  wstring nickname, hostname, username, channel, mynick;
  utf8_mbstowstring(tempnick, nickname);
  utf8_mbstowstring(temphost, hostname);
  utf8_mbstowstring(tempuser, username);
  utf8_mbstowstring(Chan, channel);
  
  see_printstring(stdout, L"(%ls) %ls (%ls@%ls) has joined the channel\n", 
	channel.c_str(), nickname.c_str(), username.c_str(), hostname.c_str());
  
  wstring reply = ProcessMessage(I, Who, nickname);

  if (!reply.empty()) {
	vector<wstring> curlines;
	splitString(reply, curlines, L"\n");
	utf8_mbstowstring(I->Nick, mynick);
	for (size_t i = 0, sz = curlines.size(); i < sz; i++) {
	  char* utf8line = utf8_wstringtombs(curlines[i]);
	  see_printstring(stdout, L"(%ls) %ls: %ls\n", 
		channel.c_str(), mynick.c_str(), curlines[i].c_str());
	  BN_SendChannelMessage(I, Chan, utf8line);
	  safe_free(utf8line);
	}
  }
}


void ProcOnPart (BN_PInfo I, const char Chan[],const char Who[], const char Msg[]) {
  char  tempnick[4096];
  char  temphost[4096];
  char  tempuser[4096];
  BN_ExtractNick(Who, tempnick, sizeof(tempnick));
  BN_ExtractHost(Who, temphost, sizeof(temphost));
  BN_ExtractExactUserName(Who, tempuser, sizeof(tempuser));

  wstring nickname, hostname, username, channel, mynick, message;
  utf8_mbstowstring(tempnick, nickname);
  utf8_mbstowstring(temphost, hostname);
  utf8_mbstowstring(tempuser, username);
  utf8_mbstowstring(Chan, channel);
  utf8_mbstowstring(Msg, message);
  
  see_printstring(stdout, L"(%ls) %ls (%ls@%ls) has left the channel (%ls)\n", 
	channel.c_str(), nickname.c_str(), username.c_str(), hostname.c_str(), message.c_str());

  wstring reply = ProcessMessage(I, Who, nickname);

  if (!reply.empty()) {
	vector<wstring> curlines;
	splitString(reply, curlines, L"\n");
	utf8_mbstowstring(I->Nick, mynick);
	for (size_t i = 0, sz = curlines.size(); i < sz; i++) {
	  char* utf8line = utf8_wstringtombs(curlines[i]);
	  see_printstring (stdout, L"(%ls) %ls: %ls\n", channel.c_str(), mynick.c_str(), curlines[i].c_str());
	  BN_SendChannelMessage(I, Chan, utf8line);
	  safe_free(utf8line);
	}
  }
}


void ProcOnQuit (BN_PInfo I, const char Who[],const char Msg[]) {
  char  tempnick[4096];
  char  temphost[4096];
  char  tempuser[4096];
  BN_ExtractNick(Who, tempnick, sizeof(tempnick));
  BN_ExtractHost(Who, temphost, sizeof(temphost));
  BN_ExtractExactUserName(Who, tempuser, sizeof(tempuser));

  wstring nickname, hostname, username, message;
  utf8_mbstowstring(tempnick, nickname);
  utf8_mbstowstring(temphost, hostname);
  utf8_mbstowstring(tempuser, username);
  utf8_mbstowstring(Msg, message);
  
  see_printstring(stdout, L"%ls (%ls@%ls) has quit IRC (%ls)\n", 
	nickname.c_str(), username.c_str(), hostname.c_str(), message.c_str());
}

// returned string is freed later by BotNet, so we malloc() the return string each call
char *ProcOnCTCP(BN_PInfo I,const char Who[],const char Whom[],const char Type[]) {
  char  tempnick[4096];
  char  temphost[4096];
  char  tempuser[4096];
  BN_ExtractNick(Who, tempnick, sizeof(tempnick));
  BN_ExtractHost(Who, temphost, sizeof(temphost));
  BN_ExtractExactUserName(Who, tempuser, sizeof(tempuser));

  wstring nickname, hostname, username, whom, type;
  utf8_mbstowstring(tempnick, nickname);
  utf8_mbstowstring(temphost, hostname);
  utf8_mbstowstring(tempuser, username);
  utf8_mbstowstring(Whom, whom);
  utf8_mbstowstring(Type, type);
  see_printstring(stdout, L"CTCP %ls query by %ls for %ls\n", type.c_str(), nickname.c_str(), whom.c_str());

  wstring replystring;
  if (!strcasecmp(Type, "VERSION")) {
	if (!botsettings.stealth) {
	  replystring = L"SeeBorg v" SEEBORGVERSIONWSTRING;
	} else {
	  replystring = botsettings.ctcpversionstring;
	}
  }

  return utf8_wstringtombs(replystring);
}

// Bot Commands body
// ---------------------------------------------------------------------------
wstring ircParseCommands(const wstring &cmd, const char* who) {
  if (cmd[0] != L'!') return L"";
  
  if (!isOwner(who)) return ABORT_HACK;
  // TODO: remove redundant copy
  wstring command = cmd;
  lowerString(command);

  tokenizer_tokenize(botsettings.tokenizer, command.c_str());

  for (int i = 0; i < numircbotcmds; i++) {
	size_t len = wcslen(ircbotcmds[i].command);
	if (!wcsncmp(tokenizer_argv(botsettings.tokenizer, 0) + 1, ircbotcmds[i].command, len)) {
	  return ircbotcmds[i].func(&gSeeBorg, cmd);
	}
  }
  return L"";
}

wstring CMD_Shutup_f(class SeeBorg* self, const wstring command) {
  if (!botsettings.speaking) return L"";
  
  botsettings.speaking = false;
  return L"I'll shut up... :o";
}

wstring CMD_Wakeup_f(class SeeBorg* self, const wstring command) {
  if (botsettings.speaking) return L"";
  
  botsettings.speaking = true;
  return L"Woohoo!";
}

wstring CMD_Save_f(class SeeBorg* self, const wstring command) {
  see_printstring(stdout, L"Saving settings...\n");
  SaveBotSettings();
  gSeeBorg.SaveSettings();
  return L"done";
}                                                             

wstring CMD_Join_f (class SeeBorg* self, const wstring command) {
  size_t argc = tokenizer_argc(botsettings.tokenizer);
  if (argc < 2) return L"";
  
  for (size_t i = 1; i < argc; i++) {
	wstring channel = tokenizer_argv(botsettings.tokenizer, i);
	char* utf8channel = utf8_wstringtombs(channel);

	see_printstring(stdout, L"Joining %ls...\n", channel.c_str());
	BN_SendJoinMessage (&Info, utf8channel, NULL);
	botsettings.channels.push_back(channel);

	safe_free(utf8channel);
  }
  
  return L"okay";
}


wstring CMD_Part_f (class SeeBorg* self, const wstring command) {
  size_t argc = tokenizer_argc(botsettings.tokenizer);
  if (argc < 2) return L"";
  
  for (size_t i = 1; i < argc; i++) {
	wstring channel = tokenizer_argv(botsettings.tokenizer, i);
	char* utf8channel = utf8_wstringtombs(channel);

	see_printstring(stdout, L"Leaving %ls...\n", channel.c_str());
	BN_SendPartMessage (&Info, utf8channel, NULL);
	
	vector<wstring>::iterator it = botsettings.channels.begin();
	for (; it != botsettings.channels.end(); ++it) {
	  if (!wcscasecmp((*it).c_str(), channel.c_str())) {
		botsettings.channels.erase(it);
		break;
	  }
	}
  }
  
  return L"okay";
}


wstring CMD_Replyrate_f(class SeeBorg* self, const wstring command) {
  size_t argc = tokenizer_argc(botsettings.tokenizer);
  wchar_t retstr[4096];
  if (argc < 2) {
	snwprintf (retstr, 4096, L"Reply rate is %.1f%%", botsettings.replyrate);
	return retstr;
  }
  
  botsettings.replyrate = (float)wcstod(tokenizer_argv(botsettings.tokenizer, 1), NULL);
  snwprintf (retstr, 4096, L"Reply rate is set to %.1f%%", botsettings.replyrate);
  return retstr;
}

wstring CMD_Replynick_f(class SeeBorg* self, const wstring command) {
  size_t argc = tokenizer_argc(botsettings.tokenizer);
  wchar_t retstr[4096];
  if (argc < 2) {
	snwprintf (retstr, 4096, L"Reply rate to nickname is %.1f%%", botsettings.replyrate_mynick);
	return retstr;
  }
  
  botsettings.replyrate_mynick = (float)wcstod(tokenizer_argv(botsettings.tokenizer, 1), NULL);
  snwprintf (retstr, 4096, L"Reply rate to nickname is set to %.1f%%", botsettings.replyrate_mynick);
  return retstr;
}

wstring CMD_Replyword_f(class SeeBorg* self, const wstring command) {
  size_t argc = tokenizer_argc(botsettings.tokenizer);
  wchar_t retstr[4096];
  if (argc < 2) {
	snwprintf (retstr, 4096, L"Reply rate to magic words is %.1f%%", botsettings.replyrate_magic);
	return retstr;
  }
  
  botsettings.replyrate_magic = (float)wcstod(tokenizer_argv(botsettings.tokenizer, 1), NULL);
  snwprintf (retstr, 4096, L"Reply rate to magic words is set to %.1f%%", botsettings.replyrate_magic);
  return retstr;
}

wstring CMD_ircHelp_f(class SeeBorg* self, const wstring command) {
  wstring retstr;
  retstr = L"IRC SeeBorg commands:\n";
  for (int i = 0; i < numircbotcmds; i++) {
	retstr += L"!";
	retstr += ircbotcmds[i].command;
	retstr += L": ";
	retstr += ircbotcmds[i].description;
	retstr += L"\n";
  }
  retstr += CMD_Help_f(self, command);
  
  return retstr;
}

wstring CMD_Stealth_f(class SeeBorg* self, const wstring command) {
  size_t argc = tokenizer_argc(botsettings.tokenizer);
  wstring retstr;
  if (argc < 2) {
	retstr = L"Stealth is ";
	retstr += (botsettings.stealth) ? L"enabled" : L"disabled";
	return retstr;
  }
  
  botsettings.stealth = wcstol(tokenizer_argv(botsettings.tokenizer, 1), NULL, 10);
  retstr = L"Stealth is set to ";
  retstr += (botsettings.stealth) ? L"enabled" : L"disabled";
  return retstr;
}

wstring CMD_Learning_f(class SeeBorg* self, const wstring command) {
  size_t argc = tokenizer_argc(botsettings.tokenizer);
  wstring retstr;
  if (argc < 2) {
	retstr = L"Learning is ";
	retstr += (botsettings.learning) ? L"enabled" : L"disabled";
	return retstr;
  }
  
  botsettings.learning = wcstol(tokenizer_argv(botsettings.tokenizer, 1), NULL, 10);
  retstr = L"Learning is set to ";
  retstr += (botsettings.learning) ? L"enabled" : L"disabled";
  return retstr;
}


// Main Body
// ---------------------------------------------------------------------------

void cleanup(void) {
  // handle this only when we are initialized
  if (initialized) {
	if (connected) {
	  char *utf8str = utf8_wstringtombs(botsettings.quitmessage);
	  see_printstring(stdout, L"Disconnecting from server...\n");
	  BN_SendQuitMessage(&Info, utf8str);
	  safe_free(utf8str);
	}
	see_printstring(stdout, L"Saving dictionary...\n");
	gSeeBorg.SaveSettings();
	see_printstring(stdout, L"Saving settings...\n");
	SaveBotSettings();
	tokenizer_free(botsettings.tokenizer);
	botsettings.tokenizer = NULL;
  }
}


typedef void (*sighandler_t)(int);

void sig_term(int i) {
  static int si = 0;
  if (!si) {
	si++;
	// Save the settings before returning back to default signal handler
	cleanup();
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
#ifndef _WIN32
	// Windows doesn't define these signals
	signal(SIGQUIT, SIG_DFL);
	signal(SIGHUP, SIG_DFL);
#endif
	_exit(0);
  }
}


int main (int argc, char* argv[]) {
  setlocale(LC_ALL, "");
  
  see_printstring(stdout, L"SeeBorg v" SEEBORGVERSIONWSTRING L", copyright (C) 2003 Eugene Bujak.\n"
	L"Uses botnet v%hs\n", BN_GetVersion());
  
  LoadBotSettings();
  if (argc < 2) {
	if (botsettings.server.empty()) {
	  SaveBotSettings();
	  see_printstring(stdout, L"No server to connect to (check seeborg-irc.cfg)\n");
	  return 1;
	}
  } else {
	utf8_mbstowstring(argv[1], botsettings.server);
  }
  
  
  memset (&Info, 0, sizeof(Info));

  Info.CB.OnError = ProcOnError;
  Info.CB.OnConnected = ProcOnConnected;
  Info.CB.OnRegistered = ProcOnRegistered;
  Info.CB.OnCTCP = ProcOnCTCP;
  Info.CB.OnInvite = ProcOnInvite;
  Info.CB.OnKick = ProcOnKick;
  Info.CB.OnPrivateTalk = ProcOnPrivateTalk;
  Info.CB.OnAction = ProcOnAction;
  Info.CB.OnJoin = ProcOnJoin;
  Info.CB.OnPart = ProcOnPart;
  Info.CB.OnQuit = ProcOnQuit;
  Info.CB.OnChannelTalk = ProcOnChannelTalk;
  Info.CB.OnPingPong = ProcOnPingPong;
  
  
  srand(time(NULL));
  see_printstring(stdout, L"Loading dictionary...\n");
  gSeeBorg.LoadSettings();
  signal(SIGINT, sig_term);
  signal(SIGTERM, sig_term);
#ifndef _WIN32
  // Windows doesn't define these signals
  signal(SIGQUIT, sig_term);
  signal(SIGHUP, sig_term);
#endif
  atexit(cleanup);
  
  initialized = true;
  char* utf8server = utf8_wstringtombs(botsettings.server);
  while(BN_Connect(&Info, utf8server, botsettings.serverport, 0) != true) {
	see_printstring(stdout, L"Disconnected.\n");
#ifdef __unix__
	sleep(10);
#elif defined(_WIN32)
	Sleep(10*1000);
#endif
	see_printstring(stdout, L"Reconnecting...\n");
  }
  safe_free (utf8server);
  return 0;
}

// Emacs editing variables
// ---------------------------------------------------------------------------
/*
** Local variables:
**  tab-width: 4 
*/
