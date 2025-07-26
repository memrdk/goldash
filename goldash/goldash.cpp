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

// --- File Paths ---
const std::string PRICE_FILENAME = "gold_price.dat";
const std::string LOG_FILENAME = "calculation_log.csv";
const std::string METALS_FILENAME = "metals.dat";

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

    void setTotalMass(double mass) {
        totalMassGrams = mass;
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
        loadMetals();
        if (metals.empty()) {
            initializeDefaultMetals();
        }
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
            if (choice != 9) {
                std::cout << "\nPress Enter to return to the main menu...";
                clearInputBuffer();
                std::cin.get();
            }
        } while (choice != 9);
    }

private:
    void displayMenu() {
        clearScreen();
        std::cout << "=======================================\n";
        std::cout << "||        Gold & Alloy Toolkit       ||\n";
        std::cout << "=======================================\n";
        std::cout << "  Current Gold Price: " << std::fixed << std::setprecision(2)
            << (goldPricePerGram > 0 ? goldPricePerGram : 0.0) << "/gram\n\n";
        std::cout << "1. Calculate Purity (from Weight)\n";
        std::cout << "2. Calculate Purity (from Density)\n";
        std::cout << "3. Alloying Calculator (Create Alloy)\n";
        std::cout << "4. Alloying Calculator (Raise Karat)\n";
        std::cout << "5. View Calculation Log (CSV)\n";
        std::cout << "6. Manage Metals\n";
        std::cout << "7. Update Gold Price\n";
        std::cout << "8. Help / Usage Guide\n";
        std::cout << "9. Exit\n";
        std::cout << "=======================================\n";
        std::cout << "Enter your choice: ";
    }

    void handleMenuChoice(int choice) {
        switch (choice) {
        case 1: performPurityFromWeight(); break;
        case 2: performPurityFromDensity(); break;
        case 3: performAlloyingCalculation(); break;
        case 4: performReverseAlloying(); break;
        case 5: viewCalculationLog(); break;
        case 6: manageMetals(); break;
        case 7: manageGoldPrice(); break;
        case 8: displayHelp(); break;
        case 9: std::cout << "Exiting program. Goodbye!\n"; break;
        default: std::cout << "Invalid choice. Please try again.\n";
        }
    }

    double getMassInGrams(const std::string& prompt) {
        std::cout << prompt << "\n";
        std::cout << "Select unit:\n1. Grams\n2. Troy Ounces\n3. Ounces (AVDP)\n4. Pennyweight (DWT)\n5. Tola\nChoice: ";
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
        case 5: return value * GRAMS_PER_TOLA;
        default: return value;
        }
    }

    Metal chooseImpurity() {
        if (metals.empty()) {
            std::cout << "No metals defined. Please add a metal in the 'Manage Metals' menu.\n";
            return Metal();
        }
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

        displayPurityResults(item, "PurityFromWeight");
    }

    void performPurityFromDensity() {
        clearScreen();
        std::cout << "--- Purity Calculation (from Density) ---\n";
        GoldItem item;
        item.setImpurity(chooseImpurity());

        double density = getValidatedNumericInput("Enter object's density (g/cm^3): ");
        item.setDensity(density);
        double mass = getMassInGrams("Enter total mass:");
        item.setTotalMass(mass);

        displayPurityResults(item, "PurityFromDensity");
    }

    void displayPurityResults(const GoldItem& item, const std::string& calcType) {
        std::stringstream textOutput;
        textOutput << std::fixed << std::setprecision(2);
        textOutput << "Calculated Density: " << item.getDensity() << " g/cm^3\n";

        if (item.isDensityValid()) {
            double pureGoldMass = item.getPureGoldMass();
            double purity = item.getPurityPercentage();
            double karats = item.getKarats();
            double marketValue = (goldPricePerGram > 0) ? pureGoldMass * goldPricePerGram : 0.0;

            textOutput << "--- Purity Analysis ---\n";
            textOutput << "Purity by mass: " << purity << "%\n";
            textOutput << "Karat value: " << karats << "K\n";
            textOutput << "Total pure gold: " << pureGoldMass << " grams\n";
            if (marketValue > 0) {
                textOutput << "Market Value (at " << goldPricePerGram << "/gram): " << marketValue << "\n";
            }
            logResult(calcType, purity, karats, pureGoldMass, marketValue);
        }
        else {
            textOutput << "Result: Inconclusive. Density is outside the possible range for the selected alloy.\n";
            logResult(calcType, 0, 0, 0, 0);
        }
        std::cout << "\n" << textOutput.str();
    }

    void performAlloyingCalculation() {
        clearScreen();
        std::cout << "--- Alloying Calculator (Create Alloy) ---\n";

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

        std::stringstream textOutput;
        textOutput << std::fixed << std::setprecision(2);
        textOutput << "--- Alloying Results ---\n";
        textOutput << "To create " << targetKarat << "K gold from " << goldMass << "g of pure gold,\n";
        textOutput << "you need to add " << impurityMass << "g of " << impurity.name << ".\n";
        textOutput << "Resulting total mass: " << totalAlloyMass << "g of " << targetKarat << "K alloy.\n";

        std::cout << "\n" << textOutput.str();
        logResult("CreateAlloy", targetKarat, 0, goldMass, 0);
    }

    void performReverseAlloying() {
        clearScreen();
        std::cout << "--- Alloying Calculator (Raise Karat) ---\n";

        double initialMass = getMassInGrams("Enter mass of existing alloy:");
        double initialKarat = getValidatedNumericInput("Enter initial Karat of alloy: ");
        double targetKarat = getValidatedNumericInput("Enter target Karat to achieve: ");

        if (targetKarat <= initialKarat || targetKarat > 24 || initialKarat >= 24) {
            std::cout << "Invalid Karat values. Target must be higher than initial, and both must be below 24.\n";
            return;
        }

        double initialPurity = initialKarat / 24.0;
        double targetPurity = targetKarat / 24.0;

        // Formula: addedGold = initialMass * (targetPurity - initialPurity) / (1 - targetPurity)
        double addedGold = initialMass * (targetPurity - initialPurity) / (1.0 - targetPurity);
        double finalMass = initialMass + addedGold;

        std::stringstream textOutput;
        textOutput << std::fixed << std::setprecision(2);
        textOutput << "--- Alloying Results ---\n";
        textOutput << "To raise " << initialMass << "g of " << initialKarat << "K gold to " << targetKarat << "K,\n";
        textOutput << "you need to add " << addedGold << "g of PURE (24K) gold.\n";
        textOutput << "The final alloy will have a total mass of " << finalMass << "g.\n";

        std::cout << "\n" << textOutput.str();
        logResult("RaiseKarat", targetKarat, initialKarat, addedGold, 0);
    }

    void viewCalculationLog() {
        clearScreen();
        std::cout << "--- Calculation Log (CSV Format) ---\n\n";
        std::ifstream logFile(LOG_FILENAME);
        if (logFile.is_open()) {
            std::cout << logFile.rdbuf();
        }
        else {
            std::cout << "Log file is empty or does not exist yet.\n";
        }
    }

    void manageMetals() {
        clearScreen();
        std::cout << "--- Manage Metals ---\n";
        std::cout << "1. List Metals\n2. Add New Metal\n3. Back to Main Menu\nChoice: ";
        int choice;
        std::cin >> choice;
        if (!std::cin.good()) { clearInputBuffer(); return; }

        if (choice == 1) {
            std::cout << "\n--- Current Metals ---\n";
            for (const auto& metal : metals) {
                std::cout << metal.name << " - " << metal.density << " g/cm^3\n";
            }
        }
        else if (choice == 2) {
            std::string name;
            double density;
            std::cout << "Enter new metal name: ";
            std::cin >> name;
            density = getValidatedNumericInput("Enter density (g/cm^3): ");
            metals.push_back(Metal(name, density));
            saveMetals();
            std::cout << name << " added successfully.\n";
        }
    }

    void manageGoldPrice() {
        clearScreen();
        std::cout << "--- Manage Gold Price ---\n";
        std::cout << "The currently saved price is: " << goldPricePerGram << "/gram.\n";
        std::cout << "1. Update price per Gram\n";
        std::cout << "2. Update price per Troy Ounce\n";
        std::cout << "3. Update price per Tola\n";
        std::cout << "Enter choice: ";
        int choice;
        std::cin >> choice;
        if (!std::cin.good()) { clearInputBuffer(); return; }

        double newPrice = getValidatedNumericInput("Enter new price: ");
        if (choice == 2) {
            goldPricePerGram = newPrice / GRAMS_PER_TROY_OUNCE;
        }
        else if (choice == 3) {
            goldPricePerGram = newPrice / GRAMS_PER_TOLA;
        }
        else {
            goldPricePerGram = newPrice;
        }
        saveGoldPrice();
        std::cout << "Price updated to " << goldPricePerGram << "/gram.\n";
    }

    void displayKaratInfo() {
        clearScreen();
        std::cout << "\n--- Gold Karat Reference Table ---\n";
        std::cout << std::left << std::setw(10) << "Karat" << std::left << std::setw(15) << "Purity (%)" << "Parts of Gold\n";
        std::cout << "-------------------------------------\n";
        std::cout << std::left << std::setw(10) << "24K" << std::left << std::setw(15) << "100%" << "24/24\n";
        std::cout << std::left << std::setw(10) << "22K" << std::left << std::setw(15) << "91.7%" << "22/24\n";
        std::cout << std::left << std::setw(10) << "18K" << std::left << std::setw(15) << "75.0%" << "18/24\n";
        std::cout << std::left << std::setw(10) << "14K" << std::left << std::setw(15) << "58.3%" << "14/24\n";
        std::cout << std::left << std::setw(10) << "10K" << std::left << std::setw(15) << "41.7%" << "10/24\n";
        std::cout << "-------------------------------------\n\n";
    }

    void displayHelp() {
        clearScreen();
        std::cout << "--- Help & Usage Guide ---\n\n";
        std::cout << "This guide explains the functionality of each menu option.\n\n";

        std::cout << "1. Calculate Purity (from Weight):\n";
        std::cout << "   - Uses Archimedes' principle to find purity from weight in air and water.\n";
        std::cout << "   - You can input weight in Grams, Ounces, Troy Ounces, Pennyweight, or Tolas.\n\n";

        std::cout << "2. Calculate Purity (from Density):\n";
        std::cout << "   - Calculates purity if you already know the item's density and mass.\n\n";

        std::cout << "3. Alloying Calculator (Create Alloy):\n";
        std::cout << "   - Calculates how much of an impurity metal to add to pure gold to create a target Karat.\n\n";

        std::cout << "4. Alloying Calculator (Raise Karat):\n";
        std::cout << "   - Calculates how much pure gold to add to an existing alloy to raise it to a higher Karat.\n\n";

        std::cout << "5. View Calculation Log (CSV):\n";
        std::cout << "   - Displays the log of all past calculations, which is saved in 'calculation_log.csv'.\n\n";

        std::cout << "6. Manage Metals:\n";
        std::cout << "   - View or add new impurity metals to the list. Data is saved in 'metals.dat'.\n\n";

        std::cout << "7. Update Gold Price:\n";
        std::cout << "   - Set the market price of gold using price per Gram, Troy Ounce, or Tola.\n\n";

        std::cout << "8. Help / Usage Guide:\n";
        std::cout << "   - Displays this help screen.\n\n";

        std::cout << "9. Exit:\n";
        std::cout << "   - Closes the program.\n";
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

    void initializeLogFile() {
        std::ifstream logFile(LOG_FILENAME);
        if (!logFile.good()) {
            std::ofstream newLogFile(LOG_FILENAME);
            newLogFile << "Timestamp,CalculationType,Purity(%),Karat,PureGold(g),MarketValue\n";
        }
    }

    void logResult(const std::string& calcType, double purity, double karat, double pureGold, double value) {
        std::ofstream logFile(LOG_FILENAME, std::ios_base::app);
        if (!logFile.is_open()) return;

        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm gmtm;
        gmtime_s(&gmtm, &now_time);
        std::stringstream ss;
        ss << std::put_time(&gmtm, "%Y-%m-%d %H:%M:%S");

        logFile << ss.str() << "," << calcType << "," << purity << "," << karat << "," << pureGold << "," << value << "\n";
    }

    void saveGoldPrice() {
        std::ofstream priceFile(PRICE_FILENAME);
        if (priceFile.is_open()) priceFile << goldPricePerGram;
    }

    void loadGoldPrice() {
        std::ifstream priceFile(PRICE_FILENAME);
        if (priceFile.is_open()) priceFile >> goldPricePerGram;
        else goldPricePerGram = 0.0;
    }

    void saveMetals() {
        std::ofstream metalFile(METALS_FILENAME);
        if (metalFile.is_open()) {
            for (const auto& metal : metals) {
                metalFile << metal.name << " " << metal.density << "\n";
            }
        }
    }

    void loadMetals() {
        std::ifstream metalFile(METALS_FILENAME);
        if (metalFile.is_open()) {
            std::string name;
            double density;
            while (metalFile >> name >> density) {
                metals.push_back(Metal(name, density));
            }
        }
    }

    void initializeDefaultMetals() {
        metals.push_back(Metal("Copper", 8.96));
        metals.push_back(Metal("Silver", 10.49));
        metals.push_back(Metal("Platinum", 21.45));
        metals.push_back(Metal("Palladium", 12.02));
        saveMetals();
    }
};

int main() {
    App toolkit;
    toolkit.run();
    return 0;
}
