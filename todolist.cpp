#include "todolist.h"

#include <unordered_set>

// считывание строк
std::string ReadLine(std::istream& istream) {
	std::string s;
	std::getline(istream, s);
	return s;
}

// разбиение строк по делителю
std::vector<std::string_view> SplitIntoWords(std::string_view str, const char div) {
	std::vector<std::string_view> result;
	const int64_t pos_end = str.npos;
	while (true) {
		int64_t space = str.find(div);
		result.push_back(space == pos_end ? str : str.substr(0, space));
		if (space == pos_end) {
			break;
		}
		else {
			str.remove_prefix(space + 1);
		}
	}
	return result;
}

using namespace to_do_list;

std::ostream& to_do_list::operator<<(std::ostream& out, const Category& category) {
	switch (category) {
	case Category::CATEGORY1:
		return out << "CATEGORY1"s;
	case Category::CATEGORY2:
		return out << "CATEGORY2"s;
	case Category::CATEGORY3:
		return out << "CATEGORY3"s;
	default:
		return out << "DEFAULT"s;
	}
}

std::ostream& to_do_list::operator<<(std::ostream& out, const Status& status) {
	switch (status) {
	case Status::PROCESS:
		return out << "PROCESS"s;
	case Status::DONE:
		return out << "DONE"s;
	default:
		return out;
	}
}

Date::Date(const std::string_view date_time_text)
{
	*this = date_time_text;
}

Date& Date::operator=(const std::string_view date_time_text) {
	Date old_date = *this;
	try {
		auto date_time = SplitIntoWords(date_time_text);
		if (date_time.size() != 2) throw error_data__;
		auto date = SplitIntoWords(date_time[0], '-');
		if (date.size() != 3) throw error_data__;
		auto time = SplitIntoWords(date_time[1], ':');
		if (time.size() != 2) throw error_data__;

		const std::string numbers = "1234567890"s;
		for (const std::string_view date_text : date) if (date_text.find_first_not_of(numbers) != date_text.npos) throw error_data__;
		for (const std::string_view time_text : time) if (time_text.find_first_not_of(numbers) != time_text.npos) throw error_data__;

		year = std::stoi(std::string(date[0]));
		mounth = std::uint8_t(std::stoi(std::string(date[1])));
		if (!(1 <= mounth && mounth <= 12)) throw error_data__;

		day = std::uint8_t(std::stoi(std::string(date[2])));
		switch (mounth) {
		case 1 :
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			if (!(1 <= day && day <= 31))throw error_data__;
			break;
		case 2:
			if (IsLeap(year)) {
				if (!(1 <= day && day <= 29))throw error_data__;
			}
			else {
				if (!(1 <= day && day <= 28))throw error_data__;
			}
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			if (!(1 <= day && day <= 30))throw error_data__;
			break;
		}

		hour = std::uint8_t(std::stoi(std::string(time[0])));
		if (!(0 <= hour && hour <= 23)) throw error_data__;

		min = std::uint8_t(std::stoi(std::string(time[1])));
		if (!(0 <= min && min <= 59)) throw error_data__;
	}
	catch (...) {
		std::cerr << "[Неверный формат даты/времени]"s << std::endl;
		year = old_date.year;
		mounth = old_date.mounth;
		day = old_date.day;
		hour = old_date.hour;
		min = old_date.min;
	}
	return *this;
}

std::ostream& to_do_list::operator<<(std::ostream& out, const Date& date) {
	return out << std::to_string(date.year) + "-"s + std::to_string(date.mounth) + "-"s + std::to_string(date.day) + 
		" "s + std::to_string(date.hour) + ":"s + std::to_string(date.min);
}

// проверка года на високосность
bool to_do_list::IsLeap(const int year) {
	return (year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0));
}

