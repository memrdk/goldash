#include <iostream>
#include <iomanip> // For std::setprecision
#include <limits>  // For std::numeric_limits
#include <string>  // For std::string
#include <cmath>   // For std::abs
#include <algorithm> // For std::min and std::max

// --- Constants based on standard physical properties ---
// Densities are in grams per cubic centimeter (g/cm^3)
const double DENSITY_WATER = 1.0;
const double DENSITY_PURE_GOLD = 19.32;
// Common metals used in gold alloys
const double DENSITY_COPPER = 8.96;
const double DENSITY_SILVER = 10.49;
const double DENSITY_PLATINUM = 21.45; // Note: Denser than gold
const double DENSITY_PALLADIUM = 12.02;


// --- Forward declarations of functions ---
void clearInputBuffer();
double getValidatedNumericInput(const std::string& prompt);
void displayKaratInfo();
double chooseImpurity(std::string& impurityName);
void calculateMarketValue(double pureGoldMass);
void performPurityCalculation();
void performDensityToPurityConversion();
void performAlloyingCalculation();


/**
 * @brief Clears the input buffer to handle invalid user input.
 */
void clearInputBuffer() {
    std::cin.clear(); // Clear error flags
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard bad input
}

/**
 * @brief Prompts the user for a numeric value and validates it.
 * @param prompt The message to display to the user.
 * @return double The validated, positive numeric value entered by the user.
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
    std::cout << "\n--- Gold Karat Reference Table ---\n";
    std::cout << std::left << std::setw(10) << "Karat"
        << std::left << std::setw(15) << "Purity (%)"
        << "Parts of Gold\n";
    std::cout << "-------------------------------------\n";
    std::cout << std::left << std::setw(10) << "24K" << std::left << std::setw(15) << "100%" << "24/24\n";
    std::cout << std::left << std::setw(10) << "22K" << std::left << std::setw(15) << "91.7%" << "22/24\n";
    std::cout << std::left << std::setw(10) << "18K" << std::left << std::setw(15) << "75.0%" << "18/24\n";
    std::cout << std::left << std::setw(10) << "14K" << std::left << std::setw(15) << "58.3%" << "14/24\n";
    std::cout << std::left << std::setw(10) << "10K" << std::left << std::setw(15) << "41.7%" << "10/24\n";
    std::cout << "-------------------------------------\n\n";
}

/**
 * @brief Prompts the user to choose an impurity metal and returns its density.
 * @param impurityName A reference to a string that will be updated with the chosen metal's name.
 * @return The density of the chosen impurity metal.
 */
double chooseImpurity(std::string& impurityName) {
    int choice;
    std::cout << "\nSelect the other metal in the alloy:\n";
    std::cout << "1. Copper (Most common)\n";
    std::cout << "2. Silver\n";
    std::cout << "3. Platinum\n";
    std::cout << "4. Palladium\n";

    while (true) {
        std::cout << "Enter your choice (1-4): ";
        std::cin >> choice;
        if (std::cin.good()) {
            switch (choice) {
            case 1: impurityName = "Copper"; return DENSITY_COPPER;
            case 2: impurityName = "Silver"; return DENSITY_SILVER;
            case 3: impurityName = "Platinum"; return DENSITY_PLATINUM;
            case 4: impurityName = "Palladium"; return DENSITY_PALLADIUM;
            default:
                std::cout << "Invalid choice. Please select from 1-4.\n";
            }
        }
        else {
            std::cout << "Invalid input. Please enter a number.\n";
            clearInputBuffer();
        }
    }
}

/**
 * @brief Calculates the market value of a given mass of pure gold.
 * @param pureGoldMass The mass of pure gold in grams.
 */
void calculateMarketValue(double pureGoldMass) {
    if (pureGoldMass <= 0) {
        return;
    }
    std::cout << "\n--- Market Value Calculation ---\n";
    double pricePerGram = getValidatedNumericInput("Enter the current market price of gold per gram: ");
    double totalValue = pureGoldMass * pricePerGram;
    std::cout << "The estimated market value of the pure gold is: " << std::fixed << std::setprecision(2) << totalValue << "\n\n";
}

/**
 * @brief Runs the main logic for calculating gold purity from weight.
 */
void performPurityCalculation() {
    std::string impurityName;
    double densityOfImpurity = chooseImpurity(impurityName);

    std::cout << "\n--- Purity Calculation (from Weight) ---\n";

    double weightInAir = getValidatedNumericInput("Enter the weight of the gold bar in air (grams): ");
    double weightInWater;

    while (true) {
        weightInWater = getValidatedNumericInput("Enter the weight of the gold bar in water (grams): ");
        if (weightInWater < weightInAir) {
            break;
        }
        else {
            std::cout << "Error: Weight in water must be less than weight in air. Please try again.\n";
        }
    }

    double displacedWaterMass = weightInAir - weightInWater;
    double objectVolume = displacedWaterMass / DENSITY_WATER;
    double objectDensity = weightInAir / objectVolume;

    double massOfPureGold = 0.0;
    double purityPercentage = 0.0;
    double karats = 0.0;

    std::cout << "\n--- Calculation Results ---\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Calculated Density of the bar: " << objectDensity << " g/cm^3\n";

    double lowerBound = std::min(DENSITY_PURE_GOLD, densityOfImpurity);
    double upperBound = std::max(DENSITY_PURE_GOLD, densityOfImpurity);

    if (objectDensity < lowerBound - 0.05 || objectDensity > upperBound + 0.05) {
        std::cout << "Error: The calculated density is not physically possible for a Gold-" << impurityName << " alloy.\n";
    }
    else if (std::abs(objectDensity - DENSITY_PURE_GOLD) < 0.05) {
        massOfPureGold = weightInAir;
        purityPercentage = 100.0;
        karats = 24.0;
    }
    else {
        double volumeFractionGold = (objectDensity - densityOfImpurity) / (DENSITY_PURE_GOLD - densityOfImpurity);
        massOfPureGold = (volumeFractionGold * objectVolume) * DENSITY_PURE_GOLD;
        purityPercentage = (massOfPureGold / weightInAir) * 100.0;
        karats = purityPercentage * (24.0 / 100.0);
    }

    std::cout << "\n--- Purity Analysis (assuming " << impurityName << " alloy) ---\n";
    std::cout << "Purity by mass: " << purityPercentage << "%\n";
    std::cout << "Karat value: " << karats << "K\n";
    std::cout << "Total pure gold content: " << massOfPureGold << " grams\n";

    calculateMarketValue(massOfPureGold);
}

