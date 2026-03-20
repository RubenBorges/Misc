#include <cstdlib>
#include <iostream>
 
template <typename ...T>
void printT(T...t)
{
    ((std::cout << t << ' '), ...);
}

int main()
{
 //   if (const char* env_p = std::getenv("PATH"))
   //     std::cout << "Your PATH is: " << env_p << '\n';
   printT(1, 2, 3, "Hello");
   std::cin.get();
   return EXIT_SUCCESS;
}
