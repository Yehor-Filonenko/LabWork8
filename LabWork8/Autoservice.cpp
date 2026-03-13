#include "autoservice.h"
#include "console.h"

// ClientRepository

void ClientRepository::add(const Client& c) {
    clients.push_back(c);
}

const std::vector<Client>& ClientRepository::all() const {
    return clients;
}

std::vector<int> ClientRepository::unassignedIndices() const {
    std::vector<int> result;
    for (int i = 0; i < (int)clients.size(); i++)
        if (clients[i].master.empty())
            result.push_back(i);
    return result;
}

Client& ClientRepository::get(size_t index) {
    if (index >= clients.size())
        throw std::out_of_range("Індекс клієнта поза межами");
    return clients[index];
}

// AddClientForm

bool AddClientForm::fill(Client& c) {
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

// OrderListView

void OrderListView::printHeader() {
    std::cout << std::left
        << std::setw(Layout::COL_NAME) << "Клієнт"
        << std::setw(Layout::COL_CAR)  << "Авто"
        << std::setw(Layout::COL_PROB) << "Несправність"
        << std::setw(Layout::COL_NOTE) << "Примітка"
        << std::setw(Layout::COL_MAST) << "Майстер"
        << "\n";

    std::cout << std::string(Layout::CONSOLE_W, '-') << "\n";
}

void OrderListView::printRow(const Client& c) {
    std::cout << std::left
        << std::setw(Layout::COL_NAME) << trunc(c.name,    Layout::COL_NAME)
        << std::setw(Layout::COL_CAR)  << trunc(c.car,     Layout::COL_CAR)
        << std::setw(Layout::COL_PROB) << trunc(c.problem, Layout::COL_PROB)
        << std::setw(Layout::COL_NOTE) << trunc(c.note,    Layout::COL_NOTE)
        << std::setw(Layout::COL_MAST) << trunc(c.master.empty() ? "-" : c.master, Layout::COL_MAST)
        << "\n";
}

void OrderListView::show(ClientRepository& repo) {
    Console::clear();
    std::cout << "Список замовлень\n\n";

    printHeader();
    for (const auto& c : repo.all())
        printRow(c);

    Console::showHotkeys("Esc / Enter - назад");
    Console::getKey();
}

// AssignMasterView

void AssignMasterView::show(ClientRepository& repo) {
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

// AutoService

void AutoService::addClient() { Client c; if (AddClientForm::fill(c)) repo.add(c); }
void AutoService::showClients() { OrderListView::show(repo); }
void AutoService::assignMaster() { AssignMasterView::show(repo); }

// AdminMenu

AdminMenu::AdminMenu(AutoService& s) : service(s) {}

void AdminMenu::run() {
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

// AutoServiceProgram
std::string AutoServiceProgram::name() const { return "Автосервіс"; }

void AutoServiceProgram::run() {
    std::vector<std::string> items = { "Клієнт", "Адміністратор" };

    while (true) {
        int c = selectFromList(items, "Автосервіс");
        if (c == -1) return;
        if (c == 0)  service.addClient();
        if (c == 1)  admin.run();
    }
}