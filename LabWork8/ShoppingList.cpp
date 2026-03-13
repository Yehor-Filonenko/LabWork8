#include "shoppingList.h"
#include "console.h"

// FileNode

FileNode::FileNode(const std::string& n) : m_name(n) {}
std::string FileNode::name() const { return m_name; }
void        FileNode::setName(const std::string& n) { m_name = n; }
bool        FileNode::isFolder() const { return false; }

// FolderNode

FolderNode::FolderNode(const std::string& n) : m_name(n) {}
std::string FolderNode::name() const { return m_name; }
void        FolderNode::setName(const std::string& n) { m_name = n; }
bool        FolderNode::isFolder() const { return true; }
bool        FolderNode::isOpen() const { return opened; }
void        FolderNode::toggle() { opened = !opened; }

std::vector<std::unique_ptr<Node>>& FolderNode::items() { return children; }

void FolderNode::add(std::unique_ptr<Node> n) {
    children.push_back(std::move(n));
}

void FolderNode::remove(int index) {
    if (index >= 0 && index < (int)children.size())
        children.erase(children.begin() + index);
}

// TreeService

FolderNode& TreeService::getRoot() { return root; }

void TreeService::buildVisible(FolderNode& folder, int depth, std::vector<VisibleNode>& out) {
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

std::vector<VisibleNode> TreeService::visible() {
    std::vector<VisibleNode> list;
    list.push_back({ &root, nullptr, 0, 0 });
    if (root.isOpen())
        buildVisible(root, 1, list);
    return list;
}

// ShoppingTreeView

void ShoppingTreeView::draw(const std::vector<VisibleNode>& nodes, int selected) {
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

// ShoppingListProgram

std::string ShoppingListProgram::name() const { return "Список покупок"; }

void ShoppingListProgram::run() {
    int selected = 0;

    while (true) {
        auto nodes = tree.visible();

        if (!nodes.empty() && selected >= (int)nodes.size())
            selected = (int)nodes.size() - 1;

        ShoppingTreeView::draw(nodes, selected);

        WORD key = Console::getKey(); // _WIN32

        if (key == Key::ESC) return;

        if (key == Key::UP && selected > 0)                       selected--;
        if (key == Key::DOWN && selected < (int)nodes.size() - 1)   selected++;

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