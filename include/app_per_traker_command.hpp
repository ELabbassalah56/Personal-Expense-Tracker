#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace expense_tracker {
// forward decalrations
class Expense; //
class ExpenseRepository;
class ExpenseValidator;
namespace fs = std::filesystem;

namespace models {
/**
 * @brief Represents a single expense entry
 */
class Expense {

public:
  Expense() = default;
  Expense(std::string title, double amount, std::string category,
          std::string date)
      : title_{std::move(title)}, amount_{amount},
        category_{std::move(category)} {
    if (date.empty()) {
      date_ = []() -> std::string {
        auto curr_time = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
        std::string dt = std::ctime(&curr_time);
        if (!dt.empty() && dt.back() == '\n')
          dt.pop_back();
        return dt;
      }();
    } else {
      date_ = date;
    }
  }

  // Getters
  const std::string &getTitle() const noexcept { return title_; }
  double getAmount() const noexcept { return amount_; }
  const std::string &getCategory() const noexcept { return category_; }
  const std::string &getDate() const noexcept { return date_; }

  // Setters
  void setTitle(const std::string &title) { title_ = title; }
  void setAmount(double amount) { amount_ = amount; }
  void setCategory(const std::string &category) { category_ = category; }
  void setDate(const std::string &date) {
    if (date.empty()) {
      date_ = []() -> std::string {
        auto curr_time = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
        std::string dt = std::ctime(&curr_time);
        if (!dt.empty() && dt.back() == '\n')
          dt.pop_back();
        return dt;
      }();
    } else {
      date_ = date;
    }
  }

  // Seralization
  std::string toCsv() const {
    std::ostringstream oss;
    oss << std::quoted(title_) << "," << amount_ << ","
        << std::quoted(category_) << "," << std::quoted(date_);
    return oss.str();
  }
  // read values from csv
  static std::optional<Expense> fromCsv(const std::string &line) {
    std::istringstream ss{line};
    std::string title, amountStr, category, date;
    double amount = 0.0;
    if (ss >> std::quoted(title) && ss.ignore() &&
        getline(ss, amountStr, ',') && ss >> std::quoted(category) &&
        ss.ignore() && ss >> std::quoted(date)) {
      try {
        auto removeQuotes = [](std::string &str) {
          if (str.length() >= 2 && str.front() == '"' && str.back() == '"') {
            str = str.substr(1, str.length() - 2);
          }
        };
        removeQuotes(title);
        removeQuotes(category);
        removeQuotes(date);
        amount = std::stod(amountStr);
      } catch (...) {
        return {};
      }
      return Expense(std::move(title), amount, std::move(category),
                     std::move(date));
    }
    return std::nullopt; // same as {} alies or temporory obj;
  }

  bool operator==(const Expense &other) const {
    return title_ == other.title_ && amount_ == other.amount_ &&
           category_ == other.category_ && date_ == other.date_;
  }

private:
  std::string title_;
  double amount_ = 0.0;
  std::string category_;
  std::string date_;
};
} // namespace models

namespace validator {
/**
 * @brief Validates expense data using Strategy pattern
 */
class ExpenseValidator {

public:
  enum class ValidationResult {
    SUCCESS,
    EMPTY_TITLE,
    INVALID_AMOUNT,
    EMPTY_CATEGORY,
    EMPTY_DATE
  };

  ValidationResult validate(const models::Expense &expense) const {
    if (expense.getTitle().empty()) {
      return ValidationResult::EMPTY_TITLE;
    }
    if (expense.getAmount() <= 0) {
      return ValidationResult::INVALID_AMOUNT;
    }
    if (expense.getCategory().empty()) {
      return ValidationResult::EMPTY_CATEGORY;
    }
    if (expense.getDate().empty()) {
      return ValidationResult::EMPTY_DATE;
    }

    return ValidationResult::SUCCESS;
  }