// обработчик введенной информации
bool ToDoList::EnterCommand(std::istream& input, std::ostream& output) {
	output << "- Введите команду. Список и описание доступных комманд можно узнать командой help."s << std::endl;
	std::string query = ReadLine(input);
	ToDoPage page;
	auto query_words = SplitIntoWords(query);

	if (query_words.size() == 1) {
		if (query_words[0] == "help"s) {
			Help(output);
			return true;
		}
		else if (query_words[0] == "exit"s) return false;
	}

	if (query_words.size() < 2) throw error_command__;

	if (query_words[0] == "add"s) {
		if (query_words.size() != 2) throw error_command__;
		page.name = query_words[1];
		// проверка наличия заголовка в списке и дальнейшая обработка
		if (list_.find(page.name) != list_.end()) {
			output << error_name_yes__ << std::endl;
			while (true) {
				std::string answer = ReadLine(input);
				if (answer == "yes") {
					Update(page.name, input, output);
					return true;
				}
				else if (answer == "no") return true;
				output << "[yes/no]"s << std::endl;
			}
		}

		output << "- Введите описание задачи."s << std::endl;
		page.description = ReadLine(input);

		const Date default_date;
		do {
			output << "- Введите дату и время в формате y-m-d h:m (2020-12-12 00:00) или введите time, чтобы вставить текущие дату и время."s << std::endl;
			std::string date = ReadLine(input);
			if (date == "time"s) {
				date = GetTimeNow();
				output << "[Записаны текущие дата и время - "s + date + "]"s << std::endl;
			}
			page.date = std::move(date);
		} while (page.date == default_date);

		Category category = Category::DEFAULT;
		while (category == Category::DEFAULT) {
			output << "- Введите категорию задачи. Узнать доступные категории можно командой help_category."s << std::endl;
			std::string text = ReadLine(input);
			if (text == "help_category"s) {
				output << "[Доступные категории:]\n"s << line__ << '\n';
				for (const std::string& c : category__) output << c << '\n';
				output << line__ << std::endl;
				continue;
			}

			try {
				category = GetCategory(text);
				page.category = std::move(category);
			}
			catch (...) {
				output << error_data__ << std::endl;
				category = Category::DEFAULT;
			}
		}

		Add(page);
		output << "[Задача добавлена в список.]\n"s << line__ << std::endl;
	}
	else if (query_words[0] == "done"s) {
		if (query_words.size() != 2) {
			output << error_data__ << std::endl;
			return true;
		}
		try {
			Done(query_words[1]);
			output << "[Задача отмечена выполненной.]\n"s << line__ << std::endl;
		}
		catch (...) {
			output << error_name_no__ << std::endl;
			return true;
		}
	}
	else if (query_words[0] == "update"s) {
		if (query_words.size() != 2) {
			output << error_data__ << std::endl;
			return true;
		}
		try {
			Update(query_words[1], input, output);
			output << "[Задача обновлена.]\n"s << line__ << std::endl;
		}
		catch (...) {
			output << error_name_no__ << std::endl;
			return true;
		}
	}
	else if (query_words[0] == "delete"s) {
		if (query_words.size() != 2) {
			output << error_command__ << std::endl;
			return true;
		}
		try {
			Delete(query_words[1]);
			output << "[Задача удалена из списка.]\n"s << line__ << std::endl;
		}
		catch (...) {
			output << error_name_no__ << std::endl;
			return true;
		}
	}
	else if (query_words[0] == "select"s && query_words[1] == "*"s) {
		if (query_words.size() == 2) {
			if (list_.empty()) output << "[Список задач пуст.]"s << std::endl;
			else output << "[Задач в списке: "s << list_.size() << "]\n"s << list_;
		}
		else if (query_words[2] == "where"s) {
			size_t id_op = query.find('{');
			size_t id_cl = query.find('}');
			if (id_op == query.npos || id_cl == query.npos || id_cl != query.size() - 1) {
				output << error_data__ << std::endl;
				return true;
			}
			std::string predicate = query.substr(id_op + 1, id_cl - id_op - 1);
			try {
				auto list = Select(std::move(predicate));
				output << "[Выборка завершена. Количество совпадений: "s << list.size() << "]\n"s << list;
			}
			catch (...) {
				output << error_command__ << std::endl;
				return true;
			}
		}
		else {
			output << error_command__ << std::endl;
			return true;
		}
	}
	else {
		output << error_command__ << std::endl;
		return true;
	}
	return true;
}

