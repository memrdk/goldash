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

// --- File Paths ---
const std::string PRICE_FILENAME = "gold_price.dat";
const std::string LOG_FILENAME = "calculation_log.txt";

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

/**
 * @class Metal
 * @brief Represents a metal with its name and density.
 */
class Metal {
public:
    std::string name;
    double density; // g/cm^3

    Metal(std::string n = "", double d = 0.0) : name(n), density(d) {}
};

/**
 * @class GoldItem
 * @brief Represents a gold alloy item and performs purity calculations.
 */
class GoldItem {
private:
    double totalMassGrams;
    double density;
    Metal impurity;

public:
    GoldItem() : totalMassGrams(0), density(0) {}

    void setImpurity(const Metal& imp) {
        impurity = imp;
    }

    void setTotalMass(double mass, const std::string& unit) {
        totalMassGrams = mass;
        // The mass is already converted to grams before this function is called.
    }

    void calculateDensityFromWeight(double weightInAirGrams, double weightInWaterGrams) {
        if (weightInAirGrams > weightInWaterGrams) {
            density = weightInAirGrams / (weightInAirGrams - weightInWaterGrams);
            totalMassGrams = weightInAirGrams;
        }
        else {
            density = 0;
        }
    }

    void setDensity(double d) {
        density = d;
    }

    bool isDensityValid() const {
        if (density <= 0) return false;
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

    double getKarats() const {
        return getPurityPercentage() * (24.0 / 100.0);
    }

    double getDensity() const { return density; }
};

/**
 * @class App
 * @brief Manages the main application flow, UI, and file I/O.
 */
class App {
private:
    double goldPricePerGram;
    std::vector<Metal> metals;

public:
    App() : goldPricePerGram(0.0) {
        // Initialize standard metals
        metals.push_back(Metal("Copper", 8.96));
        metals.push_back(Metal("Silver", 10.49));
        metals.push_back(Metal("Platinum", 21.45));
        metals.push_back(Metal("Palladium", 12.02));
        loadGoldPrice();
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
            if (choice != 8) {
                std::cout << "\nPress Enter to return to the main menu...";
                clearInputBuffer();
                std::cin.get();
            }
        } while (choice != 8);
    }

private:
    void displayMenu() {
        clearScreen();
        std::cout << "================================\n";
        std::cout << "||    Gold & Alloy Toolkit    ||\n";
        std::cout << "================================\n";
        std::cout << "  Current Gold Price: $" << std::fixed << std::setprecision(2)
            << (goldPricePerGram > 0 ? goldPricePerGram : 0.0) << "/gram\n\n";
        std::cout << "1. Calculate Purity (from Weight)\n";
        std::cout << "2. Calculate Purity (from Density)\n";
        std::cout << "3. Alloying Calculator (Target Karat)\n";
        std::cout << "4. View Calculation Log\n";
        std::cout << "5. Update Gold Price\n";
        std::cout << "6. Show Karat Reference Table\n";
        std::cout << "7. Help / Usage Guide\n";
        std::cout << "8. Exit\n";
        std::cout << "================================\n";
        std::cout << "Enter your choice: ";
    }

    void handleMenuChoice(int choice) {
        switch (choice) {
        case 1: performPurityFromWeight(); break;
        case 2: performPurityFromDensity(); break;
        case 3: performAlloyingCalculation(); break;
        case 4: viewCalculationLog(); break;
        case 5: manageGoldPrice(); break;
        case 6: displayKaratInfo(); break;
        case 7: displayHelp(); break;
        case 8: std::cout << "Exiting program. Goodbye!\n"; break;
        default: std::cout << "Invalid choice. Please try again.\n";
        }
    }