/**
 * @brief Calculates purity and mass from a known density and total weight.
 */
void performDensityToPurityConversion() {
    std::string impurityName;
    std::cout << "\n--- Purity Calculation (from Density) ---\n";

    double objectDensity = getValidatedNumericInput("Enter the object's density (g/cm^3): ");
    double totalMass = getValidatedNumericInput("Enter the object's total mass (grams): ");
    double densityOfImpurity = chooseImpurity(impurityName);

    double massOfPureGold = 0.0;
    double purityPercentage = 0.0;
    double karats = 0.0;

    double lowerBound = std::min(DENSITY_PURE_GOLD, densityOfImpurity);
    double upperBound = std::max(DENSITY_PURE_GOLD, densityOfImpurity);

    if (objectDensity < lowerBound - 0.05 || objectDensity > upperBound + 0.05) {
        std::cout << "Error: The provided density is not physically possible for a Gold-" << impurityName << " alloy.\n";
    }
    else if (std::abs(objectDensity - DENSITY_PURE_GOLD) < 0.05) {
        purityPercentage = 100.0;
        massOfPureGold = totalMass;
        karats = 24.0;
    }
    else {
        // Formula to find mass fraction (p) from densities:
        // p = (1/D_obj - 1/D_i) / (1/D_g - 1/D_i)
        double p_numerator = (1.0 / objectDensity) - (1.0 / densityOfImpurity);
        double p_denominator = (1.0 / DENSITY_PURE_GOLD) - (1.0 / densityOfImpurity);
        double massFraction = p_numerator / p_denominator;

        purityPercentage = massFraction * 100.0;
        karats = purityPercentage * (24.0 / 100.0);
        massOfPureGold = totalMass * massFraction;
    }

    std::cout << "\n--- Purity Analysis (assuming " << impurityName << " alloy) ---\n";
    std::cout << "Purity by mass: " << purityPercentage << "%\n";
    std::cout << "Karat value: " << karats << "K\n";
    std::cout << "Total pure gold content: " << massOfPureGold << " grams\n";

    calculateMarketValue(massOfPureGold);
}

/**
 * @brief Calculates the amount of impurity needed to create a target Karat gold alloy.
 */
void performAlloyingCalculation() {
    std::string impurityName;
    std::cout << "\n--- Alloying Calculator ---\n";

    double goldMass = getValidatedNumericInput("Enter the mass of PURE (24K) gold you have (grams): ");
    double targetKarat;
    while (true) {
        targetKarat = getValidatedNumericInput("Enter the target Karat value (e.g., 18, 14): ");
        if (targetKarat < 24) break;
        std::cout << "Target Karat must be less than 24.\n";
    }

    double densityOfImpurity = chooseImpurity(impurityName);

    double targetPurity = targetKarat / 24.0;
    // Formula: massOfImpurity = massOfGold * (1/targetPurity - 1)
    double impurityMass = goldMass * ((1.0 / targetPurity) - 1.0);
    double totalAlloyMass = goldMass + impurityMass;

    std::cout << "\n--- Alloying Results ---\n";
    std::cout << "To create " << targetKarat << "K gold from " << goldMass << "g of pure gold,\n";
    std::cout << "you need to add " << impurityMass << "g of " << impurityName << ".\n";
    std::cout << "This will result in a total of " << totalAlloyMass << "g of " << targetKarat << "K alloy.\n\n";
}


int main() {
    int choice;

    std::cout << "--- Gold & Alloy Toolkit ---\n";
    std::cout << "A versatile tool for purity, value, and alloy calculations." << std::endl;

    do {
        std::cout << "--- Main Menu ---\n";
        std::cout << "1. Calculate Purity (from Weight)\n";
        std::cout << "2. Calculate Purity (from Density)\n";
        std::cout << "3. Alloying Calculator (Target Karat)\n";
        std::cout << "4. Show Karat Reference Table\n";
        std::cout << "5. Exit\n";
        std::cout << "Enter your choice: ";

        std::cin >> choice;

        if (std::cin.good()) {
            switch (choice) {
            case 1:
                performPurityCalculation();
                break;
            case 2:
                performDensityToPurityConversion();
                break;
            case 3:
                performAlloyingCalculation();
                break;
            case 4:
                displayKaratInfo();
                break;
            case 5:
                std::cout << "Exiting program. Goodbye!\n";
                break;
            default:
                std::cout << "Invalid choice. Please try again.\n\n";
            }
        }
        else {
            std::cout << "Invalid input. Please enter a number.\n\n";
            clearInputBuffer();
            choice = 0; // Reset choice to prevent infinite loop if non-numeric input is entered
        }

    } while (choice != 5);

    return 0;
}