// получить текущие дату и время в формате y-m-d h:m
std::string ToDoList::GetTimeNow() {
	int year, mon, mday, hour, min;
	const time_t tStart = time(0);
	tm tmStart;
	localtime_s(&tmStart, &tStart);
	year = 1900 + tmStart.tm_year; mon = tmStart.tm_mon; mday = tmStart.tm_mday; hour = tmStart.tm_hour; min = tmStart.tm_min;
	return std::to_string(year) + "-"s + std::to_string(mon) + "-"s + std::to_string(mday) + " "s + std::to_string(hour) + ":"s + std::to_string(min);
}

// команда Помощь
void ToDoList::Help(std::ostream& output) {
	output <<
		"[Список доступных комманд:]\n" << line__ << '\n' <<
		"[add NAME]\t\t\t - добавить новую задачу в список, где NAME - заголовок задачи;\n"s <<
		"[done NAME]\t\t\t - отметить задачу как выполненную, где NAME - заголовок задачи;\n"s <<
		"[update NAME]\t\t\t - обновить задачу, где NAME - заголовок задачи;\n"s <<
		"[delete NAME]\t\t\t - удалить задачу из списка, где NAME - заголовок задачи;\n"s <<
		"[select *]\t\t\t - вывод всех задач\n"s <<
		"[select * where {PREDICATE}]\t - вывод задач по определённому критерию\n"s <<
		"Пример: select * where {date < \"2020-12-12 00:00\" and category=\"cat1\" and status=\"on\" and description like \"text\"}, где варианты операторов сравнения: <,<=,=,>=,<; комбинация условия доступна только через and; ключевое слово like обеспечивает фильтр по наличию заданной подстроки в поле;\n"s <<
		line__ << std::endl;
}

// команда Добавить задачу
void ToDoList::Add(ToDoPage page) {
	list_.emplace(page.name, page);
}

// команда Отметить задачу как выполненную
void ToDoList::Done(std::string_view name) {
	StatusChange(std::string(name), Status::DONE);
}

// команда Обновить задачу
void ToDoList::Update(std::string_view name, std::istream& input, std::ostream& output) {
	auto it = list_.find(std::string(name));
	if (it == list_.end()) throw error_data__;
	auto& page_link = (*it).second;
	
	output << "- Введите новое описание задачи или нажмите Enter, чтобы оставить старое описание."s << std::endl;
	std::string description_new = ReadLine(input);
	if (description_new != ""s) {
		page_link.description = description_new;
		output << "[Описание обновлено.]"s << std::endl;
	}
	else output << "[Оставлено старое описание.]"s << std::endl;

	const Date default_date;
	do {
		output << "- Введите дату и время в формате y-m-d h:m (2020-12-12 00:00) или введите time, чтобы вставить текущие дату и время. Нажмите Enter, чтобы оставить страое время."s << std::endl;
		std::string date = ReadLine(input);
		if (date == "time"s) {
			date = GetTimeNow();
			output << "[Записаны текущие дата и время - "s + date + "]"s << std::endl;
			page_link.date = std::move(date);
		}
		else if (date != ""s) {
			page_link.date = std::move(date);
			output << "[Время обновлено.]"s << std::endl;
		}
		else output << "[Оставлено старое время.]"s << std::endl;
	} while (page_link.date == default_date);

	Category category_new = Category::DEFAULT;
	while (category_new == Category::DEFAULT) {
		output << "- Введите категорию задачи или нажмите Enter, чтобы оставить старую категорию. Узнать доступные категории можно командой help_category."s << std::endl;
		std::string text = ReadLine(input);
		if (text == "help_category"s) {
			output << "[Доступные категории:]\n"s << line__ << '\n';
			for (const std::string& c : category__) output << c << '\n';
			output << line__ << std::endl;
			continue;
		}
		else if (text == ""s) {
			output << "[Оставлена старая категория.]"s << std::endl;
			break;
		}

		try {
			category_new = GetCategory(text);
			page_link.category = std::move(category_new);
			output << "[Категория обновлена.]"s << std::endl;
		}
		catch (...) {
			output << error_data__ << std::endl;
			category_new = Category::DEFAULT;
		}	
	}

	std::string status_new_text = ""s;
	while (status_new_text == ""s) {
		output << "- Введите новый статус задачи или нажмите Enter, чтобы оставить старый статус."s << std::endl;
		status_new_text = ReadLine(input);
		if (status_new_text == ""s) {
			output << "[Оставлен старый статус.]"s << std::endl;
			break;
		}

		try {
			Status status_new = GetStasus(status_new_text);
			page_link.status = status_new;
			output << "[Статус обновлен.]"s << std::endl;
		}
		catch (...) {
			output << error_data__ << std::endl;
			status_new_text = ""s;
		}
	}
}

