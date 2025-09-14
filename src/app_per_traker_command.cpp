#include "app_per_traker_command.hpp"
#include <chrono>
#include <ctime>

using command_tracker_T = app_per_traker_command::Expense;
using command_tracker   = app_per_traker_command::ExpensesTracker;
using app_exec          = app_per_traker_command::app_command_runner;

// Definition of createExpense (belongs to ExpensesTracker)
command_tracker_T command_tracker::createExpense(std::string&& title, double amount, std::string&& category)
{
    command_tracker_T expense; // alias to Expense
    expense.title    = title;
    expense.amount   = amount;
    expense.category = category;
    expense.date     = []()-> std::string{
        auto curr_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        return std::ctime(&curr_time);
    }();
    return expense;
}

// Free function for validation
void validateExpense(command_tracker_T& expense)
{
    if (expense.title.empty())
    {
        std::cout << "Title is required" << std::endl;
        return;
    }
    if (expense.amount <= 0)
    {
        std::cout << "Amount must be greater than 0" << std::endl;
        return;
    }
    // Add more validation rules if needed
    if(expense.category.empty())
    {
        expense.category = "Uncategorized";
        return;
    }
}
// Definition of addExpense (belongs to ExpensesTracker)
// void command_tracker::addExpense(std::vector<Expense>& expenses)
// {

// }