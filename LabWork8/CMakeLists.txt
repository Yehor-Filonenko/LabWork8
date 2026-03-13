# CMakeList.txt: проект CMake для LabWork8; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

# Добавьте источник в исполняемый файл этого проекта.
add_executable (LabWork8 "LabWork8.cpp" "LabWork8.h"   "Console.cpp" "Console.h" "Autoservice.cpp" "Autoservice.h" "ShoppingList.cpp" "ShoppingList.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LabWork8 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
