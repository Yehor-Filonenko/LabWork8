#pragma once

#include "LabWork8.h"

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
    void add(const Client& c);
    const std::vector<Client>& all() const;

    // Повертає індекси клієнтів без призначеного майстра
    std::vector<int> unassignedIndices() const;

    Client& get(size_t index);
};

// Форма введення даних нового клієнта; повертає false при скасуванні
class AddClientForm {
public:
    static bool fill(Client& c);
};

// Відображення таблиці всіх замовлень
class OrderListView {
    static void printHeader();
    static void printRow(const Client& c);
public:
    static void show(ClientRepository& repo);
};

// Вибір непризначеного клієнта та введення імені майстра
class AssignMasterView {
public:
    static void show(ClientRepository& repo);
};

// Фасад бізнес логіки автосервісу
class AutoService {
    ClientRepository repo;
public:
    void addClient();
    void showClients();
    void assignMaster();
};

// Меню адміністратора автосервісу
class AdminMenu {
    AutoService& service;
public:
    explicit AdminMenu(AutoService& s);
    void run();
};

// Точка входу модуля автосервісу
class AutoServiceProgram : public IProgram {
    AutoService service;
    AdminMenu   admin{ service };
public:
    std::string name() const override;
    void run() override;
};