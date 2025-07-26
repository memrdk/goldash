#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <vector>
#include <map>

// --- Unit Conversion Constants ---
const double GRAMS_PER_TROY_OUNCE = 31.1034768;
const double GRAMS_PER_OUNCE = 28.3495;
const double GRAMS_PER_PENNYWEIGHT = 1.55517;
const double GRAMS_PER_TOLA = 11.6638;
const double GRAMS_PER_CARAT = 0.2;

// --- File Paths ---
const std::string PRICE_FILENAME = "gold_price.dat";
const std::string LOG_FILENAME = "calculation_log.csv";
const std::string METALS_FILENAME = "metals.dat";
const std::string CONFIG_FILENAME = "toolkit_config.dat";

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

// --- Class Definitions ---

class Metal {
public:
    std::string name;
    double density; // g/cm^3
    Metal(std::string n = "", double d = 0.0) : name(n), density(d) {}
};

class Settings {
public:
    std::string currencySymbol;
    int defaultWeightUnit;

    Settings() : currencySymbol(""), defaultWeightUnit(1) {} // Default to grams

    void load() {
        std::ifstream configFile(CONFIG_FILENAME);
        if (configFile.is_open()) {
            std::string line;
            if (std::getline(configFile, line)) currencySymbol = line;
            if (std::getline(configFile, line)) defaultWeightUnit = std::stoi(line);
        }
    }

    void save() {
        std::ofstream configFile(CONFIG_FILENAME);
        if (configFile.is_open()) {
            configFile << currencySymbol << "\n";
            configFile << defaultWeightUnit << "\n";
        }
    }
};

class GoldItem {
private:
    double totalMassGrams;
    double density;
    Metal impurity;

public:
    GoldItem() : totalMassGrams(0), density(0) {}

    void setImpurity(const Metal& imp) { impurity = imp; }
    void setTotalMass(double mass) { totalMassGrams = mass; }
    void setDensity(double d) { density = d; }

    void calculateDensityFromWeight(double weightInAirGrams, double weightInWaterGrams) {
        if (weightInAirGrams > weightInWaterGrams && weightInWaterGrams > 0) {
            density = weightInAirGrams / (weightInAirGrams - weightInWaterGrams);
            totalMassGrams = weightInAirGrams;
        }
        else {
            density = 0;
        }
    }

    bool isDensityValid() const {
        if (density <= 0 || impurity.name.empty()) return false;
        double lowerBound = std::min(19.32, impurity.density);
        double upperBound = std::max(19.32, impurity.density);
        return (density >= lowerBound - 0.05 && density <= upperBound + 0.05);
    }

    double getPureGoldMass() const {
        if (!isDensityValid() || totalMassGrams <= 0) return 0.0;
        if (std::abs(density - 19.32) < 0.05) return totalMassGrams;
        double objectVolume = totalMassGrams / density;
        double volumeFractionGold = (density - impurity.density) / (19.32 - impurity.density);
        return (volumeFractionGold * objectVolume) * 19.32;
    }

    double getPurityPercentage() const {
        double pureGoldMass = getPureGoldMass();
        if (totalMassGrams <= 0 || pureGoldMass <= 0) return 0.0;
        return (pureGoldMass / totalMassGrams) * 100.0;
    }

    double getKarats() const { return getPurityPercentage() * (24.0 / 100.0); }
    double getDensity() const { return density; }
};

class App {
private:
    double goldPricePerGram;
    std::vector<Metal> metals;
    Settings settings;

public:
    App() : goldPricePerGram(0.0) {
        settings.load();
        loadMetals();
        if (metals.empty()) initializeDefaultMetals();
        loadGoldPrice();
        initializeLogFile();
    }

