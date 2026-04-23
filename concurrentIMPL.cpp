#include <iostream>
#include <print>
#include <atomic>
#include <expected>



enum class ErrorCode { InvalidInput, OutOfRange };

enum BITS {
 EXISTS   = 0,      // binary 00000000
 AWAKE    = 1 << 0, // binary 00000001
 WRITING  = 1 << 1, // binary 00000010
 READING  = 1 << 2, // binary 00000100
 REQOPEN  = 1 << 3, // binary 00001000
 OPENSET  = 1 << 4, // binary 00010000

 CONSUMED = 1 << 5, // binary 00100000
 FAILBIT  = 1 << 6, // binary 01000000
 LOCKBIT  = 1 << 7  // binary 10000000
};



std::expected<int, char> worker(std::atomic<uint8_t>& RefToMessengerAtomic) {
  int exit_code = 0;
  return exit_code;
}

int main() {
  std::atomic<uint8_t> signal = EXISTS;
  auto res = worker(signal);
  
  std::cout<<res.value() << "HEYLOOO \n";
  std::println("ELLO, BooBoo {}",res.value());
  return 0;
}
