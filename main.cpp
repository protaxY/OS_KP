#include <iostream>
#include <time.h>
#include <cassert>

#include "Memory.h"

int main(int argc, char** argv){
    assert(argc == 2);
    Memory memory(atoi(argv[1]));
    std::srand(time(NULL));
    for (int i = 0; i < 64 * 4; ++i){
        int a = rand() % 16 + 1;
        memory.Write(i, a);
        std::cout << a << " ";
    }
    std::cout << "\n";
    for (int i = 0; i < 64 * 4; ++i){
        std::cout << memory.Read(i) << " ";
    }
    return 0;
}