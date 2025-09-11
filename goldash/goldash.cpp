#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>

// --- Constants ---
const std::string CONFIG_FILENAME = "toolkit_config.dat";
const std::string LOG_FILENAME = "calculation_log.txt";
const std::string INVOICE_FILENAME = "invoices.txt";
const std::string LAB_REPORT_FILENAME = "lab_reports.txt";
const double GRAMS_PER_TOLA = 11.6638;
const int MASHA_PER_TOLA = 12;
const int RATTI_PER_MASHA = 8;
const double DENSITY_GOLD = 19.32;
const double DENSITY_COPPER = 8.96;

// --- Enums for clarity ---
enum class WeightSystem { GRAMS = 1, TOLA_MASHA_RATTI = 2 };
enum class UserRole { LAB = 1, RETAIL = 2, VIEW_LOGS = 3, SETTINGS = 4, EXIT = 5 };

// --- Helper Functions ---
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

template <typename T>
T getValidatedInput(const std::string& prompt) {
    T value;
    std::cout << prompt;
    while (!(std::cin >> value)) {
        std::cout << "Invalid input. Please enter a valid number: ";
        clearInputBuffer();
    }
    clearInputBuffer();
    return value;
}

// --- Core Data Structures ---
struct Settings {
    std::string currencySymbol = "Rs";
    WeightSystem weightSystem = WeightSystem::TOLA_MASHA_RATTI;
    double goldRate = 240000.0;
    std::string cityName = "Multan";
    bool showCityInHeader = true;
    bool askForCustomerDetails = true;

    bool save() const {
        std::ofstream configFile(CONFIG_FILENAME);
        if (!configFile) { return false; }
        configFile << currencySymbol << "\n";
        configFile << static_cast<int>(weightSystem) << "\n";
        configFile << goldRate << "\n";
        configFile << cityName << "\n";
        configFile << showCityInHeader << "\n";
        configFile << askForCustomerDetails << "\n";
        return true;
    }

    bool load() {
        std::ifstream configFile(CONFIG_FILENAME);
        if (!configFile) return false;
        try {
            std::string temp;
            std::getline(configFile, currencySymbol);
            if (std::getline(configFile, temp)) weightSystem = static_cast<WeightSystem>(std::stoi(temp));
            if (std::getline(configFile, temp)) goldRate = std::stod(temp);
            std::getline(configFile, cityName);
            if (std::getline(configFile, temp)) showCityInHeader = (temp == "1");
            if (std::getline(configFile, temp)) askForCustomerDetails = (temp == "1");
        }
        catch (const std::exception&) {
            return true;
        }
        return true;
    }
};

// --- Main Application Class ---
class App {
private:
    Settings settings;

public:
    void run() {
        if (!settings.load()) {
            runFirstTimeSetup();
        }
        while (true) {
            UserRole choice = selectRole();
            switch (choice) {
            case UserRole::LAB: runLabModule(); break;
            case UserRole::RETAIL: runRetailModule(); break;
            case UserRole::VIEW_LOGS: viewCalculationLog(); break;
            case UserRole::SETTINGS: runSettingsModule(); break;
            case UserRole::EXIT:
                std::cout << "\nThank you for using the Gold Toolkit. Goodbye!\n";
                return;
            }
        }
    }

private:
    std::string getCurrentTimestamp(bool forHeader = false) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm;

#ifdef _WIN32
        localtime_s(&local_tm, &now_time);
#else
        local_tm = *localtime(&now_time);
#endif

        std::stringstream ss;
        ss << std::put_time(&local_tm, forHeader ? "%a, %d %b %Y" : "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    void logCalculation(const std::string& entry) {
        std::ofstream logFile(LOG_FILENAME, std::ios_base::app);
        if (logFile.is_open()) {
            logFile << "[" << getCurrentTimestamp() << "] " << entry << std::endl;
        }
    }

    void printToFile(const std::string& filename, const std::string& content) {
        std::ofstream outFile(filename, std::ios_base::app);
        if (outFile.is_open()) {
            outFile << content;
        }
    }

