#pragma once

#include "LabWork8.h"

// Зчитує ввід користувача, false якщо натиснуто Esc
bool readLineEsc(const char* prompt, std::string& out);

// Інтерактивний вибір зі списку; повертає індекс або -1 при Esc
int selectFromList(
    const std::vector<std::string>& items,
    const std::string& title,
    const char* hint = "Esc - назад  Up/Down - вибір  Enter - підтвердити");

// Робота з консоллю: ініціалізація, очищення, введення, позиціонування курсора
namespace Console {

    // Встановлює кодову сторінку та розмір вікна консолі
    void init();

    // Очищає буфер екрана та переміщує курсор у початок
    void clear();

    // Чекає натискання клавіші та повертає VirtualKey
    WORD getKey(); // _WIN32

    // Переміщує курсор на передостанній рядок вікна
    void moveCursorToBottom();

    // Переміщує курсор у задану позицію
    void setCursor(int x, int y);

    // Виводить підказку внизу консолі
    void showHotkeys(const char* hint);
}