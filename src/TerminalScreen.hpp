#include <iostream>

class TerminalScreen {
public:
    TerminalScreen() {
        // Альтернативный экран и скрытие курсора
        std::cout << "\033[?1049h";
        std::cout << "\033[?25l";
        std::cout.flush();
    }

    ~TerminalScreen() {
        // Возвращаем исходное состояние терминала
        std::cout << "\033[?25h";
        std::cout << "\033[?1049l";
        std::cout.flush();
    }

    TerminalScreen(const TerminalScreen&) = delete;
    TerminalScreen& operator=(const TerminalScreen&) = delete;
};