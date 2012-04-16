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


#include <stdio.h>

#include "seeborg.h"
#include "seeutil.h"

seeborg_t gSeeBorg;

SeeBorg::SeeBorg() {
  this->num_contexts = 0;
  this->max_context_depth = 10;
  this->min_context_depth = 5;
  this->tokenizer = tokenizer_init();
}

SeeBorg::~SeeBorg() {
  tokenizer_free(this->tokenizer);
}

int SeeBorg::LoadSettings(void) {
	// TODO: WIP
	wstring str;
	FILE *f = fopen ("lines.txt", "rb");
	if (f == NULL) {
		printf ("Not found, creating dictionary.\n");
		return false;
	}
	
	while (fReadStringLine (f, str)) {
		this->Learn(str);
	}
	fclose(f);
	
	see_printstring(stdout, L"Parsed %i lines.\n", lines.size());
	see_printstring(stdout, L"I know %i words (%i contexts, %.2f per word), %i lines.\n", words.size(), 
		num_contexts, (float)num_contexts/(float)words.size(), lines.size());
	
	return true;
}

int SeeBorg::SaveSettings(void) {
	set<wstring>::iterator it;
	FILE *f = NULL; 
	
	f = fopen ("lines.txt", "wb");
	if (f == NULL) {
		perror("Couldn't save lines dictionary");
		return false;
	}
	
	utf8writer_t *utf8writer = utf8writer_init();
	if (utf8writer == NULL) {
		// TODO: write error
		fclose(f);
		return false;
	}
	
	for (it = lines.begin(); it != lines.end(); ++it) {
		utf8writer_write(utf8writer, f, (*it).c_str(), "%s\n");
	}

	utf8writer_free(utf8writer);
	
	fclose(f);
	return true;
}


int SeeBorg::BuildReply(direction_t direction, deque<wstring> &sentence) {
	bool done = false;


	return done;
}


wstring SeeBorg::Reply(IN const wstring inmsg) {
	vector<wstring> curlines, curwords, index;
	deque<wstring> sentence;
	wstring replystring;
	int i, sz;
	bool done;

	if (inmsg.empty()) return L"";

	wstring message = inmsg;
	FilterMessage(message);
	splitString(message, curlines, L". ");
	
	sz = curlines.size();
	for (i = 0; i < sz; i++) splitString(curlines[i], curwords);
	if (curwords.empty()) return L"";
	
	for (sz = curwords.size(), i = 0; i < sz; i++) {
		// Make an index of known words
		wstring &x = curwords[i];
		if (words.find(x) == words.end()) continue;

		index.push_back(x);
	}
	if (index.empty()) return L"";
	
	// pick a random word as a start for building reply
	sentence.push_back(index[rand() % index.size()]);
	
	// Build on the left edge
	done = false;
	while (!done) {
		if (words.find(sentence.back()) == words.end()) {
			see_printstring(stderr, L"%hs:%i: Shouldn't happen - words.find(sentence.back()) == words.end()\n", __FILE__, __LINE__);
		}

		vector<wstring> linewords;
		context_t wordcontext;
		int contexts, wordposition;
		
		contexts = this->words[sentence.front()].size();
		wordcontext = this->words[sentence.front()][rand() % (contexts)];

		splitString(*(wordcontext.first), linewords);

		wordposition = wordcontext.second;
		
		int depth = randInt(min_context_depth, max_context_depth);
		for (int i = 1; i <= depth; i++) {
			if ((wordposition - i) < 0) {
				done = true;
				break;
			} else {
				sentence.push_front(linewords[wordposition-i]);
			}
			if ((wordposition - i) == 0) {
				done = true;
				break;
			}
		}
	}
	
	// Build on the right edge
	done = false;
	while (!done) {
		if (words.find(sentence.back()) == words.end()) {
			see_printstring(stderr, L"%hs:%i: Shouldn't happen - words.find(sentence.back()) == words.end()\n", __FILE__, __LINE__);
		}
		
		vector<wstring> linewords;
		context_t wordcontext;
		int contexts, wordposition;

		contexts = this->words[sentence.back()].size();
		wordcontext = this->words[sentence.back()][rand() % (contexts)];
		
		splitString (*(wordcontext.first), linewords);
		wordposition = wordcontext.second;
		
		size_t depth = randInt(min_context_depth, max_context_depth);
		for (size_t i = 1; i <= depth; i++) {
			if ((wordposition + i) >= linewords.size()) {
				done = true;
				break;
			} else {
				sentence.push_back(linewords[wordposition+i]);
			}
		}
	}
	
	for (i = 0, sz = sentence.size() - 1; i < sz; i++) {
		replystring += sentence[i];
		replystring += L' ';
	}
	replystring += sentence.back();
	
	trimString(replystring, false);
	
	return replystring;
}


int SeeBorg::Learn(const wstring &body) {
	// Ignore quotes
	if (iswdigit(body[0])) return false;
	if (body[0] == L'<') return false;
	if (body[0] == L'[') return false;
	
	// TODO: move FilterMessage up in callchain
	wstring message = body;
	FilterMessage(message);

	vector<wstring> curlines;
	splitString(message, curlines, L". ");
	
	int sz = curlines.size();
	for (int i = 0; i < sz; i++) {
		LearnLine(curlines[i]);
	}
	
	return true;
}