    void displayHeader(const std::string& title) {
        clearScreen();
        std::cout << "***************************************************\n";
        std::cout << "* G O L D   &   A L L O Y   T O O L K I T         *\n";
        std::cout << "***************************************************\n";

        if (settings.showCityInHeader && !settings.cityName.empty()) {
            std::cout << "  " << settings.cityName << ", Punjab, Pakistan | " << getCurrentTimestamp(true) << "\n";
        }
        else {
            std::cout << "  " << getCurrentTimestamp(true) << "\n";
        }

        double pricePerTola = (settings.weightSystem == WeightSystem::TOLA_MASHA_RATTI)
            ? settings.goldRate
            : settings.goldRate * GRAMS_PER_TOLA;

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  Gold Rate: " << settings.currencySymbol << " " << pricePerTola << " / Tola";
        if (settings.weightSystem == WeightSystem::GRAMS) {
            std::cout << "  (" << settings.currencySymbol << " " << settings.goldRate << " / Gram)";
        }
        std::cout << "\n---------------------------------------------------\n";
        std::cout << ">> " << title << "\n";
        std::cout << "---------------------------------------------------\n\n";
    }

    void runFirstTimeSetup() {
        displayHeader("First-Time Setup");
        // ... First run setup logic ...
        settings.save();
    }

    UserRole selectRole() {
        displayHeader("Main Menu");
        std::cout << "  [1] Laboratory / Purity Testing\n";
        std::cout << "  [2] Retail Shop / Simple Invoice Calculator\n";
        std::cout << "  [3] View Calculation History\n";
        std::cout << "  [4] Settings\n";
        std::cout << "  [5] Exit\n\n";
        int choice = getValidatedInput<int>("Your choice: ");
        while (choice < 1 || choice > 5) {
            choice = getValidatedInput<int>("Invalid. Please enter 1-5: ");
        }
        return static_cast<UserRole>(choice);
    }

    void viewCalculationLog() {
        displayHeader("Internal Calculation Log");
        std::ifstream logFile(LOG_FILENAME);
        if (!logFile.is_open() || logFile.peek() == EOF) {
            std::cout << "No calculation history found.\n";
        }
        else {
            std::string line;
            while (std::getline(logFile, line)) {
                std::cout << line << std::endl;
            }
        }
        std::cout << "\nPress Enter to return...";
        std::cin.get();
    }

    void runSettingsModule() {
        displayHeader("Settings");
        std::cout << "  [1] Update Gold Rate\n";
        std::cout << "  [2] Change Weight System\n";
        std::cout << "  [3] Change Currency Symbol\n";
        std::cout << "  [4] Set City Name\n";
        std::cout << "  [5] Toggle City Name in Header (Currently: " << (settings.showCityInHeader ? "ON" : "OFF") << ")\n";
        std::cout << "  [6] Toggle Customer Details in Invoice (Currently: " << (settings.askForCustomerDetails ? "ON" : "OFF") << ")\n";
        std::cout << "  [7] Return to Main Menu\n\n";
        int choice = getValidatedInput<int>("Your choice: ");

        switch (choice) {
        case 1: { // Braces added to create scope for variable
            std::string unit = (settings.weightSystem == WeightSystem::GRAMS) ? "Gram" : "Tola";
            settings.goldRate = getValidatedInput<double>("Enter new rate for 24K Gold (per " + unit + "): ");
            break;
        }
        case 2: { // Braces added to create scope for variable
            std::cout << "\nSelect new weight system:\n  [1] Grams\n  [2] Tola / Masha / Ratti\n";
            int wsChoice = getValidatedInput<int>("Your choice: ");
            if (wsChoice == 1 || wsChoice == 2) {
                settings.weightSystem = static_cast<WeightSystem>(wsChoice);
                std::string newUnit = (settings.weightSystem == WeightSystem::GRAMS) ? "Gram" : "Tola";
                std::cout << "Weight system changed. Please update the gold rate.\n";
                settings.goldRate = getValidatedInput<double>("Enter new rate for 24K Gold (per " + newUnit + "): ");
            }
            else {
                std::cout << "Invalid choice. No changes made.\n";
            }
            break;
        }
        case 3:
            std::cout << "Enter new currency symbol: ";
            std::getline(std::cin, settings.currencySymbol);
            break;
        case 4:
            std::cout << "Enter city name (leave blank to clear): ";
            std::getline(std::cin, settings.cityName);
            break;
        case 5:
            settings.showCityInHeader = !settings.showCityInHeader;
            break;
        case 6:
            settings.askForCustomerDetails = !settings.askForCustomerDetails;
            break;
        case 7: return;
        default: std::cout << "Invalid option.\n";
        }
        settings.save();
        std::cout << "\nSettings saved successfully!\nPress Enter to return...";
        std::cin.get();
    }