    double getMassInGrams(const std::string& prompt) {
        std::cout << prompt << "\n";
        std::cout << "Select unit:\n1. Grams\n2. Troy Ounces\n3. Ounces (AVDP)\n4. Pennyweight (DWT)\nChoice: ";
        int choice;
        std::cin >> choice;
        double value;
        std::cout << "Enter value: ";
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
        default: return value;
        }
    }

    Metal chooseImpurity() {
        std::cout << "\nSelect the other metal in the alloy:\n";
        for (size_t i = 0; i < metals.size(); ++i) {
            std::cout << i + 1 << ". " << metals[i].name << "\n";
        }
        while (true) {
            std::cout << "Enter your choice (1-" << metals.size() << "): ";
            int choice;
            std::cin >> choice;
            if (std::cin.good() && choice > 0 && choice <= static_cast<int>(metals.size())) {
                return metals[choice - 1];
            }
            std::cout << "Invalid choice.\n";
            clearInputBuffer();
        }
    }

    void performPurityFromWeight() {
        clearScreen();
        std::cout << "--- Purity Calculation (from Weight) ---\n";
        GoldItem item;
        item.setImpurity(chooseImpurity());

        double weightInAir = getMassInGrams("Enter weight in air:");
        double weightInWater = getMassInGrams("Enter weight in water:");

        item.calculateDensityFromWeight(weightInAir, weightInWater);

        displayPurityResults(item);
    }

    void performPurityFromDensity() {
        clearScreen();
        std::cout << "--- Purity Calculation (from Density) ---\n";
        GoldItem item;
        item.setImpurity(chooseImpurity());

        double density = getValidatedNumericInput("Enter object's density (g/cm^3): ");
        item.setDensity(density);
        double mass = getMassInGrams("Enter total mass:");
        item.setTotalMass(mass, "grams");

        displayPurityResults(item);
    }

    void displayPurityResults(const GoldItem& item) {
        std::stringstream logStream;
        logStream << std::fixed << std::setprecision(2);
        logStream << "Calculated Density: " << item.getDensity() << " g/cm^3\n";

        if (item.isDensityValid()) {
            double pureGoldMass = item.getPureGoldMass();
            logStream << "--- Purity Analysis ---\n";
            logStream << "Purity by mass: " << item.getPurityPercentage() << "%\n";
            logStream << "Karat value: " << item.getKarats() << "K\n";
            logStream << "Total pure gold: " << pureGoldMass << " grams\n";
            if (pureGoldMass > 0) {
                calculateMarketValue(pureGoldMass, logStream);
            }
        }
        else {
            logStream << "Result: Inconclusive. Density is outside the possible range for the selected alloy.\n";
        }
        std::cout << "\n" << logStream.str();
        logResult(logStream.str());
    }

    void performAlloyingCalculation() {
        clearScreen();
        std::cout << "--- Alloying Calculator ---\n";

        double goldMass = getMassInGrams("Enter mass of PURE (24K) gold:");
        double targetKarat;
        while (true) {
            targetKarat = getValidatedNumericInput("Enter target Karat value (e.g., 18, 14): ");
            if (targetKarat < 24) break;
            std::cout << "Target Karat must be less than 24.\n";
        }
        Metal impurity = chooseImpurity();

        double targetPurity = targetKarat / 24.0;
        double impurityMass = goldMass * ((1.0 / targetPurity) - 1.0);
        double totalAlloyMass = goldMass + impurityMass;

        std::stringstream logStream;
        logStream << std::fixed << std::setprecision(2);
        logStream << "--- Alloying Results ---\n";
        logStream << "To create " << targetKarat << "K gold from " << goldMass << "g of pure gold,\n";
        logStream << "you need to add " << impurityMass << "g of " << impurity.name << ".\n";
        logStream << "Resulting total mass: " << totalAlloyMass << "g of " << targetKarat << "K alloy.\n";

        std::cout << "\n" << logStream.str();
        logResult(logStream.str());
    }

    void viewCalculationLog() {
        clearScreen();
        std::cout << "--- Calculation Log ---\n\n";
        std::ifstream logFile(LOG_FILENAME);
        if (logFile.is_open()) {
            std::cout << logFile.rdbuf();
        }
        else {
            std::cout << "Log file is empty or does not exist yet.\n";
        }
    }

    void manageGoldPrice() {
        clearScreen();
        std::cout << "--- Manage Gold Price ---\n";
        std::cout << "The currently saved price is: $" << goldPricePerGram << "/gram.\n";
        std::cout << "1. Update price per Gram\n";
        std::cout << "2. Update price per Troy Ounce\n";
        std::cout << "Enter choice: ";
        int choice;
        std::cin >> choice;
        if (!std::cin.good()) { clearInputBuffer(); return; }

        double newPrice = getValidatedNumericInput("Enter new price: ");
        if (choice == 2) {
            goldPricePerGram = newPrice / GRAMS_PER_TROY_OUNCE;
        }
        else {
            goldPricePerGram = newPrice;
        }
        saveGoldPrice();
        std::cout << "Price updated to $" << goldPricePerGram << "/gram.\n";
    }

    void displayKaratInfo() {
        clearScreen();
        std::cout << "\n--- Gold Karat Reference Table ---\n";
        // ... (content is the same)
    }

    void displayHelp() {
        clearScreen();
        std::cout << "--- Help & Usage Guide ---\n\n";
        // ... (content is the same, but can be updated to mention units)
    }

    void calculateMarketValue(double pureGoldMass, std::stringstream& logStream) {
        if (goldPricePerGram <= 0) {
            std::cout << "\nMarket price is not set. Please update it from the main menu.\n";
            return;
        }
        double totalValue = pureGoldMass * goldPricePerGram;
        logStream << "Market Value (at $" << goldPricePerGram << "/gram): $" << totalValue << "\n";
    }

    double getValidatedNumericInput(const std::string& prompt) {
        double value;
        while (true) {
            std::cout << prompt;
            std::cin >> value;
            if (std::cin.good() && value >= 0) {
                return value;
            }
            std::cout << "Invalid input. Please enter a non-negative number.\n";
            clearInputBuffer();
        }
    }

    void logResult(const std::string& logMessage) {
        std::ofstream logFile(LOG_FILENAME, std::ios_base::app);
        if (!logFile.is_open()) return;

        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        char timeStr[26];
        ctime_s(timeStr, sizeof(timeStr), &now_time);
        timeStr[24] = '\0';

        logFile << "--- Log Entry: " << timeStr << " ---\n";
        logFile << logMessage << "\n\n";
    }

    void saveGoldPrice() {
        std::ofstream priceFile(PRICE_FILENAME);
        if (priceFile.is_open()) {
            priceFile << goldPricePerGram;
        }
    }

    void loadGoldPrice() {
        std::ifstream priceFile(PRICE_FILENAME);
        if (priceFile.is_open()) {
            priceFile >> goldPricePerGram;
        }
        else {
            goldPricePerGram = 0.0;
        }
    }
};

int main() {
    App toolkit;
    toolkit.run();
    return 0;
}