// команда Удалить задачу
void ToDoList::Delete(std::string_view name) {
	auto it = list_.find(std::string(name));
	if (it == list_.end()) throw error_data__;
	list_.erase(it);
}

// команда Отфильтровать список задач по предикату
std::unordered_map<ToDoList::Header, ToDoPage> ToDoList::Select(std::string&& predicate) {
	std::unordered_map<Header, ToDoPage> list_sort;
	bool first_cycle = true;
	while (predicate.size() > 0 && predicate.find_first_not_of(' ') != predicate.npos) {
		size_t id_not_space = predicate.find_first_not_of(' ');
		if (id_not_space != 0) predicate.erase(0, predicate.find_first_not_of(' '));

		size_t op_quo = predicate.find('\"');
		if (op_quo < 2 || op_quo == predicate.npos) throw error_data__;
		if (predicate[op_quo - 1] == ' ') { // select * where {description like """sad"""      and description like ""123"" and description like "75"}
			auto variable = ProcessPredicateSortDynamicValue(predicate, op_quo);

			if (std::holds_alternative<SortDescription>(variable)) {
				Variable sort_description = std::move(std::get<SortDescription>(variable));
				first_cycle ? list_sort = SortByPredicate(list_, sort_description) : list_sort = SortByPredicate(list_sort, sort_description);
			}
			else if (std::holds_alternative<SortDate>(variable)) {
				Variable sort_date = std::move(std::get<SortDate>(variable));
				first_cycle ? list_sort = SortByPredicate(list_, sort_date) : list_sort = SortByPredicate(list_sort, sort_date);
			}
			else throw error_data__;
		}
		else if (predicate[op_quo - 1] == '=') { // select * where {     category="CATEGORY1"     and     status="on"     }
			auto variable = ProcessPredicateSortStaticValue(predicate, op_quo);

			if (std::holds_alternative<Category>(variable)) {
				Variable sort_category = std::move(std::get<Category>(variable));
				first_cycle ? list_sort = SortByPredicate(list_, sort_category) : list_sort = SortByPredicate(list_sort, sort_category);
			}
			else if (std::holds_alternative<Status>(variable)) {
				Variable sort_status = std::move(std::get<Status>(variable));
				first_cycle ? list_sort = SortByPredicate(list_, sort_status) : list_sort = SortByPredicate(list_sort, sort_status);
			}
			else throw error_data__;
		}
		first_cycle = false;
	}
	return list_sort;
}

// изменить статус задачи
void ToDoList::StatusChange(const Header& name, const Status& status) {
	auto it = list_.find(name);
	if (it == list_.end()) throw error_data__;
	auto& page_link = *it;
	if (page_link.second.status != status) page_link.second.status = status;
}

// фильтрация списка задач по динамическому значению предиката
std::variant<SortDescription, SortDate> ToDoList::ProcessPredicateSortDynamicValue(std::string& predicate, size_t op_quo) {
	std::variant<SortDescription, SortDate> result;
	size_t id_space_af = predicate.find_last_of(' ', op_quo - 2);
	std::string text = ""s;
	id_space_af == predicate.npos ? text = predicate.substr(0, op_quo - 1) : text = predicate.substr(id_space_af + 1, op_quo - 2 - id_space_af);
	if (text == "description"s || text == "like"s) {
		bool like = false;
		if (text == "like"s) like = true;
		result = std::move(ProcessPredicateSortDescription(like, predicate, op_quo, id_space_af));
	}
	else { // select * where {date < "2020-12-12 00:00"    and date > "2020-12-12 00:00" and  date >= "2020-12-12 00:00"}
		result = std::move(ProcessPredicateSortDate(text, predicate, op_quo, id_space_af));
	}
	return result;
}