  std::string getErrorMessage(ValidationResult result) const {
    static const std::map<ValidationResult, std::string> errorMessage = {
        {ValidationResult::SUCCESS, "Valid expense"},
        {ValidationResult::EMPTY_TITLE, "Title cannot be empty"},
        {ValidationResult::INVALID_AMOUNT, "Amount must be greater than 0"},
        {ValidationResult::EMPTY_CATEGORY, "Category cannot be empty"},
        {ValidationResult::EMPTY_DATE, "Date Recorded by the current time"}};

    auto it = errorMessage.find(result);
    return (it != errorMessage.end()) ? it->second : "Unkown error";
  }
};
} // namespace validator

namespace repositories {
/**
 * @brief Handles expense storage and retrieval (Repository pattern)
 */
class ExpenseRepository {
  // Calss Interfaces
public:
  using ExpenseList = std::vector<models::Expense>;
  virtual ~ExpenseRepository() = default;

  virtual void addExpense(const models::Expense &e) = 0;
  virtual void updateExpense(size_t index, const models::Expense &e) = 0;
  virtual void removeExpense(size_t index) = 0;
  virtual const ExpenseList &getAllExpenses() const = 0;
  virtual ExpenseList
  getExpensesByCategory(const std::string &category) const = 0;
  virtual ExpenseList searchExpenses(const std::string &query) const = 0;
  virtual bool saveToFile(const std::string &filename) const = 0;
  virtual bool loadFromFile(const std::string &filename) = 0;
  virtual void clear() = 0;
  virtual size_t size() const = 0;
};

class InMemoryExpenseRepository : public ExpenseRepository {
  /**
   * @brief In-memory implementation of ExpenseRepository
   */
public:
  void addExpense(const models::Expense &e) override { expenses_.push_back(e); }
  void updateExpense(size_t index, const models::Expense &e) override {
    if (index < expenses_.size()) {
      expenses_[index] = e;
    }
  }
  void removeExpense(size_t index) override {
    if (index < expenses_.size()) {
      expenses_.erase(expenses_.begin() + index);
    }
  }
  const ExpenseList &getAllExpenses() const override { return expenses_; }
  ExpenseList
  getExpensesByCategory(const std::string &category) const override {
    ExpenseList filtered;
    std::copy_if(expenses_.begin(), expenses_.end(),
                 std::back_inserter(filtered),
                 [&category](const models::Expense &e) -> bool {
                   return e.getCategory() == category;
                 });
    return filtered;
  }
  ExpenseList searchExpenses(const std::string &query) const override {
    ExpenseList results;
    std::copy_if(expenses_.begin(), expenses_.end(),
                 std::back_inserter(results),
                 [&query](const models::Expense &e) -> bool {
                   return e.getTitle().find(query) != std::string::npos ||
                          e.getCategory().find(query) != std::string::npos;
                 });

    return results;
  }
  bool saveToFile(const std::string &filename) const override {

    if (!exists(directory_path)) {
      create_directory(directory_path);
      std::cout << "Directory created: " << directory_path << std::endl;
    }
    fs::path filepath = directory_path / filename;
    std::ofstream file(filepath);
    if (!file.is_open()) {
      std::cout << "Failed to open file for writing: " << filepath << std::endl;
      return false;
    } else {
      for (const auto &e : expenses_) {
        file << e.toCsv() << "\n";
      }
      file.close();
    }
    std::cout << "Expenses saved to " << filepath << std::endl;
    return true;
  }
  bool loadFromFile(const std::string &filename) override {
    std::string line;
    size_t lineNumber = 0;
    std::optional<models::Expense> results;
    if (!exists(directory_path)) {
      std::cout << "Directory does not exist: " << directory_path << std::endl;
      return false;
    }
    fs::path filepath = directory_path / filename;
    if (!exists(filepath)) {
      std::cout << "File does not exist: " << filepath << std::endl;
      return false;
    }
    std::ifstream file(filepath);
    if (!file.is_open()) {
      std::cout << "Failed to open file for reading: " << filepath << std::endl;
      return false;
    } else {
      clear();
      while (std::getline(file, line)) {
        ++lineNumber;

        // Trim whitespace for better validation
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) {
          std::cout << "Skipping empty line " << lineNumber << std::endl;
          continue;
        }

        auto expenseOpt = models::Expense::fromCsv(line);
        if (expenseOpt.has_value()) {
          expenses_.push_back(std::move(*expenseOpt));
          std::cout << "Line " << lineNumber
                    << ": Successfully parsed expense '"
                    << expenseOpt->getTitle() << "'" << std::endl;
        } else {
          std::cerr << "Line " << lineNumber << ": Failed to parse '" << line
                    << "'" << std::endl;

          // Optional: Show expected format
          std::cerr << "Expected format: \"title\",amount,\"category\",\"date\""
                    << std::endl;
          return false;
        }
      }
    }
    file.close();
    return true;
  }
  void clear() override { expenses_.clear(); }
  size_t size() const override { return expenses_.size(); }

private:
  ExpenseList expenses_;
  const fs::path directory_path = "./data_store";
};
} // namespace repositories

