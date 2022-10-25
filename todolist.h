#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>

using namespace std::literals;

// считывание строк
std::string ReadLine(std::istream& istream);

// разбиение строк по делителю
std::vector<std::string_view> SplitIntoWords(std::string_view str, const char div = ' ');

namespace to_do_list {
    static const std::string error_command__ = "[Ќеверный формат команды.]"s;
    static const std::string error_data__ = "[Ќеверный формат данных.]"s;
    static const std::string error_name_no__ = "[“акой задачи не существует.]"s;
    static const std::string error_name_yes__ = "[“ака€ задача уже существует. ’отите еЄ заменить?]\n[yes/no]"s;

    static const std::string line__ = "----------------------------"s;

    static const std::string category__[] = { "CATEGORY1"s, "CATEGORY2"s, "CATEGORY3"s };

    // список категорий
    enum Category {
        DEFAULT = 0,
        CATEGORY1 = 1,
        CATEGORY2 = 2,
        CATEGORY3 = 3
    };

    std::ostream& operator<<(std::ostream& out, const Category& category);

    // список статусов
    enum Status {
        PROCESS = 0,
        DONE = 1
    };

    std::ostream& operator<<(std::ostream& out, const Status& status);

    // структура хранени€ даты и времени
    struct Date {
        int year = 0;
        std::uint8_t mounth = 1;
        std::uint8_t day = 1;
        std::uint8_t hour = 0;
        std::uint8_t min = 0;

        Date() = default;
        Date(const std::string_view date_time_text);

        Date& operator=(const std::string_view date_time_text);
    };

    inline bool operator==(const Date& lhs, const Date& rhs) {
        return lhs.year == rhs.year && lhs.mounth == rhs.mounth && lhs.day == rhs.day &&
            lhs.hour == rhs.hour && lhs.min == rhs.min;
    }

    inline bool operator<(const Date& lhs, const Date& rhs) {
        return lhs.year < rhs.year || lhs.year == rhs.year && lhs.mounth < rhs.mounth ||
            lhs.year == rhs.year && lhs.mounth == rhs.mounth && lhs.day < rhs.day ||
            lhs.year == rhs.year && lhs.mounth == rhs.mounth && lhs.day == rhs.day && lhs.hour < rhs.hour ||
            lhs.year == rhs.year && lhs.mounth == rhs.mounth && lhs.day == rhs.day && lhs.hour == rhs.hour && lhs.min < rhs.min;
    }

    inline bool operator<=(const Date& lhs, const Date& rhs) {
        return !(rhs < lhs);
    }

    inline bool operator>(const Date& lhs, const Date& rhs) {
        return rhs < lhs;
    }

    inline bool operator>=(const Date& lhs, const Date& rhs) {
        return !(rhs > lhs);
    }

    std::ostream& operator<<(std::ostream& out, const Date& date);

    // проверка года на високосность
    bool IsLeap(const int year);

    // структура информации о зачаче
    struct ToDoPage {
        std::string name = ""s;
        std::string description = ""s;
        Date date;
        Category category = Category::DEFAULT;
        Status status = Status::PROCESS;
    };

    // вспомогательна€ структура информации о сортировке по описанию
    struct SortDescription {
        std::string description = ""s;
        bool description_like = false;
    };
    // вспомогательна€ структура информации о сортировке по дате
    struct SortDate {
        Date date;
        std::string compare = ""s;
    };

    class ToDoList {
        using Header = std::string;
        using Variable = std::variant<SortDescription, SortDate, Category, Status>;
    public:
        // обработчик введенной информации
        bool EnterCommand(std::istream& input = std::cin, std::ostream& output = std::cout);

    private:
        std::unordered_map<Header, ToDoPage> list_; // список всех задач

        // команда ѕомощь
        void Help(std::ostream& output = std::cout);
        // команда ƒобавить задачу
        void Add(ToDoPage page);
        // команда ќтметить задачу как выполненную
        void Done(std::string_view name);
        // команда ќбновить задачу
        void Update(std::string_view name, std::istream& input = std::cin, std::ostream& output = std::cout);
        // команда ”далить задачу
        void Delete(std::string_view name);
        // команда ќтфильтровать список задач по предикату
        std::unordered_map<Header, ToDoPage> Select(std::string&& predicate);

        // получить текущие дату и врем€ в формате y-m-d h:m
        std::string GetTimeNow();
        // изменить статус задачи
        void StatusChange(const Header& name, const Status& status);
        // фильтраци€ списка задач по динамическому значению предиката
        std::variant<SortDescription, SortDate> ProcessPredicateSortDynamicValue(std::string& predicate, size_t op_quo);
        // фильтраци€ списка задач по описанию
        SortDescription ProcessPredicateSortDescription(bool like, std::string& predicate, size_t op_quo, size_t id_space_af);
        // выделение текста описани€ из предиката
        std::string FindDescriptonText(const std::string& text, const size_t op_quo, size_t size_for_find);
        // фильтраци€ списка задач по дате
        SortDate ProcessPredicateSortDate(const std::string& compare, std::string& predicate, size_t op_quo, size_t id_space_af);
        // фильтраци€ списка задач по статическому значению предиката
        std::variant<Category, Status> ProcessPredicateSortStaticValue(std::string& predicate, size_t op_quo);
        // получение значени€ категории
        Category GetCategory(const std::string& config);
        // получение значени€ статуса
        Status GetStasus(const std::string& config);
        // удаление лишнего кллючего слова and в предикате
        void DeletePredicateAnd(std::string& predicate, size_t id_space);

        // получение отфильтрованного списка
        std::unordered_map<Header, ToDoPage> SortByPredicate(const std::unordered_map<Header, ToDoPage>& list, const Variable& comp);
    };

    std::ostream& operator<<(std::ostream& out, const std::unordered_map<std::string, ToDoPage>& list);
} // namespace ToDo_List