#include "console.h"
#include "autoservice.h"
#include "shoppingList.h"

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

    }
    catch (const std::exception& e) {
        std::cerr << "Помилка: " << e.what() << std::endl;
    }

    return 0;
}