namespace services {
/**
 * @brief Business logic for expense management (Service pattern)
 */
class ExpenseService {
public:
  explicit ExpenseService(
      std::unique_ptr<repositories::ExpenseRepository> repository)
      : repository_(std::move(repository)), validator_() {}
  enum class OperationResult {
    SUCCESS,
    VALIDATION_ERROR,
    INDEX_OUT_OF_RANGE,
    FILE_ERROR
  };
  OperationResult addExpense(const std::string &title, double amount,
                             const std::string &category,
                             const std::string &date) {
    models::Expense expense{title, amount, category, date};

    auto validatorResult = validator_.validate(expense);
    if (validatorResult !=
        validator::ExpenseValidator::ValidationResult::SUCCESS) {
      lastError_ = validator_.getErrorMessage(validatorResult);
      return OperationResult::VALIDATION_ERROR;
    }
    repository_->addExpense(expense);
    return OperationResult::SUCCESS;
  }
  OperationResult updateExpense(size_t index, const std::string &title,
                                double amount, const std::string &category,
                                const std::string &date) {
    if (index >= repository_->size()) {
      lastError_ = "Index out of range!";
      return OperationResult::INDEX_OUT_OF_RANGE;
    }
    models::Expense expense{title, amount, category, date};

    auto validatorResult = validator_.validate(expense);
    if (validatorResult !=
        validator::ExpenseValidator::ValidationResult::SUCCESS) {
      lastError_ = validator_.getErrorMessage(validatorResult);
      return OperationResult::VALIDATION_ERROR;
    }

    repository_->updateExpense(index, expense);
    return OperationResult::SUCCESS;
  }
  OperationResult deleteExpense(size_t index) {
    if (index >= repository_->size()) {
      lastError_ = "Index out of range!";
      return OperationResult::INDEX_OUT_OF_RANGE;
    }
    repository_->removeExpense(index);
    return OperationResult::SUCCESS;
  }
  const repositories::ExpenseRepository::ExpenseList &getAllExpenses() const {
    return repository_->getAllExpenses();
  }
  repositories::ExpenseRepository::ExpenseList
  getExpensesByCategory(const std::string &category) const {
    return repository_->getExpensesByCategory(category);
  }
  repositories::ExpenseRepository::ExpenseList
  searchExpenses(const std::string &query) const {
    return repository_->searchExpenses(query);
  }
  double calculateTotal(const std::string &category = "") const {
    const auto &expenses = category.empty()
                               ? repository_->getAllExpenses()
                               : repository_->getExpensesByCategory(category);
    return std::accumulate(expenses.begin(), expenses.end(), 0.0,
                           [](double sum, const models::Expense &expense) {
                             return sum + expense.getAmount();
                           });
  }
  OperationResult saveToFile(const std::string &filename) {
    if (!repository_->saveToFile(filename)) {
      lastError_ = "Cannot create file!";
      return OperationResult::FILE_ERROR;
    }
    return OperationResult::SUCCESS;
  }
  OperationResult loadFromFile(const std::string &filename) {
    if (!repository_->loadFromFile(filename)) {
      lastError_ = "File not exist!";
      return OperationResult::FILE_ERROR;
    }
    return OperationResult::SUCCESS;
  }
  const std::string &getLastError() const { return lastError_; }

private:
  std::unique_ptr<repositories::ExpenseRepository> repository_;
  validator::ExpenseValidator validator_;
  std::string lastError_;
};

} // namespace services