    void run() {
        int choice;
        do {
            displayMenu();
            std::cin >> choice;
            if (std::cin.good()) {
                handleMenuChoice(choice);
            }
            else {
                std::cout << "Invalid input. Please enter a number.\n";
                clearInputBuffer();
                choice = 0;
            }
            if (choice != 10) {
                std::cout << "\nPress Enter to return to the main menu...";
                clearInputBuffer();
                std::cin.get();
            }
        } while (choice != 10);
    }

private:
    void displayDateTime() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm;
        localtime_s(&local_tm, &now_time);
        std::cout << "  Multan, Pakistan | " << std::put_time(&local_tm, "%a, %d %b %Y, %H:%M PKT") << "\n";
    }

    void displayMenu() {
        clearScreen();
        std::cout << "***************************************************\n";
        std::cout << "* G O L D   &   A L L O Y   T O O L K I T         *\n";
        std::cout << "***************************************************\n";
        displayDateTime();
        std::cout << "---------------------------------------------------\n";
        std::cout << "  Current Gold Price: " << settings.currencySymbol << std::fixed << std::setprecision(2)
            << (goldPricePerGram > 0 ? goldPricePerGram : 0.0) << "/gram\n";
        std::cout << "---------------------------------------------------\n\n";
        std::cout << "  1. Calculate Purity (from Weight)\n";
        std::cout << "  2. Calculate Purity (from Density)\n";
        std::cout << "  3. Alloying: Create New Alloy\n";
        std::cout << "  4. Alloying: Raise Karat of Existing Alloy\n";
        std::cout << "  5. Financial: 'What-If' Investment Calculator\n";
        std::cout << "  6. View Calculation Log (CSV)\n";
        std::cout << "  7. Manage Metals\n";
        std::cout << "  8. Settings & Configuration\n";
        std::cout << "  9. Help & About\n";
        std::cout << "  10. Exit\n\n";
        std::cout << "===================================================\n";
        std::cout << "  Enter your choice: ";
    }

    void handleMenuChoice(int choice) {
        switch (choice) {
        case 1: performPurityFromWeight(); break;
        case 2: performPurityFromDensity(); break;
        case 3: performAlloyingCalculation(); break;
        case 4: performReverseAlloying(); break;
        case 5: performInvestmentCalculation(); break;
        case 6: viewCalculationLog(); break;
        case 7: manageMetals(); break;
        case 8: manageSettings(); break;
        case 9: displayHelp(); break;
        case 10:
            clearScreen();
            std::cout << "\n***************************************************\n";
            std::cout << "* Thank you for using the Toolkit! Goodbye!       *\n";
            std::cout << "***************************************************\n\n";
            break;
        default: std::cout << "Invalid choice. Please try again.\n";
        }
    }

    double getMassInGrams(const std::string& prompt, bool useDefaultUnit = false) {
        std::cout << prompt << "\n";
        int choice = settings.defaultWeightUnit;
        if (!useDefaultUnit) {
            std::cout << "  Select unit (or press Enter for default):\n  1. Grams\n  2. Troy Ounces\n  3. Ounces (AVDP)\n  4. Pennyweight (DWT)\n  5. Tola\n  Choice: ";
            if (std::cin.peek() != '\n') {
                std::cin >> choice;
            }
            clearInputBuffer();
        }

        double value;
        std::cout << "  Enter value: ";
        std::cin >> value;

        if (!std::cin.good() || value <= 0) {
            std::cout << "Invalid input.\n";
            clearInputBuffer();
            return 0.0;
        }

        switch (choice) {
        case 2: return value * GRAMS_PER_TROY_OUNCE;
        case 3: return value * GRAMS_PER_OUNCE;
        case 4: return value * GRAMS_PER_PENNYWEIGHT;
        case 5: return value * GRAMS_PER_TOLA;
        default: return value;
        }
    }

    double getStoneWeightInGrams() {
        char hasStones;
        std::cout << "\nDoes the item have gemstones/stones? (y/n): ";
        std::cin >> hasStones;
        if (hasStones == 'y' || hasStones == 'Y') {
            double stoneCarats = getValidatedNumericInput("Enter total stone weight in Carats: ");
            return stoneCarats * GRAMS_PER_CARAT;
        }
        return 0.0;
    }

    Metal chooseImpurity() { /* ... unchanged ... */ return Metal(); }

    void performPurityFromWeight() {
        clearScreen();
        std::cout << "+---------------------------------------+\n|   Purity Calculation (from Weight)    |\n+---------------------------------------+\n";
        GoldItem item;
        item.setImpurity(chooseImpurity());

        double stoneWeight = getStoneWeightInGrams();
        double weightInAir = getMassInGrams("\nEnter weight in air:") - stoneWeight;
        double weightInWater = getMassInGrams("\nEnter weight in water:") - stoneWeight;

        if (weightInAir <= 0) { std::cout << "Metal weight is zero or negative after stone deduction.\n"; return; }

        item.calculateDensityFromWeight(weightInAir, weightInWater);
        displayPurityResults(item, "PurityFromWeight");
    }

    void performPurityFromDensity() {
        clearScreen();
        std::cout << "+----------------------------------------+\n|   Purity Calculation (from Density)    |\n+----------------------------------------+\n";
        GoldItem item;
        item.setImpurity(chooseImpurity());

        double density = getValidatedNumericInput("\nEnter object's density (g/cm^3): ");
        item.setDensity(density);
        double stoneWeight = getStoneWeightInGrams();
        double mass = getMassInGrams("\nEnter total mass:") - stoneWeight;
        if (mass <= 0) { std::cout << "Metal weight is zero or negative after stone deduction.\n"; return; }
        item.setTotalMass(mass);

        displayPurityResults(item, "PurityFromDensity");
    }

    void displayPurityResults(const GoldItem& item, const std::string& calcType) { /* ... unchanged ... */ }
    void performAlloyingCalculation() { /* ... unchanged ... */ }
    void performReverseAlloying() { /* ... unchanged ... */ }

    void performInvestmentCalculation() {
        clearScreen();
        std::cout << "+-----------------------------------------------+\n|   'What-If' Investment Value Calculator       |\n+-----------------------------------------------+\n";

        double totalPureGold = 0;
        char addMore;
        do {
            std::cout << "\n--- Add Gold Holding ---\n";
            double mass = getMassInGrams("Enter mass of this holding:");
            double karat = getValidatedNumericInput("Enter Karat of this holding: ");
            if (karat > 24) karat = 24;
            totalPureGold += mass * (karat / 24.0);
            std::cout << "Add another holding? (y/n): ";
            std::cin >> addMore;
        } while (addMore == 'y' || addMore == 'Y');

        std::cout << "\n--- Projections ---\n";
        std::cout << "Total pure gold in portfolio: " << std::fixed << std::setprecision(2) << totalPureGold << " grams.\n";

        double futurePrice = getValidatedNumericInput("Enter a future target price per gram: ");
        double futureValue = totalPureGold * futurePrice;
        double currentValue = totalPureGold * goldPricePerGram;

        std::cout << "\nAt a future price of " << settings.currencySymbol << futurePrice << "/gram:\n";
        std::cout << "  -> Projected Portfolio Value: " << settings.currencySymbol << futureValue << "\n";
        if (currentValue > 0) {
            double profit = futureValue - currentValue;
            double percentageChange = (profit / currentValue) * 100.0;
            std::cout << "  -> Change from current value: " << settings.currencySymbol << profit
                << " (" << (profit > 0 ? "+" : "") << percentageChange << "%)\n";
        }
    }

    void viewCalculationLog() { /* ... unchanged ... */ }
    void manageMetals() { /* ... unchanged ... */ }

    void manageSettings() {
        clearScreen();
        std::cout << "+-----------------------------+\n|   Settings & Configuration  |\n+-----------------------------+\n";
        std::cout << "  1. Set Currency Symbol (current: \"" << settings.currencySymbol << "\")\n";
        std::cout << "  2. Set Default Weight Unit (current: " << settings.defaultWeightUnit << ")\n";
        std::cout << "  Choice: ";
        int choice;
        std::cin >> choice;
        clearInputBuffer();

        if (choice == 1) {
            std::cout << "Enter new currency symbol (e.g., Rs. or leave blank): ";
            std::getline(std::cin, settings.currencySymbol);
        }
        else if (choice == 2) {
            std::cout << "Enter new default unit (1-5): ";
            std::cin >> settings.defaultWeightUnit;
        }
        settings.save();
        std::cout << "Settings saved.\n";
    }

    void manageGoldPrice() { /* ... unchanged ... */ }
    void displayKaratInfo() { /* ... unchanged ... */ }

    void displayHelp() {
        clearScreen();
        std::cout << "+------------------------+\n|   Help & Usage Guide   |\n+------------------------+\n\n";
        std::cout << "--- Features ---\n";
        std::cout << "1-2. Purity Calculators: Determine purity from weight or density. Now supports stone weight deduction (in Carats).\n\n";
        std::cout << "3-4. Alloying Calculators: Plan how to create new alloys or improve existing ones.\n\n";
        std::cout << "5. Investment Calculator: Project the future value of your gold holdings based on different price scenarios.\n\n";
        std::cout << "--- Data & Logs ---\n";
        std::cout << "6. View Log: See all past calculations in a CSV file, good for spreadsheets.\n\n";
        std::cout << "7. Manage Metals: Add or list alloying metals. Saved in 'metals.dat'.\n\n";
        std::cout << "--- Configuration ---\n";
        std::cout << "8. Settings: Set your preferred currency symbol and default weight units.\n\n";
        std::cout << "9. Help & About:\n";
        std::cout << "   - This screen.\n";
        std::cout << "   - About: A comprehensive Gold & Alloy Toolkit. Built with C++.\n\n";
        std::cout << "10. Exit: Closes the program.\n";
    }

    double getValidatedNumericInput(const std::string& prompt) { /* ... unchanged ... */ return 0.0; }
    void initializeLogFile() { /* ... unchanged ... */ }
    void logResult(const std::string& calcType, double purity, double karat, double pureGold, double value) { /* ... unchanged ... */ }
    void saveGoldPrice() { /* ... unchanged ... */ }
    void loadGoldPrice() { /* ... unchanged ... */ }
    void saveMetals() { /* ... unchanged ... */ }
    void loadMetals() { /* ... unchanged ... */ }
    void initializeDefaultMetals() { /* ... unchanged ... */ }
};

int main() {
    App toolkit;
    toolkit.run();
    return 0;
}
