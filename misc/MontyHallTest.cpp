/*
Application designed to test Monty Hall problem. 
It generates random numbers between 1 and 3 to simulate the doors in the game show. 
The output can be used to analyze the outcomes of switching vs. staying with the original choice.
*/

#include <iostream>
#include <random>

int eliminateDoor(int choice, int win, std::mt19937& gen) {
    if (choice != win) {
        return 6 - choice - win; // unique goat door
    }

    // choice == win: Monty can open either of the other two doors
    int options[2];
    int idx{0};
    for (int d{1}; d <= 3; ++d) {
        if (d != choice) options[idx++] = d;
    }
    std::uniform_int_distribution<int> pick(0, 1);
    return options[pick(gen)];
}

int main(int argc, char* argv[]) {
    // Create a random number generator for test
    std::random_device rd;  // Non-deterministic random seed
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<int> dist(1, 3);
    size_t trialnum{1000000}; // Number of trials to run
    int switchWins{0};
    int stayWins{0};


    //Monty Test
    for(size_t i = 0; i < trialnum; ++i) {
        int WIN = dist(gen); // Door with the car
        int choice =  dist(gen); // Player's initial choice
        int montyOpens{eliminateDoor(choice, WIN, gen)}; // Door Monty opens
    }

    return 0;
};

