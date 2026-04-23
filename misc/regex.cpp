#include <iostream>
#include <string>
#include <regex> // Essential header
#include <iterator>

int main() {
    std::string text = "Items: ItemA123, ItemB456, ItemC789.";
    std::string text2 = "Order ID: 12345";
    std::regex pattern2(R"(ID: (\d+))"); // (\id+) must represent digits
    std::smatch matches,  matches2;

    if (std::regex_search(text2, matches2, pattern2)) {
        std::cout << "Full match: " << matches2.str(0) << std::endl; // "ID: 12345"
        std::cout << "Capture group 1: " << matches2.str(1) << std::endl; // "12345"
    }
    // 1. SEARCH: Check if a pattern exists in the text
    std::regex pattern(R"(([a-zA-Z]+)(\d+))"); // Match Alpha + Digits
    if (std::regex_search(text, pattern)) {
        std::cout << "Pattern found!" << std::endl;
    }

    // 2. ITERATE: Extract all matches (Matches ItemA123, ItemB456, etc.)
    std::cout << "\nFound matches:" << std::endl;
    auto words_begin = std::sregex_iterator(text.begin(), text.end(), pattern);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::cout << "  Full Match: " << match.str() 
                  << " [Name: " << match[1] 
                  << ", ID: " << match[2] << "]" << std::endl;
    }

    // 3. REPLACE: Replace the digits with [ID]
    std::string replacement = "$1[ID]";
    std::string modified = std::regex_replace(text, pattern, replacement);
    std::cout << "\nOriginal: " << text << std::endl;
    std::cout << "Replaced: " << modified << std::endl;

    return 0;
}
