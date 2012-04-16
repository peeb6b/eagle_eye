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


#include <time.h>
#include <locale.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "seeborg.h"
#include "seeutil.h"

int main (int argc, char* argv[]) {
  setlocale(LC_ALL, "");
  see_printstring(stdout, L"SeeBorg v" SEEBORGVERSIONWSTRING L", copyright (C) 2003 Eugene Bujak.\n\n");
  wstring body;
  srand(time(NULL));

  see_printstring(stdout, L"Loading dictionary...\n");
  gSeeBorg.LoadSettings();

#ifndef PROFILE
  see_printstring(stdout, L"\nSeeBorg offline chat. Type '!exit' to save and exit. Press CTRL-C to quit.\n\n");

#ifdef _WIN32
  HANDLE cinput = GetStdHandle(STD_INPUT_HANDLE);
  wchar_t inbuffer[16384];
  DWORD readchars = 0;
#endif

  while (1) {
	wprintf (L"> ");
#ifdef _WIN32
	ReadConsole(cinput, inbuffer, 16383, &readchars, NULL);
	inbuffer[readchars] = L'\0';
	body = inbuffer;
#else
	fReadStringLine(stdin, body);
#endif
	FilterMessage(body);
	if (!wcsncasecmp(body.c_str(), L"!exit", 5)) break;
	if (!wcsncasecmp(body.c_str(), L"!quit", 5)) break;
	wstring seeout = gSeeBorg.Reply(body);
	see_printstring (stdout, L"<SeeBorg> %ls\n", seeout.c_str());
	gSeeBorg.Learn(body);
  }
#endif

  gSeeBorg.SaveSettings();
  return 0;
}
