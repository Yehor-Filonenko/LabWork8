#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <memory>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

// Розміри консолі та ширини колонок таблиці
namespace Layout {
    constexpr int CONSOLE_W = 120;
    constexpr int CONSOLE_H = 30;

    constexpr int COL_NAME = 24;
    constexpr int COL_CAR  = 24;
    constexpr int COL_PROB = 24;
    constexpr int COL_NOTE = 24;
    constexpr int COL_MAST = 24;
}

// Коди віртуальних клавіш 
// https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
// _WIN32
namespace Key {
    constexpr WORD ESC   = VK_ESCAPE;
    constexpr WORD ENTER = VK_RETURN;
    constexpr WORD UP    = VK_UP;
    constexpr WORD DOWN  = VK_DOWN;
    constexpr WORD BACK  = VK_BACK; 

    constexpr WORD KEY_A = 'A';
    constexpr WORD KEY_D = 'D';
    constexpr WORD KEY_F = 'F';
    constexpr WORD KEY_R = 'R';
}

// Символи для відображення дерева
namespace TreeSymbol {
    constexpr const char* FOLDER_OPEN   = "- ";
    constexpr const char* FOLDER_CLOSED = "+ ";
    constexpr const char* ITEM_INDENT   = "  ";
    constexpr const char* CURSOR_ON     = "> ";
    constexpr const char* CURSOR_OFF    = "  ";
}

// Обрізає рядок до ширини w, додаючи "~"
std::string trunc(const std::string& s, int w);

// Інтерфейс програми, якою керує ProgramManager
class IProgram {
public:
    virtual std::string name() const = 0;
    virtual void run() = 0;
    virtual ~IProgram() = default;
};
