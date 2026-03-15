#include "NotepadApp.h"
#include <consoleapi.h>
#include <cstdio>
#include "LineTextDocument.h"
#include <regex>
#include <iostream>

void Test() {
	std::string str =
		"int main() {"
		"std::cout << \"Hello World\";"
		"std::cout << \"Hello Again!\";"
		"return 0;"
		"}";

	std::regex reg0("\"(?:[^\"])*\"");
	std::regex reg1("return|break|public");
	std::regex reg2("int|float|bool");
	std::regex reg3("ההההה");

	std::regex rules[] = { reg0,reg1,reg2,reg3 };

	

	std::string::const_iterator searchStart(str.cbegin());
	

	int pos = 0;
	while (pos < (int)str.length()) {

		std::smatch match;
		std::string sub = str.substr(pos);


		int bestType = -1;
		for (int i = 0; i < 4; i++) {

			if (std::regex_search(sub, match, rules[i]) && match.position() == 0) {
				bestType = i;
				break;
			}
		}

		if (bestType != -1) {
			std::cout <<"[" << match.str() << "]";
			pos += match.length();
		}
		else {

			std::cout << str[pos];
			pos += 1;
		}
		
		
	}
}

void NotepadApp::Run()
{
	AllocConsole();

	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONOUT$", "w", stdin);

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	//ITextDocument* doc = new LineTextDocument("Hello World");

	Test();
	std::cout << "---------------\n";

	m_Window.Create();
	m_Window.Show();
	m_Window.StartMessageLoop();
}