// фильтрация списка задач по описанию
SortDescription ToDoList::ProcessPredicateSortDescription(bool like, std::string& predicate, size_t op_quo, size_t id_space_af) {
	SortDescription sort_description;
	if (like) {
		size_t id_space_bf = predicate.find_last_of(' ', id_space_af - 1);
		std::string command_description = ""s;
		id_space_bf == predicate.npos ? command_description = predicate.substr(0, id_space_af) :
			command_description = predicate.substr(id_space_bf + 1, id_space_af - 1 - id_space_bf);
		if (command_description != "description") throw error_data__;
		sort_description.description_like = true;
	}
	sort_description.description = FindDescriptonText(predicate, op_quo, predicate.size());

	size_t predicate_done_size = "description "s.size();
	if (sort_description.description_like) predicate_done_size += "like "s.size();
	predicate_done_size += sort_description.description.size() + 2;
	if (id_space_af == predicate.npos) id_space_af = 0;
	else id_space_af -= "description"s.size();
	predicate.erase(id_space_af, predicate_done_size);

	DeletePredicateAnd(predicate, id_space_af);

	return sort_description;
}

// выделение текста описания из предиката
std::string ToDoList::FindDescriptonText(const std::string& text, const size_t op_quo, size_t size_for_find) {
	size_t last_quo = text.find_last_of('\"', size_for_find);
	size_t last_and = text.rfind(" and "s, size_for_find);
	if (last_and == text.npos) return text.substr(op_quo + 1, last_quo - op_quo - 1);
	size_t new_last_quo = text.find_last_of('\"', last_and);
	size_t prev_quo = text.find_last_of('\"', new_last_quo);
	if (prev_quo == op_quo) return text.substr(op_quo + 1, new_last_quo - op_quo - 1);
	else return FindDescriptonText(text, op_quo, new_last_quo);
}

// фильтрация списка задач по дате
SortDate ToDoList::ProcessPredicateSortDate(const std::string& compare, std::string& predicate, size_t op_quo, size_t id_space_af) {
	SortDate sort_date;
	std::unordered_set<std::string> comp = { "<"s, "<="s, "="s, ">="s, ">"s }; // select * where {       date < "2020-12-12 00:00" and date > "2020-12-12 00:00"}
	size_t comp_s = comp.size();
	comp.insert(compare);
	if (comp.size() != comp_s) throw error_data__;

	size_t id_space_bf = predicate.find_last_of(' ', id_space_af - 1);
	std::string action = "";
	id_space_bf == predicate.npos ? action = predicate.substr(0, id_space_af) : action = predicate.substr(id_space_bf + 1, id_space_af - 1 - id_space_bf);
	if (action != "date") throw error_data__;
	sort_date.compare = compare;
	size_t cl_quo = predicate.find('\"', op_quo + 1);
	std::string date = predicate.substr(op_quo + 1, cl_quo - op_quo - 1);
	sort_date.date = date;

	size_t predicate_done_size = "date "s.size() + compare.size() + " "s.size() + "\""s.size() + date.size() + "\""s.size();
	if (id_space_af == predicate.npos) id_space_af = 0;
	else id_space_af -= "date"s.size();
	predicate.erase(id_space_af, predicate_done_size);

	DeletePredicateAnd(predicate, id_space_af);

	return sort_date;
}

// фильтрация списка задач по статическому значению предиката
std::variant<Category, Status> ToDoList::ProcessPredicateSortStaticValue(std::string& predicate, size_t op_quo) {
	std::variant<Category, Status> result; // select * where {     category="CATEGORY1"     and     status="on"     }
	size_t cl_quo = predicate.find('\"', op_quo + 1); // select * where {     category="CATEGORY1"     and     status="on"}
	size_t id_space_bf = predicate.find_last_of(' ', op_quo - 2); // select * where {category="CATEGORY1" and status="on"}
	std::string command = ""s;
	id_space_bf == predicate.npos ? command = predicate.substr(0, op_quo - 1) : command = predicate.substr(id_space_bf + 1, op_quo - 2 - id_space_bf);
	std::string config = predicate.substr(op_quo + 1, cl_quo - op_quo - 1);
	if (command == "category"s) result = GetCategory(config);
	else if (command == "status"s) result = GetStasus(config);
	else throw error_data__;

	size_t predicate_done_size = command.size() + "=\""s.size() + config.size() + "\""s.size();
	if (id_space_bf == predicate.npos) id_space_bf = 0;
	predicate.erase(id_space_bf, predicate_done_size);

	DeletePredicateAnd(predicate, id_space_bf);

	return result;
}

