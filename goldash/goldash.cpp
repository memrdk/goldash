#include <iostream>
#include <iomanip>      // For std::setprecision
#include <limits>       // For std::numeric_limits
#include <string>       // For std::string
#include <cmath>        // For std::abs
#include <algorithm>    // For std::min and std::max
#include <fstream>      // For file input/output (ifstream, ofstream)
#include <sstream>      // For std::stringstream
#include <cstdlib>      // For system("cls") and system("clear")
#include <chrono>       // For timestamping logs
#include <ctime>        // For timestamping logs

// --- Constants ---
// Densities are in grams per cubic centimeter (g/cm^3)
const double DENSITY_WATER = 1.0;
const double DENSITY_PURE_GOLD = 19.32;
const double DENSITY_COPPER = 8.96;
const double DENSITY_SILVER = 10.49;
const double DENSITY_PLATINUM = 21.45;
const double DENSITY_PALLADIUM = 12.02;

// --- Global Variables & File Paths ---
const std::string PRICE_FILENAME = "gold_price.dat";
const std::string LOG_FILENAME = "calculation_log.txt";
double g_currentGoldPrice = 0.0; // Global variable to hold the current price

// --- Forward declarations of functions ---
void clearScreen();
void clearInputBuffer();
double getValidatedNumericInput(const std::string& prompt);
void displayKaratInfo();
void displayHelp();
double chooseImpurity(std::string& impurityName);
void logResult(const std::string& logMessage);
void saveGoldPriceToFile(double price);
double loadGoldPriceFromFile();
void manageGoldPrice();
void calculateMarketValue(double pureGoldMass, std::stringstream& logStream);
void performPurityCalculation();
void performDensityToPurityConversion();
void performAlloyingCalculation();

/**
 * @brief Clears the console screen (cross-platform).
 */
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/**
 * @brief Clears the input buffer to handle invalid user input.
 */
void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

/**
 * @brief Prompts the user for a numeric value and validates it.
 */
double getValidatedNumericInput(const std::string& prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.good() && value > 0) {
            return value;
        }
        else {
            std::cout << "Invalid input. Please enter a positive number." << std::endl;
            clearInputBuffer();
        }
    }
}

/**
 * @brief Displays a reference table for gold Karat values.
 */
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

/**
 * @brief Displays a detailed help and usage guide for the user.
 */
void displayHelp() {
    clearScreen();
    std::cout << "--- Help & Usage Guide ---\n\n";
    std::cout << "This guide explains the functionality of each menu option.\n\n";

    std::cout << "1. Calculate Purity (from Weight):\n";
    std::cout << "   - Use this if you have a gold item and can weigh it in both air and water.\n";
    std::cout << "   - The program uses Archimedes' principle to find the item's density.\n";
    std::cout << "   - It then calculates the gold purity, Karat value, and pure gold mass, assuming it's an alloy with another metal you select.\n\n";

    std::cout << "2. Calculate Purity (from Density):\n";
    std::cout << "   - Use this if you already know the density and total mass of your item.\n";
    std::cout << "   - It directly calculates the purity without needing weight-in-water measurements.\n\n";

    std::cout << "3. Alloying Calculator (Target Karat):\n";
    std::cout << "   - A tool for jewelers or hobbyists.\n";
    std::cout << "   - Enter the mass of pure (24K) gold you have.\n";
    std::cout << "   - The program calculates how much of another metal (e.g., copper) you need to add to achieve a desired Karat (e.g., 18K).\n\n";

    std::cout << "4. Show Karat Reference Table:\n";
    std::cout << "   - Displays a quick reference table showing the gold percentage for common Karat values (24K, 22K, 18K, etc.).\n\n";

    std::cout << "5. Update Gold Price:\n";
    std::cout << "   - View and update the current market price of gold per gram.\n";
    std::cout << "   - This price is saved and used for all market value calculations.\n\n";

    std::cout << "6. Exit:\n";
    std::cout << "   - Closes the program.\n\n";

    std::cout << "FILES USED BY THIS PROGRAM:\n";
    std::cout << " - " << PRICE_FILENAME << ": Stores the current gold price so you don't have to re-enter it.\n";
    std::cout << " - " << LOG_FILENAME << ": Stores a timestamped log of all your calculations for your records.\n";
}

