/*
 * EECS 348 Lab 10: String-Based Calculator
 * Author: Shaan Bawa
 *
 * This program reads pairs of numbers as strings from a file,
 * validates them, and performs string-based addition.
 */

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype> // For isdigit()
#include <algorithm> // For swap()

using namespace std;

/**
 * @brief Checks if a string represents a valid double number.
 * Format: (+|-)A(.B)
 * - Must have at least one digit (A).
 * - If decimal point (.) exists, must have digits on *both* sides (A and B).
 */
bool isValidDouble(const string& s) {
    if (s.empty()) {
        return false;
    }

    size_t i = 0;                      // use size_t to match s.length()
    bool seenDecimal = false;
    int digitsBefore = 0;
    int digitsAfter = 0;

    // 1. Check for optional sign
    if (s[0] == '+' || s[0] == '-') {
        i = 1;
    }

    // If string is just "+" or "-", it's invalid
    if (i == 1 && s.length() == 1) {
        return false;
    }

    // 2. Loop through the rest of the string
    for (; i < s.length(); ++i) {
        unsigned char ch = static_cast<unsigned char>(s[i]);
        if (std::isdigit(ch)) {
            if (seenDecimal) {
                digitsAfter++;
            } else {
                digitsBefore++;
            }
        } else if (s[i] == '.') {
            if (seenDecimal) {
                return false; // Multiple decimal points
            }
            seenDecimal = true;
        } else {
            return false; // Invalid character
        }
    }

    // 3. Final checks
    if (!seenDecimal) {
        // No decimal, just need at least one digit (e.g., "123", "+1")
        return digitsBefore > 0;
    } else {
        // Had a decimal, must have digits on both sides (e.g., "1.0", "-0.5")
        // This rules out "5." and ".5"
        return digitsBefore > 0 && digitsAfter > 0;
    }
}

/**
 * @brief Normalizes a number string:
 * - Removes leading zeros from integer part (e.g., "007.5" -> "7.5")
 * - Removes trailing zeros from decimal part (e.g., "7.500" -> "7.5")
 * - Removes decimal point if it becomes trailing (e.g., "7.0" -> "7")
 * - Handles negative signs and the zero case ("-0.0" -> "0")
 */
string normalize(string s) {
    string sign = "";
    if (s[0] == '-') {
        sign = "-";
        s = s.substr(1);
    }

    string intPart, decPart;
    size_t decPos = s.find('.');

    if (decPos == string::npos) {
        intPart = s;
        decPart = "";
    } else {
        intPart = s.substr(0, decPos);
        decPart = s.substr(decPos + 1);
    }

    // Normalize integer part (remove leading zeros)
    size_t firstDigit = intPart.find_first_not_of('0');
    if (firstDigit == string::npos) {
        intPart = "0"; // String was "0", "00", etc.
    } else {
        intPart = intPart.substr(firstDigit); // "007" -> "7"
    }

    // Normalize decimal part (remove trailing zeros)
    size_t lastDigit = decPart.find_last_not_of('0');
    if (lastDigit == string::npos) {
        decPart = ""; // String was ".0", ".00", etc.
    } else {
        decPart = decPart.substr(0, lastDigit + 1); // "500" -> "5"
    }

    // Reconstruct
    string result;
    if (decPart.empty()) {
        result = intPart;
    } else {
        result = intPart + "." + decPart;
    }

    // Handle zero case (e.g., "-0.0" -> "0")
    if (result == "0") {
        return "0";
    }

    return sign + result;
}

/**
 * @brief Aligns two positive number strings by padding with zeros.
 * (e.g., "10.5" and "1.005" -> "10.500" and "01.005")
 */
void alignNumbers(string& num1, string& num2) {
    string intPart1, decPart1, intPart2, decPart2;
    size_t decPos1 = num1.find('.');
    size_t decPos2 = num2.find('.');

    intPart1 = (decPos1 == string::npos) ? num1 : num1.substr(0, decPos1);
    decPart1 = (decPos1 == string::npos) ? "" : num1.substr(decPos1 + 1);
    intPart2 = (decPos2 == string::npos) ? num2 : num2.substr(0, decPos2);
    decPart2 = (decPos2 == string::npos) ? "" : num2.substr(decPos2 + 1);

    // Pad integer parts with leading zeros
    while (intPart1.length() < intPart2.length()) intPart1 = "0" + intPart1;
    while (intPart2.length() < intPart1.length()) intPart2 = "0" + intPart2;

    // Pad decimal parts with trailing zeros
    while (decPart1.length() < decPart2.length()) decPart1 += "0";
    while (decPart2.length() < decPart1.length()) decPart2 += "0";

    bool hasDecimal = (decPos1 != string::npos || decPos2 != string::npos || !decPart1.empty());

    if (hasDecimal) {
        num1 = intPart1 + "." + decPart1;
        num2 = intPart2 + "." + decPart2;
    } else {
        num1 = intPart1;
        num2 = intPart2;
    }
}

