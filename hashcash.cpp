
// PracticePad.cpp
// Demonstrates a simple Proof-of-Work (PoW) mining algorithm using SHA-256 hashing.
// Shows how to use OpenSSL's EVP API for hashing and how to search for hashes with a certain number of leading or trailing zeros (difficulty).

#include <iostream>      // For input/output streams
#include <string>        // For std::string
#include <vector>        // (Unused, but included)
#include <sstream>       // For stringstream (used in hex conversion)
#include <iomanip>       // For std::setw and std::setfill (formatting hex output)
#include <openssl/evp.h> // For OpenSSL EVP hashing functions


// Computes the SHA-256 hash of a string and returns it as a hexadecimal string.
// Uses OpenSSL's EVP API for cryptographic hashing.
std::string sha256(const std::string& str) {
    unsigned char hash[EVP_MAX_MD_SIZE]; // Buffer for hash output
    unsigned int hash_len = 0;           // Actual length of the hash
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();  // Create a new digest context
    if (!ctx) return "";
    // Initialize the context for SHA-256
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }
    // Feed the input string into the hash function
    if (EVP_DigestUpdate(ctx, str.c_str(), str.size()) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }
    // Finalize the hash and get the result
    if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }
    EVP_MD_CTX_free(ctx); // Clean up
    // Convert the hash bytes to a hexadecimal string
    std::stringstream ss;
    for(unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}


// Proof-of-Work mining function.
// Tries different nonce values to find a hash of (data + nonce) with a certain number of leading or trailing zeros.
//   data:      The input string to hash (e.g., transaction data)
//   difficulty: Number of hex zeros required (e.g., 4 means hash must start/end with "0000")
//   usePrefix: If true, require zeros at the start (prefix); if false, at the end (suffix)
// Returns: The hash string that meets the difficulty requirement.
std::string mine(std::string data, int difficulty, bool usePrefix) {
    long long nonce = 0; // Number to vary to find a valid hash
    std::string target(difficulty, '0'); // Target string of zeros

    while (true) {
        // Hash the data concatenated with the current nonce
        std::string hash = sha256(data + std::to_string(nonce));

        if (usePrefix) {
            // Check if the hash starts with the required number of zeros
            if (hash.substr(0, difficulty) == target) {
                std::cout << "Found (Prefix): " << hash << " Nonce: " << nonce << std::endl;
                return hash;
            }
        } else {
            // Check if the hash ends with the required number of zeros
            if (hash.substr(hash.size() - difficulty) == target) {
                std::cout << "Found (Suffix): " << hash << " Nonce: " << nonce << std::endl;
                return hash;
            }
        }
        nonce++; // Try the next nonce
    }
}


// Entry point: demonstrates mining with both prefix and suffix difficulty.
int main() {
    std::string data = "transaction_data"; // Example data to hash
    int diff = 6; // Number of hex zeros required (higher = harder)

    std::cout << "Mining with Prefix..." << std::endl;
    // Find a hash with 'diff' leading zeros
    mine(data, diff, true);

    std::cout << "Mining with Suffix..." << std::endl;
    // Find a hash with 'diff' trailing zeros
    mine(data, diff, false);

    return 0;
}
