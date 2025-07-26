#include <iostream>
#include <iomanip> // For std::setprecision
#include <limits>  // For std::numeric_limits

// --- Constants based on standard physical properties ---
// Densities are in grams per cubic centimeter (g/cm^3)
const double DENSITY_PURE_GOLD = 19.32;
const double DENSITY_COPPER = 8.96;    // Common metal used in gold alloys
const double DENSITY_WATER = 1.0;      // Density of water is approx. 1.0 g/cm^3

/**
 * @brief Clears the input buffer to handle invalid user input.
 */
void clearInputBuffer() {
    std::cin.clear(); // Clear error flags
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard bad input
}

/**
 * @brief Prompts the user for a numeric value and validates the input.
 * * @param prompt The message to display to the user.
 * @return double The validated, positive numeric value entered by the user.
 */
double getValidatedInput(const std::string& prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;

        if (std::cin.good() && value > 0) {
            // Input is a valid positive number
            return value;
        }
        else {
            std::cout << "Invalid input. Please enter a positive number." << std::endl;
            clearInputBuffer();
        }
    }
}

int main() {
    // --- Introduction and User Input ---
    std::cout << "--- Gold Purity Calculator ---" << std::endl;
    std::cout << "This program estimates gold purity based on its weight in air and water." << std::endl;
    std::cout << "It assumes the only impurity is Copper." << std::endl << std::endl;

    double weightInAir = getValidatedInput("Enter the weight of the gold bar in air (grams): ");
    double weightInWater;

    // Loop to ensure weight in water is less than weight in air
    while (true) {
        weightInWater = getValidatedInput("Enter the weight of the gold bar in water (grams): ");
        if (weightInWater < weightInAir) {
            break;
        }
        else {
            std::cout << "Error: Weight in water must be less than weight in air. Please try again." << std::endl;
        }
    }

    // --- Calculations based on Archimedes' Principle ---

    // 1. The difference in weight is the mass of the displaced water.
    double displacedWaterMass = weightInAir - weightInWater;

    // 2. The volume of the object is equal to the volume of the displaced water.
    // Since density of water is ~1 g/cm^3, Volume = Mass / 1.0.
    double objectVolume = displacedWaterMass / DENSITY_WATER;

    // 3. Calculate the density of the object.
    double objectDensity = weightInAir / objectVolume;

    // --- Purity Analysis ---
    double massOfPureGold = 0.0;
    double purityPercentage = 0.0;
    double karats = 0.0;

    std::cout << "\n--- Calculation Results ---" << std::endl;
    std::cout << std::fixed << std::setprecision(2); // Set output to 2 decimal places
    std::cout << "Calculated Density of the bar: " << objectDensity << " g/cm^3" << std::endl;

    // Check if the density is within the range of a gold-copper alloy
    if (objectDensity > DENSITY_PURE_GOLD + 0.05) { // Adding a small tolerance
        std::cout << "Warning: Density is higher than pure gold. The item might be hollow or contain a denser metal like Platinum." << std::endl;
    }
    else if (objectDensity < DENSITY_COPPER - 0.05) { // Adding a small tolerance
        std::cout << "Warning: Density is lower than copper. The item is likely not a gold alloy." << std::endl;
    }
    else if (abs(objectDensity - DENSITY_PURE_GOLD) < 0.05) {
        // The object is essentially pure gold
        massOfPureGold = weightInAir;
        purityPercentage = 100.0;
        karats = 24.0;
    }
    else {
        // It's an alloy. Calculate the volume fraction of gold.
        // Formula derived from: objectDensity = (volFractionGold * DENSITY_PURE_GOLD) + ((1 - volFractionGold) * DENSITY_COPPER)
        double volumeFractionGold = (objectDensity - DENSITY_COPPER) / (DENSITY_PURE_GOLD - DENSITY_COPPER);

        // Calculate the mass of the pure gold component
        massOfPureGold = (volumeFractionGold * objectVolume) * DENSITY_PURE_GOLD;

        // Calculate purity as a percentage of total mass
        purityPercentage = (massOfPureGold / weightInAir) * 100.0;

        // Convert mass purity to Karats (24 Karat is 100% pure)
        karats = purityPercentage * (24.0 / 100.0);
    }

    // --- Final Output ---
    std::cout << "\n--- Purity Analysis (assuming Copper alloy) ---" << std::endl;
    std::cout << "Purity by mass: " << purityPercentage << "%" << std::endl;
    std::cout << "Karat value: " << karats << "K" << std::endl;
    std::cout << "Total pure gold content: " << massOfPureGold << " grams" << std::endl;

    return 0;
}