namespace ui {
// Forward declaration
class ExpenseTrackerUI;
/**
 * @brief Command pattern for UI operations
 */
class Command {
public:
  virtual ~Command() = default;
  virtual void execute() = 0;
  virtual std::string getDescription() const = 0;
};

/**
 * @brief Concrete command implementations
 */
class AddExpenseCommand : public Command {
public:
  explicit AddExpenseCommand(ExpenseTrackerUI *ui) : ui_(ui) {}
  void execute() override;
  std::string getDescription() const override { return "Add Expense"; }

private:
  ExpenseTrackerUI *ui_;
};

class ViewExpensesCommand : public Command {
public:
  explicit ViewExpensesCommand(ExpenseTrackerUI *ui) : ui_(ui) {}
  void execute() override;
  std::string getDescription() const override { return "View All Expenses"; }

private:
  ExpenseTrackerUI *ui_;
};

class EditExpenseCommand : public Command {
public:
  explicit EditExpenseCommand(ExpenseTrackerUI *ui) : ui_(ui) {}
  void execute() override;
  std::string getDescription() const override { return "Edit Expense"; }

private:
  ExpenseTrackerUI *ui_;
};

class DeleteExpenseCommand : public Command {
public:
  explicit DeleteExpenseCommand(ExpenseTrackerUI *ui) : ui_(ui) {}
  void execute() override;
  std::string getDescription() const override { return "Delete Expense"; }

private:
  ExpenseTrackerUI *ui_;
};

class SearchExpensesCommand : public Command {
public:
  explicit SearchExpensesCommand(ExpenseTrackerUI *ui) : ui_(ui) {}
  void execute() override;
  std::string getDescription() const override { return "Search Expenses"; }

private:
  ExpenseTrackerUI *ui_;
};

class CalculateTotalCommand : public Command {
public:
  explicit CalculateTotalCommand(ExpenseTrackerUI *ui) : ui_(ui) {}
  void execute() override;
  std::string getDescription() const override { return "Calculate Total"; }

private:
  ExpenseTrackerUI *ui_;
};

class SaveToFileCommand : public Command {
public:
  explicit SaveToFileCommand(ExpenseTrackerUI *ui) : ui_(ui) {}
  void execute() override;
  std::string getDescription() const override { return "Save to File"; }

private:
  ExpenseTrackerUI *ui_;
};

class LoadFromFileCommand : public Command {
public:
  explicit LoadFromFileCommand(ExpenseTrackerUI *ui) : ui_(ui) {}
  void execute() override;
  std::string getDescription() const override { return "Load from File"; }

private:
  ExpenseTrackerUI *ui_;
};

class ExpenseTrackerUI {
public:
  explicit ExpenseTrackerUI(std::unique_ptr<services::ExpenseService> service)
      : service_(std::move(service)) {
    setupCommands();
  }

  void run() {
    while (true) {
      displayMenu();
      int choice = getUserChoice();

      if (choice == 0) {
        std::cout << "Goodbye!\n";
        break;
      }

      executeCommand(choice);
    }
  }

  // Public methods that commands can call
  void addExpenseInteractive() {
    std::string title, category, date;
    double amount;

    std::cout << "Enter title: ";
    std::getline(std::cin, title);

    std::cout << "Enter amount: ";
    std::cin >> amount;
    std::cin.ignore();

    std::cout << "Enter category: ";
    std::getline(std::cin, category);

    std::cout << "Enter date (YYYY-MM-DD): ";
    std::getline(std::cin, date);

    auto result = service_->addExpense(title, amount, category, date);
    if (result == services::ExpenseService::OperationResult::SUCCESS) {
      std::cout << "✓ Expense added successfully!\n";
    } else {
      std::cout << "✗ Error: " << service_->getLastError() << "\n";
    }
  }

