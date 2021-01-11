#ifndef OS_KP_MEMORY_H
#define OS_KP_MEMORY_H

//16 страниц физически
//64 виртуально

#include <vector>
#include <iostream>
#include <cstdio>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <cmath>

long long timer = 8;

struct PageNote{
    long long LastRead = -1;
    long long LastWrite = -1;
    int RealMemoryPlace = -1;
    bool Loaded = false;
    int Allocated = 0;
};

struct Page{
    int PageSize;
    void* Data;
    Page(){
        PageSize = 0;
        Data = nullptr;
    }
};

class Memory{
private:
    int PageSize;
    int LogPageSize;
    int MemorySize;
    int VirtualSize;
    char* VirtualMemoryFilePath;
    Page* RealMemory;
    std::vector<PageNote> Pages;
    long long cnt = 0;

    void SwapPage(int newPage){
        int candidate = -1;
        for (int i = 0; i < Pages.size(); ++i) {
            if (cnt - Pages[i].LastWrite > timer && cnt - Pages[i].LastRead > timer && Pages[i].Loaded) {
                candidate = i;
                break;
            }
            if (cnt - Pages[i].LastRead > timer && Pages[i].Loaded) {
                candidate = i;
                continue;
            } else if (cnt - Pages[i].LastWrite > timer && Pages[i].Loaded) {
                candidate = i;
                continue;
            } else if (candidate == -1 && Pages[i].Loaded) {
                candidate = i;
                continue;
            }
        }

        int id = open(VirtualMemoryFilePath, O_RDWR);

        lseek(id, candidate * PageSize, 0);
        write(id, RealMemory[Pages[candidate].RealMemoryPlace].Data, PageSize);

        Pages[newPage].Loaded = true;
        Pages[newPage].LastRead = cnt;
        Pages[newPage].LastWrite = cnt;
        Pages[newPage].RealMemoryPlace = Pages[candidate].RealMemoryPlace;

        lseek(id, newPage * PageSize, 0);
        read(id, RealMemory[Pages[newPage].RealMemoryPlace].Data, PageSize);

        Pages[candidate].Loaded = false;
        Pages[candidate].LastRead = -1;
        Pages[candidate].LastWrite = -1;
        Pages[candidate].RealMemoryPlace = -1;

        close(id);
    }

public:
    Memory(int pageSize, int memorySize, int virtualSize, char* virtualMemoryFilePath){
        PageSize = pageSize;
        MemorySize = memorySize;
        VirtualSize = virtualSize;
        LogPageSize = log2(PageSize);
        VirtualMemoryFilePath = virtualMemoryFilePath;
        Pages.resize(virtualSize / PageSize);
        for (int i = 0; i < memorySize / PageSize; ++i){
            Pages[i].Loaded = true;
            Pages[i].LastRead = cnt;
            Pages[i].LastWrite = cnt;
            Pages[i].RealMemoryPlace = i;
        }

        RealMemory = new Page [memorySize / PageSize];
        for (int i = 0; i < memorySize / PageSize; ++i){
            RealMemory[i].PageSize = pageSize;
            RealMemory[i].Data = malloc(pageSize);
        }

        FILE* swapFile = fopen(VirtualMemoryFilePath, "wb");
        if (swapFile == NULL){
            std::cout << "fopen error\n";
        }

        for (int i = 0; i < virtualSize; ++i){
            char tmp = 0;
            fwrite(&tmp, sizeof(char), 1, swapFile);
        }
        fclose(swapFile);
    }

    int Allocate(int bytesSize){
        bool success = false;
        int result;
        for (int i = 0; i < Pages.size(); ++i){
            if (PageSize - Pages[i].Allocated >= bytesSize){
                Pages[i].Allocated += bytesSize;
                success = true;
                result = i * PageSize + Pages[i].Allocated - bytesSize;
                return result;
            }
        }
        if (!success){
            std::cout << "nowhere to allocate\n";
            return -1;
        }
    }

    void* Read(int address, int bytesSize){
        if (Pages[(long) address >> LogPageSize].Allocated < ((long) address & (1l << LogPageSize) - 1) + bytesSize){
            std::cout << "this memory block is not allcoated\n";
        }
        void* result = malloc(bytesSize);
        if (!(Pages[(long) address >> LogPageSize].Loaded)){
            SwapPage((long) address >> LogPageSize);
            if (!Pages[(long) address >> LogPageSize].Loaded){
                std::cout << "oops\n";
                return nullptr;
            }
        }
        memcpy(result,
               (void*)((char*)RealMemory[Pages[(long) address >> LogPageSize].RealMemoryPlace].Data + ((long) address & (1l << LogPageSize) - 1)),
               bytesSize);
        Pages[(long) address >> LogPageSize].LastRead = cnt;
        ++cnt;
        return result;
    }
    void Write(int address, void* val, int bytesSize){
        if (Pages[(long) address >> LogPageSize].Allocated < ((long) address & (1l << LogPageSize) - 1) + bytesSize){
            std::cout << "this memory block is not allcoated\n";
        }
        if (!(Pages[((long) address >> LogPageSize)].Loaded)){
            SwapPage((long) address >> LogPageSize);
            if (!Pages[(long) address >> LogPageSize].Loaded){
                std::cout << "oops\n";
                return;
            }
        }
        memcpy((void*)((char*)RealMemory[Pages[(long) address >> LogPageSize].RealMemoryPlace].Data + ((long) address & ((long) address & (1l << LogPageSize) - 1))),
               val, bytesSize);
        Pages[(long) address >> LogPageSize].LastWrite = cnt;
        ++cnt;
    }
};

Memory CreateMemory(int pageSize, int memorySize, int virtualSize, char* virtualMemoryFilePath){
    Memory res(pageSize, memorySize, virtualSize, virtualMemoryFilePath);
    return res;
}

#endif //OS_KP_MEMORY_H
