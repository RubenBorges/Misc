#include "os_abstraction.hpp"
#include <cstdlib>
#include <iostream>
#include <print>
#include <sys/types.h>


void SetCursorPosandClear() {
  std::cout << "\033[7;1H";
  std::cout << "\033[2K";
  std::cout << "\033[7;1H";
};
void SetCursorPosandMark() {
  std::cout << "\033[7;1HCommand: ";
  std::cout << "\033[2K";
  std::cout << "\033[7;1HCommand: ";
};
void ExitAnimation(const char* message) {
         std::string dots = ".";
         for (int i = 0; i < 4; i++) {
           SetCursorPosandClear();
           std::print("{0}", message);
           std::cout << dots << std::flush;
           std::cout.flush(); 
           std::fflush(stdout);
           std::this_thread::sleep_for(std::chrono::milliseconds(500));
           dots += ".";
           if (dots.length() > 4) dots = ".";
         }
           std::cout << "\033[2J"; // Clear screen
    os_clear_screen(); // OS-specific

};
void drawHeader() {
  std::cout << "\033[1;1H\033[44mPC KiLLSWiTCh\033[0m"; // Move to top-left,
                                                        // blue background
}

void drawFooter() {
  std::cout
      << "\033[20;1H\033[42mProgram Written by: R.M. Borges\033[0m"; // Move to
                                                                     // row 20,
                                                                     // green
                                                                     // background
}

void drawMainContent() {
  std::cout << "\033[5;1HTo Shutdown PC enter '[Y]es' or '[C]ancel'...";
}

int PollUserInput() {
  std::string userInput, selection;
  std::cin >> userInput; 
  SetCursorPosandMark();
  
  for ( auto &c : userInput) selection += std::toupper(c);

  if (selection == "Q" || selection == "QUIT" 
			|| selection == "C" ||selection == "CANCEL") {
	return 1;
  }else if (selection == "Y" || selection == "YES" 
			|| selection == "S" || selection == "SHUTDOWN") {
	return 0;
  } else {
    SetCursorPosandClear();
    selection.clear();
    std::cout
        << "\nInvalid input. \n\nSHUTDOWN PC: [Y]es/[S]hutdown\n\nEXIT "
           "PROGRAM: [Q]uit/[C]ancel(C)"
        << std::endl;
    return -1;
  }
};
void drawScreen() {
  drawHeader();
  drawMainContent();
  drawFooter();
}
int main() {
  std::cout << "\033[2J"; // Clear screen
  int result{-1};
  int counter{0};
  while (result < 0 && counter < 5) {
    counter++;
	
    drawScreen();
	    SetCursorPosandClear();

    result = PollUserInput();
  }
  if (counter >= 5) {
	std::println("Too many invalid attempts.Exiting program.");
	SetCursorPosandClear();
  os_clear_screen(); // Windows-specific
  ExitAnimation("Exiting Program");
    return 2;
  }
  if (result == 1) {
    std::println("CANCELLING the shutdown process.");
    SetCursorPosandClear();
    os_clear_screen(); // OS-specific
    ExitAnimation("Exiting Program");
    return 1;
  } else {
    SetCursorPosandClear();
    os_shutdown();
    os_clear_screen(); // OS-specific
    ExitAnimation("Shutting down the computer...");
    return 0;
  }
};