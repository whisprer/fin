#include "../core/woflang.hpp"
#include <iostream>
#include <string>

int main() {
    woflang::WoflangInterpreter interp;
    std::cout << "Welcome to Woflang REPL (plugin/egg/curses ready!)\n";
    std::string line;
    while (std::cout << "wof> ", std::getline(std::cin, line)) {
        interp.execute_line(line);
    }
}