    std::string formatTolaMashaRatti(double totalTola) {
        if (totalTola < 0) totalTola = 0;
        int tola = static_cast<int>(totalTola);
        double masha_decimal = (totalTola - tola) * MASHA_PER_TOLA;
        int masha = static_cast<int>(masha_decimal);
        int ratti = static_cast<int>(round((masha_decimal - masha) * RATTI_PER_MASHA));
        if (ratti >= RATTI_PER_MASHA) { masha++; ratti = 0; }
        if (masha >= MASHA_PER_TOLA) { tola++; masha = 0; }
        std::stringstream ss;
        ss << tola << " Tola, " << masha << " Masha, " << ratti << " Ratti";
        return ss.str();
    }

    double getWeightFromUser(const std::string& prompt) {
        std::cout << prompt << ":\n";
        if (settings.weightSystem == WeightSystem::GRAMS) {
            return getValidatedInput<double>("  Enter weight in Grams: ");
        }
        else {
            int tola = getValidatedInput<int>("  Enter Tola: ");
            int masha = getValidatedInput<int>("  Enter Masha: ");
            int ratti = getValidatedInput<int>("  Enter Ratti: ");
            return tola + (double)masha / MASHA_PER_TOLA + (double)ratti / (RATTI_PER_MASHA * MASHA_PER_TOLA);
        }
    }

    void runLabModule() {
        displayHeader("Laboratory / Purity Testing");

        double purityPercent = 0;
        double density = 0;
        double weightInAir = getWeightFromUser("Enter weight in AIR");
        double weightInWater = getWeightFromUser("Enter weight in WATER");

        double weightInAirGrams = (settings.weightSystem == WeightSystem::GRAMS) ? weightInAir : weightInAir * GRAMS_PER_TOLA;
        double weightInWaterGrams = (settings.weightSystem == WeightSystem::GRAMS) ? weightInWater : weightInWater * GRAMS_PER_TOLA;

        std::stringstream reportStream;
        if (weightInAirGrams <= weightInWaterGrams || weightInWaterGrams <= 0) {
            reportStream << "\n--- ERROR ---\nInvalid measurement. Weight in air must be > weight in water.\n";
        }
        else {
            double volume = weightInAirGrams - weightInWaterGrams;
            density = weightInAirGrams / volume;
            if (density >= DENSITY_COPPER && density <= DENSITY_GOLD) {
                double massFractionGold = (DENSITY_GOLD * (density - DENSITY_COPPER)) / (density * (DENSITY_GOLD - DENSITY_COPPER));
                purityPercent = massFractionGold * 100.0;
            }

            reportStream << "\n--- Analysis Report ---\n";
            reportStream << std::fixed << std::setprecision(2);
            reportStream << "Calculated Density....: " << density << " g/cm^3\n";
            if (purityPercent > 0) {
                reportStream << "Purity (Gold by mass).: " << purityPercent << " %\n";
                reportStream << "Karat Value...........: " << purityPercent * (24.0 / 100.0) << " K\n";
                reportStream << "Estimated Pure Gold...: ";
                double pureGoldGrams = weightInAirGrams * (purityPercent / 100.0);
                if (settings.weightSystem == WeightSystem::GRAMS) {
                    reportStream << pureGoldGrams << " Grams\n";
                }
                else {
                    reportStream << formatTolaMashaRatti(pureGoldGrams / GRAMS_PER_TOLA) << "\n";
                }
            }
            else {
                reportStream << "Purity could not be determined. Density is outside the expected range.\n";
            }
        }

        // --- MODIFIED: Automatic Printing ---
        std::string reportContent = reportStream.str();
        std::cout << reportContent; // Display on screen

        std::string printableReport = "--- Lab Report Printed on " + getCurrentTimestamp() + " ---\n" + reportContent + "---------------------------------------------------\n\n";
        printToFile(LAB_REPORT_FILENAME, printableReport);
        std::cout << "\nReport saved to " << LAB_REPORT_FILENAME << ".\n";

        if (purityPercent > 0) {
            std::stringstream logStream;
            logStream << "Lab Test - Density: " << density << ", Purity: " << purityPercent << "%";
            logCalculation(logStream.str());
        }

        std::cout << "\nPress Enter to return...";
        std::cin.get();
    }