/**
 * @brief Prompts the user to choose an impurity metal and returns its density.
 */
double chooseImpurity(std::string& impurityName) {
    int choice;
    std::cout << "\nSelect the other metal in the alloy:\n";
    std::cout << "1. Copper (Most common)\n2. Silver\n3. Platinum\n4. Palladium\n";
    while (true) {
        std::cout << "Enter your choice (1-4): ";
        std::cin >> choice;
        if (std::cin.good()) {
            switch (choice) {
            case 1: impurityName = "Copper"; return DENSITY_COPPER;
            case 2: impurityName = "Silver"; return DENSITY_SILVER;
            case 3: impurityName = "Platinum"; return DENSITY_PLATINUM;
            case 4: impurityName = "Palladium"; return DENSITY_PALLADIUM;
            default: std::cout << "Invalid choice. Please select from 1-4.\n";
            }
        }
        else {
            std::cout << "Invalid input. Please enter a number.\n";
            clearInputBuffer();
        }
    }
}

/**
 * @brief Appends a timestamped message to the log file.
 */
void logResult(const std::string& logMessage) {
    std::ofstream logFile(LOG_FILENAME, std::ios_base::app);
    if (!logFile.is_open()) {
        std::cerr << "Error: Could not open log file for writing." << std::endl;
        return;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char timeStr[26];
    ctime_s(timeStr, sizeof(timeStr), &now_time);
    timeStr[24] = '\0'; // Remove newline character

    logFile << "--- Log Entry: " << timeStr << " ---\n";
    logFile << logMessage << "\n\n";
    logFile.close();
}

/**
 * @brief Saves the provided gold price to the data file.
 */
void saveGoldPriceToFile(double price) {
    std::ofstream priceFile(PRICE_FILENAME);
    if (priceFile.is_open()) {
        priceFile << price;
        priceFile.close();
    }
    else {
        std::cerr << "Error: Could not open price file for writing." << std::endl;
    }
}

/**
 * @brief Loads the gold price from the data file.
 * @return The price from the file, or 0.0 if not found.
 */
double loadGoldPriceFromFile() {
    std::ifstream priceFile(PRICE_FILENAME);
    double price = 0.0;
    if (priceFile.is_open()) {
        priceFile >> price;
        priceFile.close();
    }
    return price > 0 ? price : 0.0;
}

/**
 * @brief Manages viewing and updating the global gold price.
 */
void manageGoldPrice() {
    clearScreen();
    std::cout << "--- Manage Gold Price ---\n";
    std::cout << "The currently saved price is: " << (g_currentGoldPrice > 0 ? std::to_string(g_currentGoldPrice) : "Not Set") << " per gram.\n";
    std::cout << "Enter a new price to update it, or enter 0 to return to the menu.\n";
    double newPrice;
    std::cout << "New price: ";
    std::cin >> newPrice;

    if (std::cin.good()) {
        if (newPrice > 0) {
            g_currentGoldPrice = newPrice;
            saveGoldPriceToFile(g_currentGoldPrice);
            std::cout << "Gold price updated to " << g_currentGoldPrice << " per gram.\n";
        }
    }
    else {
        std::cout << "Invalid input.\n";
        clearInputBuffer();
    }
}

/**
 * @brief Calculates market value and appends info to the log stream.
 */
void calculateMarketValue(double pureGoldMass, std::stringstream& logStream) {
    if (pureGoldMass <= 0) return;

    if (g_currentGoldPrice <= 0) {
        std::cout << "\n--- Market Value Calculation ---\n";
        std::cout << "Market price is not set. Please enter it now.\n";
        g_currentGoldPrice = getValidatedNumericInput("Enter the current market price of gold per gram: ");
        saveGoldPriceToFile(g_currentGoldPrice);
    }

    double totalValue = pureGoldMass * g_currentGoldPrice;
    logStream << "Market Value (at " << g_currentGoldPrice << " per gram): " << totalValue << "\n";
}

/**
 * @brief Main logic for calculating gold purity from weight.
 */
void performPurityCalculation() {
    clearScreen();
    std::stringstream resultStream;
    std::string impurityName;
    double densityOfImpurity = chooseImpurity(impurityName);

    std::cout << "\n--- Purity Calculation (from Weight) ---\n";
    resultStream << "Calculation Type: Purity from Weight\n";
    resultStream << "Assumed Impurity: " << impurityName << "\n";

    double weightInAir = getValidatedNumericInput("Enter weight in air (grams): ");
    double weightInWater;
    while (true) {
        weightInWater = getValidatedNumericInput("Enter weight in water (grams): ");
        if (weightInWater < weightInAir) break;
        else std::cout << "Error: Weight in water must be less than weight in air.\n";
    }

    resultStream << "Input - Weight in Air: " << weightInAir << "g\n";
    resultStream << "Input - Weight in Water: " << weightInWater << "g\n";

    double objectDensity = weightInAir / (weightInAir - weightInWater);
    double objectVolume = weightInAir / objectDensity;

    resultStream << std::fixed << std::setprecision(2);
    resultStream << "Calculated Density: " << objectDensity << " g/cm^3\n";

    double massOfPureGold = 0.0, purityPercentage = 0.0, karats = 0.0;
    double lowerBound = std::min(DENSITY_PURE_GOLD, densityOfImpurity);
    double upperBound = std::max(DENSITY_PURE_GOLD, densityOfImpurity);

    if (objectDensity < lowerBound - 0.05 || objectDensity > upperBound + 0.05) {
        resultStream << "Result: Inconclusive. Density is outside the possible range for a Gold-" << impurityName << " alloy.\n";
    }
    else {
        if (std::abs(objectDensity - DENSITY_PURE_GOLD) < 0.05) {
            massOfPureGold = weightInAir;
            purityPercentage = 100.0;
            karats = 24.0;
        }
        else {
            double volFracGold = (objectDensity - densityOfImpurity) / (DENSITY_PURE_GOLD - densityOfImpurity);
            massOfPureGold = (volFracGold * objectVolume) * DENSITY_PURE_GOLD;
            purityPercentage = (massOfPureGold / weightInAir) * 100.0;
            karats = purityPercentage * (24.0 / 100.0);
        }
        resultStream << "--- Purity Analysis ---\n";
        resultStream << "Purity by mass: " << purityPercentage << "%\n";
        resultStream << "Karat value: " << karats << "K\n";
        resultStream << "Total pure gold: " << massOfPureGold << " grams\n";
        calculateMarketValue(massOfPureGold, resultStream);
    }

    std::cout << "\n" << resultStream.str();
    logResult(resultStream.str());
}

/**
 * @brief Calculates purity from a known density and total weight.
 */
void performDensityToPurityConversion() {
    clearScreen();
    std::stringstream resultStream;
    std::string impurityName;
    std::cout << "\n--- Purity Calculation (from Density) ---\n";
    resultStream << "Calculation Type: Purity from Density\n";

    double objectDensity = getValidatedNumericInput("Enter the object's density (g/cm^3): ");
    double totalMass = getValidatedNumericInput("Enter the object's total mass (grams): ");
    double densityOfImpurity = chooseImpurity(impurityName);

    resultStream << "Input - Object Density: " << objectDensity << " g/cm^3\n";
    resultStream << "Input - Total Mass: " << totalMass << "g\n";
    resultStream << "Assumed Impurity: " << impurityName << "\n";
    resultStream << std::fixed << std::setprecision(2);

    double massOfPureGold = 0.0, purityPercentage = 0.0, karats = 0.0;
    double lowerBound = std::min(DENSITY_PURE_GOLD, densityOfImpurity);
    double upperBound = std::max(DENSITY_PURE_GOLD, densityOfImpurity);

    if (objectDensity < lowerBound - 0.05 || objectDensity > upperBound + 0.05) {
        resultStream << "Result: Inconclusive. Density is outside the possible range.\n";
    }
    else {
        if (std::abs(objectDensity - DENSITY_PURE_GOLD) < 0.05) {
            purityPercentage = 100.0; massOfPureGold = totalMass; karats = 24.0;
        }
        else {
            double massFraction = ((1.0 / objectDensity) - (1.0 / densityOfImpurity)) / ((1.0 / DENSITY_PURE_GOLD) - (1.0 / densityOfImpurity));
            purityPercentage = massFraction * 100.0;
            karats = purityPercentage * (24.0 / 100.0);
            massOfPureGold = totalMass * massFraction;
        }
        resultStream << "--- Purity Analysis ---\n";
        resultStream << "Purity by mass: " << purityPercentage << "%\n";
        resultStream << "Karat value: " << karats << "K\n";
        resultStream << "Total pure gold: " << massOfPureGold << " grams\n";
        calculateMarketValue(massOfPureGold, resultStream);
    }

    std::cout << "\n" << resultStream.str();
    logResult(resultStream.str());
}

/**
 * @brief Calculates the amount of impurity needed to create a target Karat alloy.
 */
void performAlloyingCalculation() {
    clearScreen();
    std::stringstream resultStream;
    std::string impurityName;
    std::cout << "\n--- Alloying Calculator ---\n";
    resultStream << "Calculation Type: Alloying Calculator\n";

    double goldMass = getValidatedNumericInput("Enter mass of PURE (24K) gold (grams): ");
    double targetKarat;
    while (true) {
        targetKarat = getValidatedNumericInput("Enter target Karat value (e.g., 18, 14): ");
        if (targetKarat < 24) break;
        std::cout << "Target Karat must be less than 24.\n";
    }
    double densityOfImpurity = chooseImpurity(impurityName);

    resultStream << "Input - Pure Gold Mass: " << goldMass << "g\n";
    resultStream << "Input - Target Karat: " << targetKarat << "K\n";
    resultStream << "Alloying Metal: " << impurityName << "\n";

    double targetPurity = targetKarat / 24.0;
    double impurityMass = goldMass * ((1.0 / targetPurity) - 1.0);
    double totalAlloyMass = goldMass + impurityMass;

    resultStream << std::fixed << std::setprecision(2);
    resultStream << "--- Alloying Results ---\n";
    resultStream << "To create " << targetKarat << "K gold, add " << impurityMass << "g of " << impurityName << ".\n";
    resultStream << "Resulting total mass: " << totalAlloyMass << "g of " << targetKarat << "K alloy.\n";

    std::cout << "\n" << resultStream.str();
    logResult(resultStream.str());
}

int main() {
    g_currentGoldPrice = loadGoldPriceFromFile();
    int choice;

    do {
        clearScreen();
        std::cout << "================================\n";
        std::cout << "||    Gold & Alloy Toolkit    ||\n";
        std::cout << "================================\n";
        std::cout << "  Current Gold Price: " << std::fixed << std::setprecision(2)
            << (g_currentGoldPrice > 0 ? std::to_string(g_currentGoldPrice) : "Not Set") << "\n\n";
        std::cout << "1. Calculate Purity (from Weight)\n";
        std::cout << "2. Calculate Purity (from Density)\n";
        std::cout << "3. Alloying Calculator (Target Karat)\n";
        std::cout << "4. Show Karat Reference Table\n";
        std::cout << "5. Update Gold Price\n";
        std::cout << "6. Help / Usage Guide\n";
        std::cout << "7. Exit\n";
        std::cout << "================================\n";
        std::cout << "Enter your choice: ";

        std::cin >> choice;

        if (std::cin.good()) {
            switch (choice) {
            case 1: performPurityCalculation(); break;
            case 2: performDensityToPurityConversion(); break;
            case 3: performAlloyingCalculation(); break;
            case 4: displayKaratInfo(); break;
            case 5: manageGoldPrice(); break;
            case 6: displayHelp(); break;
            case 7: std::cout << "Exiting program. Goodbye!\n"; break;
            default: std::cout << "Invalid choice. Please try again.\n";
            }
        }
        else {
            std::cout << "Invalid input. Please enter a number.\n";
            clearInputBuffer();
            choice = 0;
        }

        if (choice != 7) {
            std::cout << "\nPress Enter to return to the main menu...";
            clearInputBuffer();
            std::cin.get();
        }

    } while (choice != 7);

    return 0;
}
