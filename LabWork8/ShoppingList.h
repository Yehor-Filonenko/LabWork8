#pragma once

#include "LabWork8.h"

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
    explicit FileNode(const std::string& n);
    std::string name() const override;
    void setName(const std::string& n) override;
    bool isFolder() const override;
};

// Вузол папка (може містити дочірні вузли)
class FolderNode : public Node {
    std::string m_name;
    bool        opened = true;
    std::vector<std::unique_ptr<Node>> children;
public:
    explicit FolderNode(const std::string& n);
    std::string name() const override;
    void setName(const std::string& n) override;
    bool isFolder() const override;

    bool isOpen() const;
    void toggle();

    std::vector<std::unique_ptr<Node>>& items();

    void add(std::unique_ptr<Node> n);
    void remove(int index);
};

// Запис видимого вузла з контекстом для відображення та редагування.
// Вказівники не власні: вузли живуть у дереві FolderNode
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
    FolderNode& getRoot();

    // Рекурсивно заповнює out вузлами відкритих папок
    void buildVisible(FolderNode& folder, int depth, std::vector<VisibleNode>& out);

    // Повертає плаский список всіх видимих вузлів, починаючи з кореня
    std::vector<VisibleNode> visible();
};

// Відображає дерево з курсором вибору та підказкою клавіш
class ShoppingTreeView {
public:
    static void draw(const std::vector<VisibleNode>& nodes, int selected);
};

// Точка входу модуля списку покупок
class ShoppingListProgram : public IProgram {
    TreeService tree;
public:
    std::string name() const override;
    void run() override;
};