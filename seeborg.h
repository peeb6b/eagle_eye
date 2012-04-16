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
#include "seeutil.h"

#ifndef __SEEBORG_H__
#define __SEEBORG_H__

#include <map>
#include <vector>
#include <string>
#include <set>
#include <deque>

typedef struct botcommand_s {
	const wchar_t* command;
	const wchar_t* description;
	wstring (*func) (class SeeBorg* self, const wstring cmd);
} botcommand_t;

typedef pair<set<wstring>::iterator,size_t> context_t;
typedef vector<context_t> word_t;

typedef set<wstring> lines_t;
typedef map<wstring, word_t> words_t;

class SeeBorg {
public:
  SeeBorg();
  ~SeeBorg();
  
  int Learn(const wstring &body);
  wstring Reply(const wstring message);
  
  int LoadSettings(void);
  int SaveSettings(void);
  
  wstring ParseCommands(const wstring &cmd);
  
  int num_contexts;

  lines_t lines;
  words_t words;
  tokenizer_t* tokenizer;

private:
  typedef enum direction_e {
	DIR_LEFT,
	  DIR_RIGHT,
	  DIR_INVALID
  } direction_t;

  int LearnLine(const wstring &line);
  int BuildReply(direction_t direction, deque<wstring> &sentence);

  int min_context_depth;
  int max_context_depth;
};

typedef class SeeBorg seeborg_t;

extern seeborg_t gSeeBorg;

// Bot commands
wstring CMD_Help_f (class SeeBorg* self, const wstring command);
wstring CMD_Version_f (class SeeBorg* self, const wstring command);
wstring CMD_Words_f (class SeeBorg* self, const wstring command);
wstring CMD_Known_f (class SeeBorg* self, const wstring command);
wstring CMD_Contexts_f (class SeeBorg* self, const wstring command);
wstring CMD_Unlearn_f (class SeeBorg* self, const wstring command);
wstring CMD_Replace_f (class SeeBorg* self, const wstring command);
wstring CMD_Quit_f (class SeeBorg* self, const wstring command);

static botcommand_t botcmds[] = {
  {L"help", L"Show this command list", CMD_Help_f},
  {L"version", L"Show SeeBorg version", CMD_Version_f},
  {L"words", L"Show how many words the borg knows", CMD_Words_f},
  {L"known", L"Query the bot if the word is known", CMD_Known_f},

  {L"contexts", L"Show contexts containing the command argument", CMD_Contexts_f},
  {L"unlearn", L"Unlearn the command argument", CMD_Unlearn_f},
  {L"replace", L"Replace all occurences of old word with new one", CMD_Replace_f},

  {L"quit", L"As the name implies", CMD_Quit_f},

  {NULL, NULL, NULL}
};
static int numbotcmds = sizeof(botcmds) / sizeof(botcmds[0]) - 1;

// ---------------------------------------------------------------------------

#endif
