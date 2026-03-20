#include <cstdlib>
#include <iostream>
 
template <typename T>
T Sum(T last)
{
   return last;
}

template <typename T, typename ...Args>
T Sum(T first, Args... args)
{
   return first + Sum(args...) ;
}

int main()
{
 //   if (const char* env_p = std::getenv("PATH"))
   //     std::cout << "Your PATH is: " << env_p << '\n';
   std::cout<<Sum(1, 2, 3, 4)<<std::endl;
   std::cin.get();
   return EXIT_SUCCESS;
}
