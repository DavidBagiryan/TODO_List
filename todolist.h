#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>

using namespace std::literals;

// ���������� �����
std::string ReadLine(std::istream& istream);

// ��������� ����� �� ��������
std::vector<std::string_view> SplitIntoWords(std::string_view str, const char div = ' ');

namespace to_do_list {
    static const std::string error_command__ = "[�������� ������ �������.]"s;
    static const std::string error_data__ = "[�������� ������ ������.]"s;
    static const std::string error_name_no__ = "[����� ������ �� ����������.]"s;
    static const std::string error_name_yes__ = "[����� ������ ��� ����������. ������ � ��������?]\n[yes/no]"s;

    static const std::string line__ = "----------------------------"s;

    static const std::string category__[] = { "CATEGORY1"s, "CATEGORY2"s, "CATEGORY3"s };

    // ������ ���������
    enum Category {
        DEFAULT = 0,
        CATEGORY1 = 1,
        CATEGORY2 = 2,
        CATEGORY3 = 3
    };

    std::ostream& operator<<(std::ostream& out, const Category& category);

    // ������ ��������
    enum Status {
        PROCESS = 0,
        DONE = 1
    };

    std::ostream& operator<<(std::ostream& out, const Status& status);

    // ��������� �������� ���� � �������
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

    // �������� ���� �� ������������
    bool IsLeap(const int year);

    // ��������� ���������� � ������
    struct ToDoPage {
        std::string name = ""s;
        std::string description = ""s;
        Date date;
        Category category = Category::DEFAULT;
        Status status = Status::PROCESS;
    };

    // ��������������� ��������� ���������� � ���������� �� ��������
    struct SortDescription {
        std::string description = ""s;
        bool description_like = false;
    };
    // ��������������� ��������� ���������� � ���������� �� ����
    struct SortDate {
        Date date;
        std::string compare = ""s;
    };

    class ToDoList {
        using Header = std::string;
        using Variable = std::variant<SortDescription, SortDate, Category, Status>;
    public:
        // ���������� ��������� ����������
        bool EnterCommand(std::istream& input = std::cin, std::ostream& output = std::cout);

    private:
        std::unordered_map<Header, ToDoPage> list_; // ������ ���� �����

        // ������� ������
        void Help(std::ostream& output = std::cout);
        // ������� �������� ������
        void Add(ToDoPage page);
        // ������� �������� ������ ��� �����������
        void Done(std::string_view name);
        // ������� �������� ������
        void Update(std::string_view name, std::istream& input = std::cin, std::ostream& output = std::cout);
        // ������� ������� ������
        void Delete(std::string_view name);
        // ������� ������������� ������ ����� �� ���������
        std::unordered_map<Header, ToDoPage> Select(std::string&& predicate);

        // �������� ������� ���� � ����� � ������� y-m-d h:m
        std::string GetTimeNow();
        // �������� ������ ������
        void StatusChange(const Header& name, const Status& status);
        // ���������� ������ ����� �� ������������� �������� ���������
        std::variant<SortDescription, SortDate> ProcessPredicateSortDynamicValue(std::string& predicate, size_t op_quo);
        // ���������� ������ ����� �� ��������
        SortDescription ProcessPredicateSortDescription(bool like, std::string& predicate, size_t op_quo, size_t id_space_af);
        // ��������� ������ �������� �� ���������
        std::string FindDescriptonText(const std::string& text, const size_t op_quo, size_t size_for_find);
        // ���������� ������ ����� �� ����
        SortDate ProcessPredicateSortDate(const std::string& compare, std::string& predicate, size_t op_quo, size_t id_space_af);
        // ���������� ������ ����� �� ������������ �������� ���������
        std::variant<Category, Status> ProcessPredicateSortStaticValue(std::string& predicate, size_t op_quo);
        // ��������� �������� ���������
        Category GetCategory(const std::string& config);
        // ��������� �������� �������
        Status GetStasus(const std::string& config);
        // �������� ������� �������� ����� and � ���������
        void DeletePredicateAnd(std::string& predicate, size_t id_space);

        // ��������� ���������������� ������
        std::unordered_map<Header, ToDoPage> SortByPredicate(const std::unordered_map<Header, ToDoPage>& list, const Variable& comp);
    };

    std::ostream& operator<<(std::ostream& out, const std::unordered_map<std::string, ToDoPage>& list);
} // namespace ToDo_List