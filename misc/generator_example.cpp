#include <iostream>
#include <generator> // C++23
#include <ranges>

// Function returns a generator of integers
std::generator<int> odd_numbers(int limit) {
    for (int i = 1; i <= limit; ++i) {
        if (i % 2 != 0) {
            co_yield i; // Pauses here, yields value to caller
        }
    }
}

int main() {
    // Usage in a range-based for loop
    for (int n : odd_numbers(10)) {
        std::cout << n << " "; // Outputs: 1 3 5 7 9
    }
    std::cout << "\n";

    // Can be used with views
    for (int n : odd_numbers(10) | std::views::take(2)| std::views::drop(1)) {
        std::cout << n << " "; // Outputs: 1 3
    }
}
