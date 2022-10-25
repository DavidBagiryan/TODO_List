#include <iostream>
#include <Windows.h>

#include "todolist.h"

int main()
{
    SetConsoleCP(1251);         // устанавливает вывод данных в кодировку 1251 - +русский язык
    SetConsoleOutputCP(1251);   // устанавливает ввод данных в кодировку 1251 - +русский язык
    //setlocale(LC_ALL, "Russian");

    std::ostream& output = std::cout;
    std::istream& input = std::cin;

    bool process = true;
    to_do_list::ToDoList test;
    do {
        try {
             process = test.EnterCommand(input, output);
        }
        catch (...) { output << to_do_list::error_command__ << std::endl; }
    } while (process);
}