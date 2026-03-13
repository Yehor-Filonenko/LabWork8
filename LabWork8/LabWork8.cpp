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

// Обрізає рядок до ширини w, додаючи "~" якщо текст не вміщується
std::string trunc(const std::string& s, int w) {
    if (static_cast<int>(s.size()) > w)
        return s.substr(0, w - 2) + "~";
    return s;
}

// Робота з консоллю: ініціалізація, очищення, введення, позиціонування курсора
namespace Console {

    // Встановлює кодову сторінку та розмір вікна консолі
    void init() {
#ifdef _WIN32
        SetConsoleCP(1251);
        SetConsoleOutputCP(1251);

        // Може змінитися під час роботи
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

        COORD size = { Layout::CONSOLE_W, Layout::CONSOLE_H };
        SetConsoleScreenBufferSize(h, size);

        SMALL_RECT rect = { 0, 0, Layout::CONSOLE_W - 1, Layout::CONSOLE_H - 1 };
        SetConsoleWindowInfo(h, TRUE, &rect);
#endif
    }

    // Очищає буфер екрана та переміщує курсор у початок
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
        HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
        INPUT_RECORD ir;
        DWORD read;

        while (true) {
            ReadConsoleInput(h, &ir, 1, &read);

            if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown)
                return ir.Event.KeyEvent.wVirtualKeyCode;
        }
    }

    // Переміщує курсор на передостанній рядок вікна
    void moveCursorToBottom() {
#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(h, &info);

        COORD pos = { 0, (static_cast<SHORT>(info.srWindow.Bottom) - 1) };
        SetConsoleCursorPosition(h, pos);
#endif
    }

    // Переміщує курсор у задану позицію
    void setCursor(int x, int y) {
#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
        SetConsoleCursorPosition(h, pos);
#endif
    }

    // Виводить підказку внизу консолі
    void showHotkeys(const char* hint) {
        moveCursorToBottom();
        std::cout << hint << std::flush;
    }
}

