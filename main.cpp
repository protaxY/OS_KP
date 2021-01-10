#include <iostream>
#include <time.h>
#include <cassert>

#include "Memory.h"

int main(int argc, char** argv){
    void* mem;
    Memory memory(mem, 4 * sizeof(int), 16 * sizeof(int), 64 * sizeof(int), "lol");
    std::vector<int> customPointers(64, 0);
    for (int i = 0; i < 64; ++i){
        customPointers[i] = memory.Allocate(sizeof(int));
    }
    for(int i = 0; i < customPointers.size(); ++i){
        int tmp = i;
        memory.Write(customPointers[i], &tmp, sizeof(int));
    }
    for (int i = 0; i < customPointers.size(); ++i){
        int* tmp = (int*) memory.Read(customPointers[i], sizeof(int));
        std::cout << *tmp << " ";
    }
    return 0;
}