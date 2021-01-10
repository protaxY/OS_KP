#include <iostream>
#include <cassert>
#include <ctime>

#include "Memory.h"

int main(int argc, char** argv){
    srand(time(NULL));
    assert(argc == 4);
    unsigned int logPageSize = atoi(argv[1]);
    unsigned int memorySize = atoi(argv[2]);
    unsigned int virtualSize = atoi(argv[3]);

    if (memorySize > virtualSize){
        std::cout << "memorySize cannot be larger than virtualSize\n";
        return -1;
    }

    Memory memory = CreateMemory(1 << logPageSize, memorySize * (1 << logPageSize), virtualSize * (1 << logPageSize), "swapFile");
    std::cout << "enter array size:";
    unsigned int size;
    std::cin >> size;
    if ((size + 1) * sizeof(int) > virtualSize * (1 << logPageSize)){
        std::cout << "to many elements to sort\n";
        return -1;
    }
    std::vector<int> customPointers(size, 0);
    for (int i = 0; i < size; ++i){
        customPointers[i] = memory.Allocate(sizeof(int));
        int tmp = rand() % 100;
        memory.Write(customPointers[i], &tmp, sizeof(int));
    }

    for (int i = 0; i < customPointers.size(); ++i){
        int* tmp = (int*) memory.Read(customPointers[i], sizeof(int));
        std::cout << *tmp << " ";
    }
    std::cout << "\n";

    size_t i, j;
    int tmpPointer = memory.Allocate(sizeof(int));
    for (i = 1; i < size; i++) {
        for (j = 1; j < size; j++) {
            if (*(int *) memory.Read(customPointers[j], sizeof(int)) < *(int *) memory.Read(customPointers[j - 1], sizeof(int))) {
                *(int *) memory.Read(tmpPointer, sizeof(int)) = *(int *) memory.Read(customPointers[j], sizeof(int));
                *(int *) memory.Read(customPointers[j], sizeof(int)) = *(int *) memory.Read(customPointers[j - 1], sizeof(int));
                *(int *) memory.Read(customPointers[j - 1], sizeof(int)) = *(int *) memory.Read(tmpPointer, sizeof(int));
            }
        }
    }

    for (i = 1; i < size; i++) {
        for (j = 1; j < size; j++) {
            if (*(int *) memory.Read(customPointers[j], sizeof(int)) < *(int *) memory.Read(customPointers[j - 1], sizeof(int))) {
                memory.Write(tmpPointer, memory.Read(customPointers[j], sizeof(int)), sizeof(int));
                memory.Write(customPointers[j], memory.Read(customPointers[j - 1], sizeof(int)), sizeof(int));
                memory.Write(customPointers[j - 1], memory.Read(tmpPointer, sizeof(int)), sizeof(int));
            }
        }
    }

    for (int i = 0; i < customPointers.size(); ++i){
        int* tmp = (int*) memory.Read(customPointers[i], sizeof(int));
        std::cout << *tmp << " ";
    }
    std::cout << "\n";

    return 0;
}