// Зчитує ввід користувача, false якщо натиснуто Esc
bool readLineEsc(const char* prompt, std::string& out) {
    std::cout << prompt;
    out.clear();

    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD ir;
    DWORD read;

    while (true) {
        ReadConsoleInputW(h, &ir, 1, &read);

        if (ir.EventType != KEY_EVENT || !ir.Event.KeyEvent.bKeyDown)
            continue;

        WORD  vk  = ir.Event.KeyEvent.wVirtualKeyCode;
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
}

// Інтерактивний вибір зі списку; повертає індекс або -1 при Esc
int selectFromList(
    const std::vector<std::string>& items,
    const std::string& title,
    const char* hint = "Esc - назад  Up/Down - вибір  Enter - підтвердити")
{
    int selected = 0;

    while (true) {
        Console::clear();
        std::cout << title << "\n\n";

        for (int i = 0; i < (int)items.size(); i++)
            std::cout << (i == selected ? " > " : "   ") << items[i] << "\n";

        Console::showHotkeys(hint);

        WORD key = Console::getKey();

        if (key == Key::ESC)                                       return -1;
        if (key == Key::ENTER)                                     return selected;
        if (key == Key::UP   && selected > 0)                      selected--;
        if (key == Key::DOWN && selected < (int)items.size() - 1)  selected++;
    }
}

// Інтерфейс программи, якими керує ProgramManager
class IProgram {
public:
    virtual std::string name() const = 0;
    virtual void run() = 0;
    virtual ~IProgram() = default;
};

// АВТОСЕРВІС

// Дані одного замовлення
struct Client {
    std::string name;
    std::string car;
    std::string problem;
    std::string note;
    std::string master;
};

// Сховище клієнтів з базовими операціями доступу
class ClientRepository {
    std::vector<Client> clients;
public:
    void add(const Client& c) { clients.push_back(c); }

    const std::vector<Client>& all() const { return clients; }

    // Повертає індекси клієнтів без призначеного майстра
    std::vector<int> unassignedIndices() const {
        std::vector<int> result;
        for (int i = 0; i < (int)clients.size(); i++)
            if (clients[i].master.empty())
                result.push_back(i);
        return result;
    }

    Client& get(size_t index) {
        if (index >= clients.size())
            throw std::out_of_range("Індекс клієнта поза межами");
        return clients[index];
    }
};

// Форма введення даних нового клієнта; повертає false при скасуванні
class AddClientForm {
public:
    static bool fill(Client& c) {
        Console::clear();
        std::cout << "Новий клієнт\n\n";
        Console::showHotkeys("Esc - скасувати");
        Console::setCursor(0, 2);

        if (!readLineEsc("Прізвище та ім'я : ", c.name))    return false;
        if (!readLineEsc("Марка автомобіля : ", c.car))     return false;
        if (!readLineEsc("Несправність     : ", c.problem)) return false;
        if (!readLineEsc("Примітка         : ", c.note))    return false;

        c.master = "";
        return true;
    }
};

// Відображення таблиці всіх замовлень
class OrderListView {
    static void printHeader() {
        std::cout << std::left
            << std::setw(Layout::COL_NAME) << "Клієнт"
            << std::setw(Layout::COL_CAR)  << "Авто"
            << std::setw(Layout::COL_PROB) << "Несправність"
            << std::setw(Layout::COL_NOTE) << "Примітка"
            << std::setw(Layout::COL_MAST) << "Майстер"
            << "\n";

        std::cout << std::string(Layout::CONSOLE_W, '-') << "\n";
    }

    static void printRow(const Client& c) {
        std::cout << std::left
            << std::setw(Layout::COL_NAME) << trunc(c.name,    Layout::COL_NAME)
            << std::setw(Layout::COL_CAR)  << trunc(c.car,     Layout::COL_CAR)
            << std::setw(Layout::COL_PROB) << trunc(c.problem, Layout::COL_PROB)
            << std::setw(Layout::COL_NOTE) << trunc(c.note,    Layout::COL_NOTE)
            << std::setw(Layout::COL_MAST) << trunc(c.master.empty() ? "-" : c.master, Layout::COL_MAST)
            << "\n";
    }

public:
    static void show(ClientRepository& repo) {
        Console::clear();
        std::cout << "Список замовлень\n\n";

        printHeader();
        for (const auto& c : repo.all())
            printRow(c);

        Console::showHotkeys("Esc / Enter - назад");
        Console::getKey();
    }
};

// Вибір непризначеного клієнта та введення імені майстра
class AssignMasterView {
public:
    static void show(ClientRepository& repo) {
        auto indices = repo.unassignedIndices();

        if (indices.empty()) {
            Console::clear();
            std::cout << "Немає непризначених клієнтів\n";
            Console::showHotkeys("Esc / Enter - назад");
            Console::getKey();
            return;
        }

        std::vector<std::string> items;
        for (auto i : indices)
            items.push_back(repo.get(i).name + " / " + repo.get(i).car);

        int choice = selectFromList(items, "Вибір клієнта");
        if (choice == -1) return;

        Client& client = repo.get(indices[choice]);

        Console::clear();
        std::cout << "Клієнт: " << client.name << "\n\n";
        std::string m;
        if (!readLineEsc("Майстер: ", m)) return;
        client.master = m;
    }
};

// Фасад бізнес логіки автосервісу
class AutoService {
    ClientRepository repo;
public:
    void addClient()    { Client c; if (AddClientForm::fill(c)) repo.add(c); }
    void showClients()  { OrderListView::show(repo); }
    void assignMaster() { AssignMasterView::show(repo); }
};

// Меню адміністратора автосервісу
class AdminMenu {
    AutoService& service;
public:
    AdminMenu(AutoService& s) : service(s) {}

    void run() {
        std::vector<std::string> items = {
            "Список замовлень",
            "Призначити майстра"
        };

        while (true) {
            int c = selectFromList(items, "Адміністратор");
            if (c == -1) return;
            if (c == 0)  service.showClients();
            if (c == 1)  service.assignMaster();
        }
    }
};

// Точка входу модуля автосервісу
class AutoServiceProgram : public IProgram {
    AutoService service;
    AdminMenu   admin{ service };
public:
    std::string name() const override { return "Автосервіс"; }

    void run() override {
        std::vector<std::string> items = { "Клієнт", "Адміністратор" };

        while (true) {
            int c = selectFromList(items, "Автосервіс");
            if (c == -1) return;
            if (c == 0)  service.addClient();
            if (c == 1)  admin.run();
        }
    }
};

// СПИСОК ПОКУПОК (дерево)
// Абстрактний вузол дерева
class Node {
public:
    virtual std::string name() const = 0;
    virtual void setName(const std::string& n) = 0;
    virtual bool isFolder() const = 0;
    virtual ~Node() = default;
};

// Листовий вузол (елемент списку)
class FileNode : public Node {
    std::string m_name;
public:
    explicit FileNode(const std::string& n) : m_name(n) {}
    std::string name() const override { return m_name; }
    void setName(const std::string& n) override { m_name = n; }
    bool isFolder() const override { return false; }
};

// Вузол папка (може містити дочірні вузли)
class FolderNode : public Node {
    std::string m_name;
    bool        opened = true;
    std::vector<std::unique_ptr<Node>> children;
public:
    explicit FolderNode(const std::string& n) : m_name(n) {}
    std::string name() const override { return m_name; }
    void setName(const std::string& n) override { m_name = n; }
    bool isFolder() const override { return true; }

    bool isOpen() const { return opened; }
    void toggle() { opened = !opened; }

    std::vector<std::unique_ptr<Node>>& items() { return children; }

    void add(std::unique_ptr<Node> n) { children.push_back(std::move(n)); }
    void remove(int index) {
        if (index >= 0 && index < children.size())
            children.erase(children.begin() + index);
    }
};

// Запис видимого вузла з контекстом для відображення та редагування.
// Вказівники не власні, вузли живуть у дереві FolderNode
struct VisibleNode {
    Node* node;          // відображуваний вузол
    FolderNode* parent;        // батьківська папка (nullptr для кореня)
    int         depth;         // глибина вкладеності
    int         indexInParent; // позиція серед дочірніх елементів батька
};

// Керує деревом та будує список видимих вузлів для відображення
class TreeService {
    FolderNode root{ "Список покупок" };
public:
    FolderNode& getRoot() { return root; }

    // Рекурсивно заповнює out вузлами відкритих папок
    void buildVisible(FolderNode& folder, int depth, std::vector<VisibleNode>& out) {
        int i = 0;
        for (auto& n : folder.items()) {
            out.push_back({ n.get(), &folder, depth, i });

            if (n->isFolder()) {
                auto* f = static_cast<FolderNode*>(n.get());
                if (f->isOpen())
                    buildVisible(*f, depth + 1, out);
            }
            ++i;
        }
    }

    // Повертає плаский список всіх видимих вузлів, починаючи з кореня
    std::vector<VisibleNode> visible() {
        std::vector<VisibleNode> list;
        list.push_back({ &root, nullptr, 0, 0 });
        if (root.isOpen())
            buildVisible(root, 1, list);
        return list;
    }
};

// Відображає дерево з курсором вибору та підказкою клавіш
class ShoppingTreeView {
public:
    static void draw(const std::vector<VisibleNode>& nodes, int selected) {
        Console::clear();
        std::cout << "Список покупок\n\n";

        for (int i = 0; i < (int)nodes.size(); i++) {
            const auto& v = nodes[i];

            // Відступ за глибиною + символ папки або елемента
            std::string prefix(v.depth, ' ');

            if (v.node->isFolder()) {
                auto* f = static_cast<FolderNode*>(v.node);
                prefix += f->isOpen() ? TreeSymbol::FOLDER_OPEN : TreeSymbol::FOLDER_CLOSED;
            }
            else {
                prefix += TreeSymbol::ITEM_INDENT;
            }

            std::cout
                << (i == selected ? TreeSymbol::CURSOR_ON : TreeSymbol::CURSOR_OFF)
                << prefix
                << v.node->name()
                << "\n";
        }

        Console::showHotkeys(
            "Enter - відкрити/згорнути  A - елемент  F - папка  D - видалити  R - перейменувати  Esc - назад"
        );
    }
};

// Точка входу модуля списку покупок
class ShoppingListProgram : public IProgram {
    TreeService tree;
public:
    std::string name() const override { return "Список покупок"; }

    void run() override {
        int selected = 0;

        while (true) {
            auto nodes = tree.visible();

            if (!nodes.empty() && selected >= (int)nodes.size())
                selected = (int)nodes.size() - 1;

            ShoppingTreeView::draw(nodes, selected);

            WORD key = Console::getKey();

            if (key == Key::ESC) return;

            if (key == Key::UP && selected > 0)                      selected--;
            if (key == Key::DOWN && selected < (int)nodes.size() - 1)  selected++;

            // Enter — розкрити / згорнути папку
            if (key == Key::ENTER && !nodes.empty()) {
                if (nodes[selected].node->isFolder())
                    static_cast<FolderNode*>(nodes[selected].node)->toggle();
            }

            // A — додати елемент до поточної папки або папки батька
            if (key == Key::KEY_A) {
                Console::clear();
                std::cout << "Додати елемент\n\n";
                Console::showHotkeys("Esc - скасувати");
                Console::setCursor(0, 2);

                std::string nm;
                if (!readLineEsc("Назва: ", nm)) continue;

                FolderNode* parent = &tree.getRoot();
                if (!nodes.empty()) {
                    auto& v = nodes[selected];
                    parent = v.node->isFolder()
                        ? static_cast<FolderNode*>(v.node)
                        : (v.parent ? v.parent : &tree.getRoot());
                }
                parent->add(std::make_unique<FileNode>(nm));
            }

            // F — додати підпапку до поточної папки або папки батька
            if (key == Key::KEY_F) {
                Console::clear();
                std::cout << "Додати папку\n\n";
                Console::showHotkeys("Esc - скасувати");
                Console::setCursor(0, 2);

                std::string nm;
                if (!readLineEsc("Назва: ", nm)) continue;

                FolderNode* parent = &tree.getRoot();
                if (!nodes.empty()) {
                    auto& v = nodes[selected];
                    if (v.node->isFolder())
                        parent = static_cast<FolderNode*>(v.node);
                    else if (v.parent)
                        parent = v.parent;
                }
                parent->add(std::make_unique<FolderNode>(nm));
            }

            // D — видалити поточний вузол (не корінь)
            if (key == Key::KEY_D && !nodes.empty()) {
                auto& v = nodes[selected];
                if (v.parent == nullptr) continue;
                v.parent->remove(v.indexInParent);
                if (selected > 0) selected--;
            }

            // R — перейменувати поточний вузол
            if (key == Key::KEY_R && !nodes.empty()) {
                Console::clear();
                std::cout << "Перейменувати: " << nodes[selected].node->name() << "\n\n";
                Console::showHotkeys("Esc - скасувати");
                Console::setCursor(0, 2);

                std::string nm;
                if (!readLineEsc("Нова назва: ", nm)) continue;
                nodes[selected].node->setName(nm);
            }
        }
    }
};

// Реєструє програми та запускає головне меню вибору між ними
class ProgramManager {
    std::vector<std::unique_ptr<IProgram>> programs;
public:
    void add(std::unique_ptr<IProgram> p) { programs.push_back(std::move(p)); }

    void run() {
        while (true) {
            std::vector<std::string> items;
            for (const auto& p : programs)
                items.push_back(p->name());

            int c = selectFromList(items, "Головне меню",
                "Esc - вихід  Up/Down - вибір  Enter - відкрити");
            if (c == -1) return;
            programs[c]->run();
        }
    }
};

int main() {
    try {

        Console::init();

        ProgramManager manager;
        manager.add(std::make_unique<AutoServiceProgram>());
        manager.add(std::make_unique<ShoppingListProgram>());

        manager.run();

    } catch (const std::exception& e) {
            std::cerr << "Помилка: " << e.what() << std::endl;
    }

    return 0;
}