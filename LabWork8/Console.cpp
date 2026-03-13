#include "console.h"

std::string trunc(const std::string& s, int w) {
    if (static_cast<int>(s.size()) > w)
        return s.substr(0, w - 2) + "~";
    return s;
}

namespace Console {

    void init() {
#ifdef _WIN32
        SetConsoleCP(1251);
        SetConsoleOutputCP(1251);

        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

        COORD size = { Layout::CONSOLE_W, Layout::CONSOLE_H };
        SetConsoleScreenBufferSize(h, size);

        SMALL_RECT rect = { 0, 0, Layout::CONSOLE_W - 1, Layout::CONSOLE_H - 1 };
        SetConsoleWindowInfo(h, TRUE, &rect);
#endif
    }

    void clear() {
#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(h, &csbi);

        DWORD cells = csbi.dwSize.X * csbi.dwSize.Y;
        DWORD written;

        FillConsoleOutputCharacter(h, ' ', cells, { 0, 0 }, &written);
        FillConsoleOutputAttribute(h, csbi.wAttributes, cells, { 0, 0 }, &written);

        SetConsoleCursorPosition(h, { 0, 0 });
#endif
    }

    // Чекає натискання клавіші та повертає VirtualKey
    WORD getKey() { 
#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_INPUT_HANDLE); 
        INPUT_RECORD ir;                            
        DWORD read;                                

        while (true) {
            ReadConsoleInput(h, &ir, 1, &read);

            if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) 
                return ir.Event.KeyEvent.wVirtualKeyCode;
        }
#endif
    }

    void moveCursorToBottom() {
#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(h, &info);

        COORD pos = { 0, (static_cast<SHORT>(info.srWindow.Bottom) - 1) };
        SetConsoleCursorPosition(h, pos);
#endif
    }

    void setCursor(int x, int y) {
#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
        SetConsoleCursorPosition(h, pos);
#endif
    }

    void showHotkeys(const char* hint) {
        moveCursorToBottom();
        std::cout << hint << std::flush;
    }
}

bool readLineEsc(const char* prompt, std::string& out) {
    std::cout << prompt;
    out.clear();

#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE); 
    INPUT_RECORD ir;                            
    DWORD read;                                 

    while (true) {
        ReadConsoleInputW(h, &ir, 1, &read); 

        if (ir.EventType != KEY_EVENT || !ir.Event.KeyEvent.bKeyDown) 
            continue;

        WORD  vk = ir.Event.KeyEvent.wVirtualKeyCode; 
        WCHAR wch = ir.Event.KeyEvent.uChar.UnicodeChar; 

        if (vk == VK_ESCAPE) return false; 

        if (vk == VK_RETURN) {
            std::cout << "\n";
            if (!out.empty()) return true;
            continue;
        }

        if (vk == VK_BACK) {
            if (!out.empty()) {
                out.pop_back();
                std::cout << "\b \b";
            }
            continue;
        }

        // Конвертуємо Unicode у CP1251 та додає до рядка
        if (wch >= 32) {
            char mb[4] = {};
            int n = WideCharToMultiByte(
                1251, 0, &wch, 1, mb, sizeof(mb), nullptr, nullptr);

            for (int i = 0; i < n; i++) {
                out.push_back(mb[i]);
                std::cout << mb[i];
            }
        }
    }
#endif
}

int selectFromList(
    const std::vector<std::string>& items,
    const std::string& title,
    const char* hint)
{
    int selected = 0;

    while (true) {
        Console::clear();
        std::cout << title << "\n\n";

        for (int i = 0; i < (int)items.size(); i++)
            std::cout << (i == selected ? " > " : "   ") << items[i] << "\n";

        Console::showHotkeys(hint);

        WORD key = Console::getKey(); // _WIN32

        if (key == Key::ESC)                                       return -1;
        if (key == Key::ENTER)                                     return selected;
        if (key == Key::UP && selected > 0)                      selected--;
        if (key == Key::DOWN && selected < (int)items.size() - 1)  selected++;
    }
}