// получение значения категории
Category ToDoList::GetCategory(const std::string& config) {
	Category result;
	if (config == "CATEGORY1"s) result = Category::CATEGORY1;
	else if (config == "CATEGORY2"s) result = Category::CATEGORY2;
	else if (config == "CATEGORY3"s) result = Category::CATEGORY3;
	else throw error_data__;
	return result;
}

// получение значения статуса из предиката
Status ToDoList::GetStasus(const std::string& config) {
	Status result;
	if (config == "on"s) result = Status::PROCESS;
	else if (config == "off"s) result = Status::DONE;
	else throw error_data__;
	return result;
}

// удаление лишнего кллючего слова and в предикате
void ToDoList::DeletePredicateAnd(std::string& predicate, size_t id_space) {
	if (predicate.size() > 0 && predicate.find_first_not_of(' ') != predicate.npos) {
		size_t id_and = predicate.find(" and "s);
		if (id_and == predicate.npos) throw error_command__;
		predicate.erase(id_space, id_and + " and "s.size());
	}
}

// получение отфильтрованного списка
std::unordered_map<ToDoList::Header, ToDoPage> ToDoList::SortByPredicate(const std::unordered_map<Header, ToDoPage>& list, const Variable& comp) {
	std::unordered_map<Header, ToDoPage> result;

	if (std::holds_alternative<SortDescription>(comp)) {
		SortDescription sort_description = std::get<SortDescription>(comp);

		if (sort_description.description_like) {
			for (const auto& [header, page] : list) {
				if (page.description.find(sort_description.description) != page.description.npos) result[header] = page;
			}
		}
		else {
			for (const auto& [header, page] : list) {
				if (page.description == sort_description.description) result[header] = page;
			}
		}
	}
	else if (std::holds_alternative<SortDate>(comp)) {
		SortDate sort_date = std::get<SortDate>(comp);
		const Date default_date;
		if (sort_date.date == default_date) throw error_data__;
		
		if (sort_date.compare == "<"s) {
			for (const auto& [header, page] : list) {
				if (page.date < sort_date.date) result[header] = page;
			}
		}
		else if (sort_date.compare == "<="s) {
			for (const auto& [header, page] : list) {
				if (page.date <= sort_date.date) result[header] = page;
			}
		}
		else if (sort_date.compare == "="s) {
			for (const auto& [header, page] : list) {
				if (page.date == sort_date.date) result[header] = page;
			}
		}
		else if (sort_date.compare == ">"s) {
			for (const auto& [header, page] : list) {
				if (page.date > sort_date.date) result[header] = page;
			}
		}
		else if (sort_date.compare == ">="s) {
			for (const auto& [header, page] : list) {
				if (page.date >= sort_date.date) result[header] = page;
			}
		}
	}
	else if (std::holds_alternative<Category>(comp)) {
		Category sort_category = std::get<Category>(comp);
		
		for (const auto& [header, page] : list) {
			if (page.category == sort_category) result[header] = page;
		}
	}
	else if (std::holds_alternative<Status>(comp)) {
		Status sort_status = std::get<Status>(comp);
		
		for (const auto& [header, page] : list) {
			if (page.status == sort_status) result[header] = page;
		}
	}
	else throw error_data__;

	return result;
}

std::ostream& to_do_list::operator<<(std::ostream& out, const std::unordered_map<std::string, ToDoPage>& list) {
	for (const auto& [header, page] : list) {
		out << line__ <<
			"\nЗаголовок:\t"s << page.name <<
			"\nОписание:\t"s << page.description <<
			"\nДата:\t\t"s << page.date <<
			"\nКатегория:\t"s << page.category <<
			"\nСтатус:\t\t["s << page.status << "]\n"s;
	}
	return out << line__ << std::endl;
}