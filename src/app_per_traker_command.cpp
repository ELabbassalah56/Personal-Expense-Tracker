#include "app_per_traker_command.hpp"
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>

using namespace std::filesystem;
using command_tracker_T = app_per_traker_command::Expense;
using command_tracker = app_per_traker_command::ExpensesTracker;
using app_exec = app_per_traker_command::app_command_runner;

// Definition of createExpense (belongs to ExpensesTracker)
command_tracker_T command_tracker::createExpense(std::string &&title,
                                                 double amount,
                                                 std::string &&category,
                                                 std::string &&date) {
  command_tracker_T expense; // alias to Expense
  expense.title = title;
  expense.amount = amount;
  expense.category = category;
  if (date != "")
    expense.date = date;
  else
    expense.date = []() -> std::string {
      auto curr_time = std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now());
      std::string dt = std::ctime(&curr_time);
      if (!dt.empty() && dt.back() == '\n')
        dt.pop_back();

      return dt;
    }();
  return expense;
}

// Free function for validation
bool command_tracker::validateExpense(command_tracker_T &expense) {
  if (expense.title.empty()) {
    std::cout << "Title is required" << std::endl;
    return false;
  }
  if (expense.amount <= 0) {
    std::cout << "Amount must be greater than 0" << std::endl;
    return false;
  }
  if (expense.date.empty()) {
    std::cout << "Date is required" << std::endl;
  }
  // Add more validation rules if needed
  if (expense.category.empty()) {
    expense.category = "Uncategorized";
  }
  return true;
}
// Definition of addExpense (belongs to ExpensesTracker)
void command_tracker::addExpense(std::vector<command_tracker_T> &expenses) {
  command_tracker_T expense;
  std::cout << "Enter expense title: ";
  std::getline(std::cin, expense.title);
  std::cout << "Enter expense amount: ";
  std::cin >> expense.amount;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  std::cout << "Enter expense Date (optional): ";
  std::getline(std::cin, expense.date);
  std::cout << "Enter expense Category (optional): ";
  std::getline(std::cin, expense.category);
  if (validateExpense(expense)) {
    expenses.push_back(createExpense(std::move(expense.title), expense.amount,
                                     std::move(expense.category),
                                     std::move(expense.date)));
    std::cout << "Expense added successfully!" << std::endl;
  } else {
    std::cout << "Failed to add expense due to validation errors." << std::endl;
  }
}
// Definition of viewExpenses (belongs to ExpensesTracker)
void command_tracker::viewExpenses(const std::vector<Expense> &expenses,
                                   const std::string &filterCategory = "",
                                   const std::string &filterDate = "") const {
  if (expenses.empty()) {
    std::cout << "No expenses recorded." << std::endl;
    return;
  }
  int i = 0;
  if (filterCategory.empty() && filterDate.empty()) {

    for (auto e : expenses) {
      std::cout << i++ << ". " << e.title << " | Amount: " << e.amount
                << " | Date: " << e.date << " | Category: " << e.category
                << std::endl;
    }
  } else {
    for (auto e : expenses) {
      if (e.category == filterCategory || e.date == filterDate) {
        std::cout << i++ << ". " << e.title << " | Amount: " << e.amount
                  << " | Date: " << e.date << " | Category: " << e.category
                  << std::endl;
      }
    }
  }
}
// Definition of editExpense (belongs to ExpensesTracker)
void command_tracker::editExpense(std::vector<command_tracker_T> &expenses,
                                  int index) {
  if (index < 0 || index >= static_cast<int>(expenses.size())) {
    std::cout << "Invalid index." << std::endl;
    return;
  }
  command_tracker_T &oldExpenses = expenses[index];
  command_tracker_T newExpenses; // Reference to the expense to be edited
  std::cout << "Editing expense: " << oldExpenses.title
            << " | Amount: " << oldExpenses.amount
            << " | Date: " << oldExpenses.date
            << " | Category: " << oldExpenses.category << std::endl;
  std::cout << "Enter new title (leave blank to keep current): ";
  std::getline(std::cin, newExpenses.title);
  std::cout << "Enter new amount (leave blank to keep current): ";
  std::cin >> newExpenses.amount;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  std::cout << "Enter new Date (leave blank to keep current): ";
  std::getline(std::cin, newExpenses.date);
  std::cout << "Enter new Category (leave blank to keep current): ";
  std::getline(std::cin, newExpenses.category);
  // Update fields if new values are provided
  if (validateExpense(newExpenses)) {
    oldExpenses = createExpense(
        std::move(newExpenses.title), newExpenses.amount,
        std::move(newExpenses.category), std::move(newExpenses.date));
    std::cout << "Expense updated successfully!" << std::endl;
  } else {
    std::cout << "Failed to update expense due to validation errors."
              << std::endl;
  }
}
// Definition of deleteExpense (belongs to ExpensesTracker)
void command_tracker::deleteExpense(std::vector<command_tracker_T> &expenses,
                                    int index) {
  if (index < 0 || index >= static_cast<int>(expenses.size())) {
    std::cout << "Invalid index." << std::endl;
    return;
  }
  expenses.erase(expenses.begin() + index);
  std::cout << "Expense deleted successfully!" << std::endl;
}
// Definition of calculateTotal (belongs to ExpensesTracker)
double command_tracker::calculateTotal(
    const std::vector<command_tracker_T> &expenses,
    const std::string_view &filterCategory = "") const {
  double total = 0;
  for (auto e : expenses) {
    if (filterCategory.empty() || e.category == filterCategory) {
      total += e.amount;
    }
  }
  return total;
}
// Defenition of searchExpenses (belongs to ExpensesTracker)
void command_tracker::searchExpenses(
    const std::vector<command_tracker_T> &expenses,
    const std::string &query) const {
  if (expenses.empty()) {
    std::cout << "No expenses recorded." << std::endl;
    return;
  }
  int i = 0;
  for (auto e : expenses) {
    if (e.title.find(query) != std::string::npos ||
        e.category.find(query) != std::string::npos) {
      std::cout << i++ << ". " << e.title << " | Amount: " << e.amount
                << " | Date: " << e.date << " | Category: " << e.category
                << std::endl;
    }
  }
}
// Defenition of saveExpenses (belongs to ExpensesTracker)
bool command_tracker::saveExpenses(
    const std::vector<command_tracker_T> &expenses,
    const std::string &filename) const 
{
    path directorypath = "./data_store";
    if (!exists(directorypath)) {
        create_directory(directorypath);
        std::cout << "Directory created: " << directorypath
             << std::endl;
    }
    path filepath = directorypath / filename;
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cout << "Failed to open file for writing: " << filepath
             << std::endl;
        return false;  
    }
    else {
        for (const auto &e : expenses) {
            file << e.title << "," << e.amount << "," << e.date << ","
                 << e.category << "\n";
        }
        file.close();
        std::cout << "Expenses saved to " << filepath << std::endl;
        return true;  
    }   
}
// Defenition of loadExpenses (belongs to ExpensesTracker)
void command_tracker::loadExpenses(std::vector<command_tracker_T> &expenses,
                    const std::string &filename)
                    {
                        

                    }