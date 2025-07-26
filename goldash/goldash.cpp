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
void performPurityCalculation();


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
    std::cout << "\nSelect the suspected impurity metal in the alloy:\n";
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
 * @brief Runs the main logic for calculating gold purity.
 */
void performPurityCalculation() {
    std::string impurityName;
    double densityOfImpurity = chooseImpurity(impurityName);

    std::cout << "\n--- New Calculation (assuming " << impurityName << " alloy) ---\n";

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

    // --- IMPROVED LOGIC: Check if the density is physically possible for the alloy ---
    double lowerBound = std::min(DENSITY_PURE_GOLD, densityOfImpurity);
    double upperBound = std::max(DENSITY_PURE_GOLD, densityOfImpurity);

    // Check if the object's density falls within the range of its supposed components.
    // A small tolerance (0.05) is added to account for minor measurement variations.
    if (objectDensity < lowerBound - 0.05 || objectDensity > upperBound + 0.05) {
        std::cout << "Error: The calculated density is not physically possible for a Gold-" << impurityName << " alloy.\n";
        std::cout << "The density should be between " << lowerBound << " g/cm^3 (" << (lowerBound == DENSITY_PURE_GOLD ? "Gold" : impurityName)
            << ") and " << upperBound << " g/cm^3 (" << (upperBound == DENSITY_PURE_GOLD ? "Gold" : impurityName) << ").\n";
    }
    else if (std::abs(objectDensity - DENSITY_PURE_GOLD) < 0.05) {
        // If density is very close to pure gold, consider it 100% pure.
        massOfPureGold = weightInAir;
        purityPercentage = 100.0;
        karats = 24.0;
    }
    else {
        // The formula for volume fraction works correctly regardless of which metal is denser.
        // V_f_gold = (rho_object - rho_impurity) / (rho_gold - rho_impurity)
        double volumeFractionGold = (objectDensity - densityOfImpurity) / (DENSITY_PURE_GOLD - densityOfImpurity);

        massOfPureGold = (volumeFractionGold * objectVolume) * DENSITY_PURE_GOLD;
        purityPercentage = (massOfPureGold / weightInAir) * 100.0;
        karats = purityPercentage * (24.0 / 100.0);
    }

    std::cout << "\n--- Purity Analysis (assuming " << impurityName << " alloy) ---\n";
    std::cout << "Purity by mass: " << purityPercentage << "%\n";
    std::cout << "Karat value: " << karats << "K\n";
    std::cout << "Total pure gold content: " << massOfPureGold << " grams\n\n";
}

int main() {
    int choice;

    std::cout << "--- Gold Purity Calculator ---" << std::endl;
    std::cout << "This program estimates gold purity based on its weight in air and water." << std::endl;

    do {
        std::cout << "--- Main Menu ---\n";
        std::cout << "1. Calculate Gold Purity\n";
        std::cout << "2. Show Karat Reference Table\n";
        std::cout << "3. Exit\n";
        std::cout << "Enter your choice: ";

        std::cin >> choice;

        if (std::cin.good()) {
            switch (choice) {
            case 1:
                performPurityCalculation();
                break;
            case 2:
                displayKaratInfo();
                break;
            case 3:
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

    } while (choice != 3);

    return 0;
}
