/*
 * EECS 348 Lab 10: String-Based Calculator (fixed)
 * Author: Shaan Bawa (fixed version)
 *
 * This program reads pairs of numbers as strings from a file,
 * validates them, and performs string-based addition without
 * converting to built-in numeric types.
 */

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>

using namespace std;

/* ----------------- Validator ----------------- */
/**
 * @brief Checks if a string represents a valid double number.
 * Format: (+|-)A(.B)
 * - Optional leading '+' or '-'
 * - At least one digit before decimal (A)
 * - If decimal exists, at least one digit after decimal (B)
 */
bool isValidDouble(const string& s) {
    if (s.empty()) return false;

    size_t i = 0;
    bool seenDecimal = false;
    int digitsBefore = 0;
    int digitsAfter = 0;

    // optional sign
    if (s[0] == '+' || s[0] == '-') {
        i = 1;
    }

    // string can't be only sign
    if (i == 1 && s.length() == 1) return false;

    for (; i < s.length(); ++i) {
        unsigned char ch = static_cast<unsigned char>(s[i]);
        if (isdigit(ch)) {
            if (seenDecimal) digitsAfter++;
            else digitsBefore++;
        } else if (s[i] == '.') {
            if (seenDecimal) return false; // multiple dots
            seenDecimal = true;
        } else {
            return false; // invalid char
        }
    }

    if (!seenDecimal) {
        return digitsBefore > 0;
    } else {
        // digit required on both sides of decimal
        return digitsBefore > 0 && digitsAfter > 0;
    }
}

/* ----------------- Helpers for signs and normalization ----------------- */

/**
 * @brief Strip leading '+' if present. Leaves leading '-' intact.
 */
string stripPlus(const string& s) {
    if (!s.empty() && s[0] == '+') return s.substr(1);
    return s;
}

/**
 * @brief Extract sign character. Returns '+' or '-'.
 */
char getSign(const string& s) {
    if (!s.empty() && s[0] == '-') return '-';
    return '+';
}

/**
 * @brief Remove leading sign if present ('+' or '-'), returning magnitude only.
 */
string magnitudeOnly(const string& s) {
    if (s.empty()) return s;
    if (s[0] == '+' || s[0] == '-') return s.substr(1);
    return s;
}

/**
 * @brief Normalize a number string that may include a sign.
 * - Removes leading '+'
 * - Removes leading zeros in integer part
 * - Removes trailing zeros in decimal part
 * - Removes decimal point if decimal part becomes empty
 * - Converts any zero result (including "-0", "-0.0") to "0"
 *
 * Example: "+007.500" -> "7.5", "-0.000" -> "0", "-001.2300" -> "-1.23"
 */
string normalize(const string& raw) {
    if (raw.empty()) return raw;

    // handle sign
    char sign = getSign(raw);
    string mag = magnitudeOnly(raw);

    // split into integer and decimal parts
    size_t dot = mag.find('.');
    string intPart = (dot == string::npos) ? mag : mag.substr(0, dot);
    string decPart = (dot == string::npos) ? "" : mag.substr(dot + 1);

    // remove leading zeros from integer part
    size_t firstNonZero = intPart.find_first_not_of('0');
    if (firstNonZero == string::npos) {
        intPart = "0";
    } else {
        intPart = intPart.substr(firstNonZero);
    }

    // remove trailing zeros from decimal part
    if (!decPart.empty()) {
        size_t lastNonZero = decPart.find_last_not_of('0');
        if (lastNonZero == string::npos) {
            decPart = "";
        } else {
            decPart = decPart.substr(0, lastNonZero + 1);
        }
    }

    // reconstruct
    string result = intPart;
    if (!decPart.empty()) result += "." + decPart;

    // zero canonicalization
    if (result == "0") return "0";

    if (sign == '-') return string("-") + result;
    return result;
}

/* ----------------- Alignment and magnitude helpers ----------------- */

/**
 * @brief Given two magnitude strings (no sign), pad integer parts with leading zeros
 * and decimal parts with trailing zeros so the two strings have same layout.
 * Example: "10.5", "1.005" -> "10.500", "01.005"
 *
 * Both inputs are magnitudes (no '+' or '-' at front). Function modifies inputs in-place.
 */
void alignMagnitudes(string& a, string& b) {
    // split
    size_t da = a.find('.');
    size_t db = b.find('.');

    string intA = (da == string::npos) ? a : a.substr(0, da);
    string decA = (da == string::npos) ? "" : a.substr(da + 1);
    string intB = (db == string::npos) ? b : b.substr(0, db);
    string decB = (db == string::npos) ? "" : b.substr(db + 1);

    // pad integer parts
    if (intA.length() < intB.length()) intA = string(intB.length() - intA.length(), '0') + intA;
    if (intB.length() < intA.length()) intB = string(intA.length() - intB.length(), '0') + intB;

    // pad decimal parts
    if (decA.length() < decB.length()) decA += string(decB.length() - decA.length(), '0');
    if (decB.length() < decA.length()) decB += string(decA.length() - decB.length(), '0');

    bool hasDecimal = (!decA.empty() || !decB.empty());

    if (hasDecimal) {
        a = intA + "." + decA;
        b = intB + "." + decB;
    } else {
        a = intA;
        b = intB;
    }
}