    void runRetailModule() {
        displayHeader("Retail Shop / Simple Invoice Calculator");
        double itemWeight = getWeightFromUser("--- Step 1: Enter jewelry weight");
        std::cout << "\n--- Step 2: Additional Costs ---\n";
        double laborCost = getValidatedInput<double>("Enter labor charges (Mazdoori): ");
        double polishCost = getValidatedInput<double>("Enter polish/other charges: ");
        double gemstoneValue = getValidatedInput<double>("Enter value of any gemstones: ");

        std::string customerName = "";
        if (settings.askForCustomerDetails) {
            std::cout << "\n--- Step 3: Customer Details ---\n";
            std::cout << "Enter Customer Name (optional, press Enter to skip): ";
            std::getline(std::cin, customerName);
        }

        double goldValue = itemWeight * settings.goldRate;
        double totalAmount = goldValue + laborCost + polishCost + gemstoneValue;

        std::stringstream invoiceStream;
        invoiceStream << "\n--- Customer Invoice ---\n";
        if (!customerName.empty()) {
            invoiceStream << "Customer: " << customerName << "\n\n";
        }
        invoiceStream << std::fixed << std::setprecision(2);
        invoiceStream << std::left << std::setw(35) << "Gold Value (based on total weight)" << ": " << settings.currencySymbol << " " << std::right << std::setw(12) << goldValue << "\n";
        invoiceStream << std::left << std::setw(35) << "Labor Charges (Mazdoori)" << ": " << settings.currencySymbol << " " << std::right << std::setw(12) << laborCost << "\n";
        invoiceStream << std::left << std::setw(35) << "Polish & Other Charges" << ": " << settings.currencySymbol << " " << std::right << std::setw(12) << polishCost << "\n";
        invoiceStream << std::left << std::setw(35) << "Gemstone Value" << ": " << settings.currencySymbol << " " << std::right << std::setw(12) << gemstoneValue << "\n";
        invoiceStream << "---------------------------------------------------\n";
        invoiceStream << std::left << std::setw(35) << "TOTAL AMOUNT PAYABLE" << ": " << settings.currencySymbol << " " << std::right << std::setw(12) << totalAmount << "\n";
        invoiceStream << "---------------------------------------------------\n";

        // --- MODIFIED: Automatic Printing ---
        std::string invoiceContent = invoiceStream.str();
        std::cout << invoiceContent; // Display on screen

        std::string printableInvoice = "--- Invoice Printed on " + getCurrentTimestamp() + " ---\n" + invoiceContent + "\n";
        printToFile(INVOICE_FILENAME, printableInvoice);
        std::cout << "\nInvoice saved to " << INVOICE_FILENAME << ".\n";

        std::stringstream logStream;
        logStream << "Retail Invoice - Total: " << settings.currencySymbol << " " << totalAmount;
        if (!customerName.empty()) logStream << ", Customer: " << customerName;
        logCalculation(logStream.str());

        std::cout << "\nPress Enter to return...";
        std::cin.get();
    }
};

int main() {
    App toolkit;
    toolkit.run();
    return 0;
}