#include <iostream>
#include "app_per_traker_command.hpp"



/**
 * @brief Application entry point
 */
int main(int argc, char *argv[])
{
  try
  {
    std::cout << "╔════════════════════════════════════════════╗\n";
    std::cout << "║  Welcome to Expense Tracker Application    ║\n";
    std::cout << "║           Version 1.0.0                    ║\n";
    std::cout << "╚════════════════════════════════════════════╝\n\n";

    // Parse command line arguments (optional)
    std::string defaultFile = "expenses.csv";
    bool autoLoad = false;

    for (int i = 1; i < argc; ++i)
    {
      std::string arg = argv[i];

      if (arg == "--help" || arg == "-h")
      {
        std::cout << "Usage: " << argv[0] << " [options]\n\n";
        std::cout << "Options:\n";
        std::cout << "  -h, --help              Show this help message\n";
        std::cout << "  -f, --file <filename>   Specify default file to load\n";
        std::cout << "  -l, --load              Auto-load default file on startup\n";
        std::cout << "  -v, --version           Show version information\n";
        return 0;
      }
      else if (arg == "--version" || arg == "-v")
      {
        std::cout << "Expense Tracker v1.0.0\n";
        std::cout << "Built with C++17\n";
        return 0;
      }
      else if ((arg == "--file" || arg == "-f") && i + 1 < argc)
      {
        defaultFile = argv[++i];
        std::cout << "Default file set to: " << defaultFile << "\n";
      }
      else if (arg == "--load" || arg == "-l")
      {
        autoLoad = true;
        std::cout << "Auto-load enabled\n";
      }
    }

    // Create the application using factory
    auto app = expense_tracker::factory::ExpenseTrackerFactory::createApplication();

    // Auto-load expenses if requested
    if (autoLoad)
    {
      std::cout << "\nAttempting to load expenses from: " << defaultFile << "\n";
    }

    // Run the application
    std::cout << "\nStarting application...\n";
    app->run();

    std::cout << "\nThank you for using Expense Tracker!\n";
    return 0;
  }
  catch (const std::exception &e)
  {
    std::cerr << "\n╔════════════════════════════════════════════╗\n";
    std::cerr << "║           FATAL ERROR                      ║\n";
    std::cerr << "╚════════════════════════════════════════════╝\n";
    std::cerr << "Error: " << e.what() << "\n";
    std::cerr << "The application will now terminate.\n";
    return 1;
  }
  catch (...)
  {
    std::cerr << "\n╔════════════════════════════════════════════╗\n";
    std::cerr << "║        UNKNOWN FATAL ERROR                 ║\n";
    std::cerr << "╚════════════════════════════════════════════╝\n";
    std::cerr << "An unknown error occurred.\n";
    std::cerr << "The application will now terminate.\n";
    return 2;
  }
}


