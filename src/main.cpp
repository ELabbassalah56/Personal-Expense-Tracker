#include <iostream>
#include "app_per_traker_command.hpp"



int main()
{
    std::cout << "Hello, Expense Tracker!" << std::endl;
    app_per_traker_command::ExpensesTracker tracker;
    std::vector<app_per_traker_command::Expense> expenses;
    tracker.addExpense(expenses);
    return 0;
}