  void viewExpensesInteractive() const {
    const auto &expenses = service_->getAllExpenses();

    if (expenses.empty()) {
      std::cout << "No expenses found.\n";
      return;
    }

    std::cout
        << "\n╔═══════════════════════════════════════════════════════════╗\n";
    std::cout
        << "║                      All Expenses                         ║\n";
    std::cout
        << "╚═══════════════════════════════════════════════════════════╝\n";

    for (size_t i = 0; i < expenses.size(); ++i) {
      const auto &expense = expenses[i];
      std::cout << std::setw(3) << "[" << i << "] " << std::left
                << std::setw(25) << expense.getTitle() << " $" << std::right
                << std::setw(10) << std::fixed << std::setprecision(2)
                << expense.getAmount() << "  " << std::setw(15)
                << expense.getCategory() << "  " << expense.getDate() << "\n";
    }
    std::cout << std::string(60, '-') << "\n";
  }

  void editExpenseInteractive() {
    viewExpensesInteractive();

    if (service_->getAllExpenses().empty()) {
      return;
    }

    std::cout << "\nEnter index to edit: ";
    size_t index;
    std::cin >> index;
    std::cin.ignore();

    std::string title, category, date;
    double amount;

    std::cout << "Enter new title: ";
    std::getline(std::cin, title);

    std::cout << "Enter new amount: ";
    std::cin >> amount;
    std::cin.ignore();

    std::cout << "Enter new category: ";
    std::getline(std::cin, category);

    std::cout << "Enter new date (YYYY-MM-DD): ";
    std::getline(std::cin, date);

    auto result = service_->updateExpense(index, title, amount, category, date);
    if (result == services::ExpenseService::OperationResult::SUCCESS) {
      std::cout << "✓ Expense updated successfully!\n";
    } else {
      std::cout << "✗ Error: " << service_->getLastError() << "\n";
    }
  }

  void deleteExpenseInteractive() {
    viewExpensesInteractive();

    if (service_->getAllExpenses().empty()) {
      return;
    }

    std::cout << "\nEnter index to delete: ";
    size_t index;
    std::cin >> index;
    std::cin.ignore();

    std::cout << "Are you sure? (y/n): ";
    char confirm;
    std::cin >> confirm;
    std::cin.ignore();

    if (confirm == 'y' || confirm == 'Y') {
      auto result = service_->deleteExpense(index);
      if (result == services::ExpenseService::OperationResult::SUCCESS) {
        std::cout << "✓ Expense deleted successfully!\n";
      } else {
        std::cout << "✗ Error: " << service_->getLastError() << "\n";
      }
    } else {
      std::cout << "Deletion cancelled.\n";
    }
  }

  void searchExpensesInteractive() const {
    std::cout << "Enter search query: ";
    std::string query;
    std::getline(std::cin, query);

    auto results = service_->searchExpenses(query);

    if (results.empty()) {
      std::cout << "No expenses found matching '" << query << "'.\n";
      return;
    }

    std::cout
        << "\n╔═══════════════════════════════════════════════════════════╗\n";
    std::cout
        << "║                    Search Results                         ║\n";
    std::cout
        << "╚═══════════════════════════════════════════════════════════╝\n";

    for (const auto &expense : results) {
      std::cout << std::left << std::setw(25) << expense.getTitle() << " $"
                << std::right << std::setw(10) << std::fixed
                << std::setprecision(2) << expense.getAmount() << "  "
                << std::setw(15) << expense.getCategory() << "  "
                << expense.getDate() << "\n";
    }
  }

  void calculateTotalInteractive() const {
    std::cout << "Calculate total for:\n";
    std::cout << "1. All expenses\n";
    std::cout << "2. Specific category\n";
    std::cout << "Choice: ";

    int choice;
    std::cin >> choice;
    std::cin.ignore();

    double total;
    if (choice == 2) {
      std::cout << "Enter category: ";
      std::string category;
      std::getline(std::cin, category);
      total = service_->calculateTotal(category);
      std::cout << "Total for '" << category << "': $" << std::fixed
                << std::setprecision(2) << total << "\n";
    } else {
      total = service_->calculateTotal();
      std::cout << "Total expenses: $" << std::fixed << std::setprecision(2)
                << total << "\n";
    }
  }

