#include <iostream>
#include <time.h>
#include <cassert>

#include "Memory.h"

int main(int argc, char** argv){
    void* mem;
    Memory memory(mem, 1 * sizeof(long), 4 * sizeof(long), 64 * sizeof(long), "lol");
    for (int i = 0; i < 64; ++i){
        long tmp = i;
        memory.Write(i * sizeof(long), &tmp, sizeof(long));
    }
    for (int i = 0; i < 64; ++i){
        long* tmp = (long*)memory.Read(i * sizeof(long), sizeof(long));
        std::cout << *tmp << "\n";
    }
    return 0;
}