/**
 * @brief Checks if positive number string num1 >= num2.
 */
bool stringIsGreater(string num1, string num2) {
    // Normalize before comparison
    num1 = normalize(num1);
    num2 = normalize(num2);

    string intPart1, decPart1, intPart2, decPart2;
    size_t decPos1 = num1.find('.');
    size_t decPos2 = num2.find('.');

    intPart1 = (decPos1 == string::npos) ? num1 : num1.substr(0, decPos1);
    decPart1 = (decPos1 == string::npos) ? "" : num1.substr(decPos1 + 1);
    intPart2 = (decPos2 == string::npos) ? num2 : num2.substr(0, decPos2);
    decPart2 = (decPos2 == string::npos) ? "" : num2.substr(decPos2 + 1);

    // 1. Compare integer part lengths
    if (intPart1.length() > intPart2.length()) return true;
    if (intPart1.length() < intPart2.length()) return false;

    // 2. Compare integer parts directly
    if (intPart1 > intPart2) return true;
    if (intPart1 < intPart2) return false;

    // 3. Integer parts are equal, compare decimal parts (must pad)
    while (decPart1.length() < decPart2.length()) decPart1 += "0";
    while (decPart2.length() < decPart1.length()) decPart2 += "0";

    return decPart1 >= decPart2;
}


/**
 * @brief Adds two positive, aligned number strings.
 */
string stringAddMag(string num1, string num2) {
    alignNumbers(num1, num2);
    string result = "";
    int carry = 0;

    for (int i = num1.length() - 1; i >= 0; i--) {
        if (num1[i] == '.') {
            result = "." + result;
            continue;
        }

        int sum = (num1[i] - '0') + (num2[i] - '0') + carry;
        result = to_string(sum % 10) + result;
        carry = sum / 10;
    }

    if (carry > 0) {
        result = to_string(carry) + result;
    }
    return result;
}

/**
 * @brief Subtracts two positive, aligned number strings (num1 - num2).
 * Assumes num1 >= num2.
 */
string stringSubtractMag(string num1, string num2) {
    alignNumbers(num1, num2);
    string result = "";
    int borrow = 0;

    for (int i = num1.length() - 1; i >= 0; i--) {
        if (num1[i] == '.') {
            result = "." + result;
            continue;
        }

        int diff = (num1[i] - '0') - (num2[i] - '0') - borrow;
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result = to_string(diff) + result;
    }
    return result;
}


/**
 * @brief Main dispatcher function for addition. Handles signs.
 */
string stringAdd(string num1, string num2) {
    string sign1 = (num1[0] == '-') ? "-" : "+";
    string mag1 = (sign1 == "-" || num1[0] == '+') ? num1.substr(1) : num1;

    string sign2 = (num2[0] == '-') ? "-" : "+";
    string mag2 = (sign2 == "-" || num2[0] == '+') ? num2.substr(1) : num2;

    string result;

    if (sign1 == sign2) {
        // Both positive or both negative (e.g., 5 + 2 or -5 + -2)
        result = stringAddMag(mag1, mag2);
        if (sign1 == "-") {
            result = "-" + result;
        }
    } else {
        // Mixed signs (e.g., 5 + -2 or -5 + 2)
        if (stringIsGreater(mag1, mag2)) {
            // e.g., 5 + (-2) -> 5 - 2 = 3
            // e.g., -5 + 2 -> -(5 - 2) = -3
            result = stringSubtractMag(mag1, mag2);
            if (sign1 == "-") {
                result = "-" + result;
            }
        } else {
            // e.g., 2 + (-5) -> -(5 - 2) = -3
            // e.g., -2 + 5 -> 5 - 2 = 3
            result = stringSubtractMag(mag2, mag1);
            if (sign2 == "-") {
                result = "-" + result;
            }
        }
    }

    return normalize(result);
}

/**
 * @brief Main function to read file and process lines.
 */
int main(int argc, char* argv[]) {
    // 1. Check for correct command-line arguments
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    // 2. Open the input file
    ifstream file(argv[1]);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << argv[1] << endl;
        return 1;
    }

    string line;
    // 3. Read file line by line
    while (getline(file, line)) {
        stringstream ss(line);
        string num1, num2;

        // 4. Parse the two numbers from the line
        if (!(ss >> num1 >> num2)) {
            // Skip empty lines or lines with parsing errors
            continue;
        }

        // 5. Validate both numbers
        if (isValidDouble(num1) && isValidDouble(num2)) {
            // 6. If valid, perform addition and print result
            cout << stringAdd(num1, num2) << endl;
        } else {
            // 7. If invalid, print error message
            cout << "Invalid input on line: " << line << endl;
        }
    }

    file.close();
    return 0;
}