  void saveToFileInteractive() const {
    std::cout << "Enter filename to save (without path): ";
    std::string filename;
    std::getline(std::cin, filename);
    if (filename.empty()) {
      filename = "expenses.csv";
    }
    // Add .csv extension if not present
    if (filename.find(".csv") == std::string::npos) {
      filename += ".csv";
    }

    auto result = service_->saveToFile(filename);
    if (result == services::ExpenseService::OperationResult::SUCCESS) {
      std::cout << "✓ Expenses saved successfully from: " << filename << "\n";
    } else {
      std::cout << "✗ Error: " << service_->getLastError() << "\n";
    }
  }

  void loadFromFileInteractive() {
    std::cout << "Enter filename to load (without path): ";
    std::string filename;
    std::getline(std::cin, filename);
    if (filename.empty()) {
      filename = "expenses.csv";
    }
    // Add .csv extension if not present
    if (filename.find(".csv") == std::string::npos) {
      filename += ".csv";
    }

    auto result = service_->loadFromFile(filename);
    if (result == services::ExpenseService::OperationResult::SUCCESS) {
      std::cout << "✓ Expenses loaded successfully from: " << filename << "\n";
    } else {
      std::cout << "✗ Error: " << service_->getLastError() << "\n";
    }
  }

private:
  std::unique_ptr<services::ExpenseService> service_;
  std::map<int, std::unique_ptr<Command>> commands_;

  void setupCommands() {
    commands_[1] = std::make_unique<AddExpenseCommand>(this);
    commands_[2] = std::make_unique<ViewExpensesCommand>(this);
    commands_[3] = std::make_unique<EditExpenseCommand>(this);
    commands_[4] = std::make_unique<DeleteExpenseCommand>(this);
    commands_[5] = std::make_unique<SearchExpensesCommand>(this);
    commands_[6] = std::make_unique<CalculateTotalCommand>(this);
    commands_[7] = std::make_unique<SaveToFileCommand>(this);
    commands_[8] = std::make_unique<LoadFromFileCommand>(this);
  }

  void displayMenu() const {
    std::cout << "\n╔════════════════════════════════════╗\n";
    std::cout << "║      Expense Tracker Menu          ║\n";
    std::cout << "╚════════════════════════════════════╝\n";

    for (const auto &[key, command] : commands_) {
      std::cout << "  " << key << ". " << command->getDescription() << "\n";
    }
    std::cout << "  0. Exit\n";
    std::cout << "─────────────────────────────────────\n";
    std::cout << "Choice: ";
  }

  int getUserChoice() const {
    int choice;
    if (!(std::cin >> choice)) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      return -1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
  }

  void executeCommand(int choice) {
    auto it = commands_.find(choice);
    if (it != commands_.end()) {
      try {
        it->second->execute();
      } catch (const std::exception &e) {
        std::cerr << "Error executing command: " << e.what() << "\n";
      }
    } else {
      std::cout << "Invalid choice! Please try again.\n";
    }
  }
};

// Command execute() implementations
inline void AddExpenseCommand::execute() { ui_->addExpenseInteractive(); }

inline void ViewExpensesCommand::execute() { ui_->viewExpensesInteractive(); }

inline void EditExpenseCommand::execute() { ui_->editExpenseInteractive(); }

inline void DeleteExpenseCommand::execute() { ui_->deleteExpenseInteractive(); }

inline void SearchExpensesCommand::execute() {
  ui_->searchExpensesInteractive();
}

inline void CalculateTotalCommand::execute() {
  ui_->calculateTotalInteractive();
}

inline void SaveToFileCommand::execute() { ui_->saveToFileInteractive(); }

inline void LoadFromFileCommand::execute() { ui_->loadFromFileInteractive(); }
} // namespace ui
namespace factory {
/**
 * @brief Factory for creating application components
 */
class ExpenseTrackerFactory {
public:
  static std::unique_ptr<ui::ExpenseTrackerUI> createApplication() {
    auto repository =
        std::make_unique<repositories::InMemoryExpenseRepository>();
    auto service =
        std::make_unique<services::ExpenseService>(std::move(repository));
    return std::make_unique<ui::ExpenseTrackerUI>(std::move(service));
  }
};

} // namespace factory
} // namespace expense_tracker