int SeeBorg::LearnLine(const wstring &line) {
	vector<wstring> curwords;
	splitString(line, curwords);
	wstring cleanline = joinString(curwords);
	
	// check to see if we've learned this line already
	if (lines.find(cleanline) != lines.end()) return false;
	
	set<wstring>::iterator lineit = lines.insert(cleanline).first;
	
	int sz = curwords.size();
	for (int i = 0; i < sz; i++) {
		map<wstring, word_t>::iterator wit = words.find(curwords[i]);
		if (wit == words.end()) {
			word_t cword;
			context_t cxt(lineit, i);
			cword.push_back(cxt);
			words[curwords[i]] = cword;
		} else {
			context_t cxt(lineit, i);
			((*wit).second).push_back(cxt);
		}
		num_contexts++;
	}
	
	return true;
}

// ---------------------------------------------------------------------------

wstring SeeBorg::ParseCommands(const wstring &cmd) {
	if (cmd[0] != '!') return L"";
	wstring command = cmd;
	lowerString(command);
	tokenizer_tokenize(this->tokenizer, cmd.c_str());
	//  CMA_TokenizeString(command.c_str());
	for (int i = 0; i < numbotcmds; i++) {
		if (!wcsncasecmp(tokenizer_argv(tokenizer, 0) + 1, botcmds[i].command, wcslen(botcmds[i].command))) {
			return botcmds[i].func(this, cmd);
		}
	}
	return L"";
}

wstring CMD_Help_f(class SeeBorg* self, wstring command) {
	static wstring retstr;
	retstr = L"Core SeeBorg commands:\n";
	for (int i = 0; i < numbotcmds; i++) {
		retstr += L"!";
		retstr += botcmds[i].command;
		retstr += L": ";
		retstr += botcmds[i].description;
		retstr += L"\n";
	}
	return retstr;
}

wstring CMD_Version_f(class SeeBorg* self, wstring command) {
	return L"I am SeeBorg v" SEEBORGVERSIONWSTRING L" by Eugene 'HMage' Bujak";
}

wstring CMD_Words_f(class SeeBorg* self, wstring command) {
	static wchar_t retstr[4096];
	
	snwprintf (retstr, 4096, L"I know %i words (%i contexts, %.2f per word), %i lines.", 
		self->words.size(), self->num_contexts, 
		self->num_contexts/(float)self->words.size(),
		self->lines.size());
	retstr[4095] = L'\0';
	return retstr;
}

wstring CMD_Known_f(class SeeBorg* self, wstring command) {
	if (tokenizer_argc(self->tokenizer) < 2) return L"Not enough parameters, usage: !known <word>";
	wstring curword;
	curword = tokenizer_argv(self->tokenizer, 1);
	lowerString(curword);
	
	words_t::iterator wit = self->words.find(curword);
	wchar_t retstr[4096];
	if (wit != self->words.end()) {
		int wordcontexts = ((*wit).second).size();
		snwprintf (retstr, 4096, L"\"%ls\" is known (known as \"%ls\", %i contexts)", tokenizer_argv(self->tokenizer, 1), curword.c_str(), wordcontexts);
	} else {
		snwprintf (retstr, 4096, L"%ls is unknown, (was looking for \"%ls\")", tokenizer_argv(self->tokenizer, 1), curword.c_str());
	}
	retstr[4095] = L'\0';
	
	return retstr;
}

wstring CMD_Contexts_f(class SeeBorg* self, wstring command) {
  if (tokenizer_argc(self->tokenizer) < 2) return L"Not enough parameters, usage: !contexts <word>";
  wstring curword = tokenizer_argv(self->tokenizer, 1);
  lowerString(curword);
  wstring retstr;

  words_t::iterator wit = self->words.find(curword);
  if (wit != self->words.end()) {
	// word is known
	size_t count = self->words[curword].size();
	retstr = curword + L" is known, has ";
	retstr.append(wva(L"%i", count));
	retstr += L" contexts";


	if (count > 20) {
	  if ((tokenizer_argc(self->tokenizer) > 2) && (!wcscmp(tokenizer_argv(self->tokenizer, 2), L"YES"))) {
		;
	  } else {
		retstr += L", which is quite much, I won't list them unless you type \"!contexts ";
		retstr += curword;
		retstr += L" YES\".";
		return retstr;
	  }
	}
	retstr += L", listing goes below:\n";
	for (size_t i = 0; i < count; i++) {
	  context_t ctx = self->words[curword][i];
	  retstr += *(ctx.first) + L"\n";
	}
  } else {
	retstr = L"\"";
	retstr += curword + L"\" is unknown";
  }

  return retstr;
}

wstring CMD_Unlearn_f(class SeeBorg* self, wstring command) {
	return L"Not implemented yet";
}

wstring CMD_Replace_f(class SeeBorg* self, wstring command) {
	return L"Not implemented yet";
}

wstring CMD_Quit_f(class SeeBorg* self, wstring command) {
	exit(0);
	
	return L"Wow!";
}