/**
 * @brief Compare two positive magnitude strings (no sign). Returns true if a >= b.
 * Uses normalize internally to ensure canonical forms.
 */
bool magnitudeGE(string a, string b) {
    a = normalize(a);
    b = normalize(b);

    // remove any leading sign artifacts (normalize should not produce +)
    a = magnitudeOnly(a);
    b = magnitudeOnly(b);

    // split int/dec
    size_t da = a.find('.');
    size_t db = b.find('.');

    string intA = (da == string::npos) ? a : a.substr(0, da);
    string decA = (da == string::npos) ? "" : a.substr(da + 1);
    string intB = (db == string::npos) ? b : b.substr(0, db);
    string decB = (db == string::npos) ? "" : b.substr(db + 1);

    // compare integer length
    if (intA.length() > intB.length()) return true;
    if (intA.length() < intB.length()) return false;

    // compare integer lexicographically
    if (intA > intB) return true;
    if (intA < intB) return false;

    // pad decimals and compare
    if (decA.length() < decB.length()) decA += string(decB.length() - decA.length(), '0');
    if (decB.length() < decA.length()) decB += string(decA.length() - decB.length(), '0');

    return decA >= decB;
}

/* ----------------- Magnitude addition/subtraction ----------------- */

/**
 * @brief Add two positive magnitudes (strings without sign). Assumes they may be unaligned.
 * Returns the raw magnitude string (may have leading digits, decimal point). Caller should normalize result.
 */
string addMagnitudes(string a, string b) {
    alignMagnitudes(a, b);
    string res = "";
    int carry = 0;

    for (int i = (int)a.length() - 1; i >= 0; --i) {
        if (a[i] == '.') {
            res.insert(res.begin(), '.');
            continue;
        }
        int da = a[i] - '0';
        int db = b[i] - '0';
        int sum = da + db + carry;
        res.insert(res.begin(), char('0' + (sum % 10)));
        carry = sum / 10;
    }

    if (carry) res.insert(res.begin(), char('0' + carry));
    return res;
}

/**
 * @brief Subtract two positive magnitudes (a - b). Precondition: a >= b.
 * Assumes they may be unaligned. Returns raw magnitude string; caller should normalize.
 */
string subtractMagnitudes(string a, string b) {
    alignMagnitudes(a, b);
    string res = "";
    int borrow = 0;

    for (int i = (int)a.length() - 1; i >= 0; --i) {
        if (a[i] == '.') {
            res.insert(res.begin(), '.');
            continue;
        }
        int da = a[i] - '0';
        int db = b[i] - '0';
        int diff = da - db - borrow;
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        res.insert(res.begin(), char('0' + diff));
    }

    // At this point, res may have leading zeros; normalize will fix.
    return res;
}

/* ----------------- Public add (handles signs) ----------------- */

/**
 * @brief Add two signed number strings (may include leading '+' or '-').
 * Returns normalized signed result string.
 */
string stringAdd(const string& raw1, const string& raw2) {
    // canonicalize inputs: remove leading '+' if present
    string s1 = stripPlus(raw1);
    string s2 = stripPlus(raw2);

    char sign1 = getSign(s1);
    char sign2 = getSign(s2);

    string mag1 = magnitudeOnly(s1);
    string mag2 = magnitudeOnly(s2);

    string resultMag;
    char resultSign = '+';

    if (sign1 == sign2) {
        // same sign: add magnitudes, keep sign (unless result is zero)
        resultMag = addMagnitudes(mag1, mag2);
        resultSign = sign1;
    } else {
        // different signs: subtract smaller magnitude from larger magnitude
        if (magnitudeGE(mag1, mag2)) {
            resultMag = subtractMagnitudes(mag1, mag2);
            resultSign = sign1; // sign of the larger magnitude (mag1 >= mag2)
        } else {
            resultMag = subtractMagnitudes(mag2, mag1);
            resultSign = sign2;
        }
    }

    string normalized = normalize((resultSign == '-') ? "-" + resultMag : resultMag);
    return normalized;
}

/* ----------------- Main: file I/O and processing ----------------- */

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    ifstream in(argv[1]);
    if (!in.is_open()) {
        cerr << "Error: Could not open file " << argv[1] << endl;
        return 1;
    }

    string line;
    while (getline(in, line)) {
        // keep original line for error message
        string original = line;

        // skip empty lines or lines that are whitespace
        bool allws = true;
        for (char c : line) if (!isspace(static_cast<unsigned char>(c))) { allws = false; break; }
        if (allws) continue;

        stringstream ss(line);
        string a, b;
        if (!(ss >> a >> b)) {
            // not two tokens -> treat as invalid
            cout << "Invalid input on line: " << original << endl;
            continue;
        }

        if (!isValidDouble(a) || !isValidDouble(b)) {
            cout << "Invalid input on line: " << original << endl;
            continue;
        }

        cout << stringAdd(a, b) << endl;
    }

    in.close();
    return 0;
}
