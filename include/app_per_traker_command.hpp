#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <cstdlib>


namespace app_per_traker_command 
{   
    // Forward declaration of Expense struct
    struct Expense;

    class ExpensesTracker
    {

        public:
        ExpensesTracker()=default;
        ~ExpensesTracker()=default;
        ExpensesTracker(const ExpensesTracker& other)=default;
        ExpensesTracker& operator=(const ExpensesTracker& other)=default;
        ExpensesTracker(ExpensesTracker&& other) noexcept=default;
        ExpensesTracker& operator=(ExpensesTracker&& other) noexcept=default;
        /**
        * @brief Adds a new expense to the expenses list by prompting user input.
        *
        * This function requests the user to input the expense details: title, amount,
        * date, and an optional category. It then adds the constructed expense to the provided list.
        *
        * @param[out] expenses Vector of expenses where the new expense will be added.
        */
        void addExpense(std::vector<Expense>& expenses);

        /**
        * @brief Displays all recorded expenses to the user.
        *
        * Lists the expenses in a readable format including title, amount, date, and category.
        * If no expenses are present, a message indicating this will be shown.
        *
        * @param[in] expenses Vector of expenses to display.
        */
        void viewExpenses(const std::vector<Expense>& expenses, const std::string& filterCategory = "", const std::string& filterDate = "") const ;

        /**
        * @brief Allows users to edit an existing expense by providing a new title, amount, date, and category.
        *
        * This function prompts the user to enter the index of the expense they want to edit.
        * It then allows the user to input new values for the expense's title, amount, date, and category.
        *
        * @param[in,out] expenses Vector of expenses where the expense will be edited.
        * @param[in] index Index of the expense to edit.
        */
        void editExpense(std::vector<Expense>& expenses, int index);

        /**
        * @brief Deletes an expense from the expenses list by providing its index.
        *
        * This function prompts the user to enter the index of the expense they want to delete.
        * It then removes the expense from the provided list.
        *
        * @param[in,out] expenses Vector of expenses where the expense will be deleted.
        * @param[in] index Index of the expense to delete.
        */
        void deleteExpense(std::vector<Expense>& expenses, int index);

        /**
        * @brief Calculates the total amount of expenses in the provided list.
        *
        * This function iterates through the provided list of expenses and sums up the amounts of all expenses.
        *
        * @param[in] expenses Vector of expenses to calculate the total for.
        * @param[in] filterCategory Optional category to filter expenses by.
        */
        double calculateTotal(const std::vector<Expense>& expenses, const std::string_view& filterCategory = "")const;

        /**
        * @brief Searches for expenses matching a given query in the provided list.
        *
        * This function iterates through the provided list of expenses and searches for expenses whose title or category contains the given query.
        *
        * @param[in] expenses Vector of expenses to search through.
        * @param[in] query The search query to match against expense titles and categories.
        */
        void searchExpenses(const std::vector<Expense>& expenses, const std::string& query)const;

        /**
        * @brief Saves the expenses to a file.
        *
        * This function writes the expenses to a file in a formatted way.
        *
        * @param[in] expenses Vector of expenses to save.
        * @param[in] filename The name of the file to save the expenses to.
        */
        bool saveExpenses(const std::vector<Expense>& expenses, const std::string& filename)const;

        /**
        * @brief Loads the expenses from a file.
        *
        * This function reads the expenses from a file and adds them to the provided list.
        *
        * @param[in,out] expenses Vector of expenses where the loaded expenses will be added.
        * @param[in] filename The name of the file to load the expenses from.
        */
        void loadExpenses(std::vector<Expense>& expenses, const std::string& filename);


        private:
        Expense createExpense(std::string&& title, double amount, std::string&& category);
        void validateExpense(const Expense& expense) const;
    };
    class app_command_runner
    {
        public:
        app_command_runner()=default;
        ~app_command_runner()= default;
        app_command_runner(const app_command_runner& other)=default;
        app_command_runner& operator=(const app_command_runner& other)=default;
        app_command_runner(app_command_runner&& other) noexcept=default;
        app_command_runner& operator=(app_command_runner&& other) noexcept=default;

        void run();

        private:
        ExpensesTracker tracker;
    };
    
    // 💡 typedef is redundant in modern C++ (struct already creates a type alias).
    // You can just write: struct Expense { ... }; without typedef.
    struct Expense
    {
        std::string title;
        double amount;
        std::string category;
        std::string date;
    };
}