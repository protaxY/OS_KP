//
// Created by protaxy on 12/27/20.
//

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
#include <math.h>

long long timer = 8;

struct PageNote{
    long long LastRead = -1;
    long long LastWrite = -1;
    int RealMemoryPlace = -1;
    bool Loaded = false;
    //bool Allocated = false;
};

struct Page{
    int PageSize;
    void* Data;
    Page(){
        PageSize = 0;
        Data = nullptr;
    }
    Page(int size){
        PageSize = size;
        Data = malloc(size);
    }
};

class Memory{
private:
    int PageSize;
    int LogPageSize = log2(PageSize);
    int MemorySize;
    int VirtualSize;
    char* virtualMemoryFilePath;
    Page* RealMemory;
    std::vector<PageNote> Pages;
    long long cnt = 0;
    int SwapPageType = -1;

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

        int id = open("swapFile", O_RDWR);

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
    Memory(void* memory, int pageSize, int memorySize, int virtualSize, char* virtualMemoryFilePath){
        PageSize = pageSize;
        MemorySize = memorySize;
        VirtualSize = virtualSize;
        LogPageSize = log2(PageSize);

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

        FILE* swapFile = fopen("swapFile", "wb");
        if (swapFile == NULL){
            std::cout << "fopen error\n";
        }

        for (int i = 0; i < virtualSize; ++i){
            for (int j = 0; j < pageSize; ++j){
                char tmp = 0;
                fwrite(&tmp, sizeof(char), 1, swapFile);
            }
        }
        fclose(swapFile);
    }

//    void* Allocate(int address, int bytesSize){
//        for (int i = 0; i < Pages.size(); ++i){
//            if (Pages[i].Allocated == false){
//
//            }
//        }
//    }

    void* Read(int address, int bytesSize){
        void* result = malloc(bytesSize);
        if (Pages[(long) address >> LogPageSize].Loaded){
            memcpy(result,
                   (void*)((char*)RealMemory[Pages[(long) address >> LogPageSize].RealMemoryPlace].Data + ((long) address & (1l << LogPageSize) - 1)),
                   bytesSize);
            ++cnt;
            return result;
        } else {
            SwapPage((long) address >> LogPageSize);
            if (!Pages[(long) address >> LogPageSize].Loaded){
                std::cout << "oops\n";
                return 0;
            }
            memcpy(result,
                   (void*)((char*)RealMemory[Pages[(long) address >> LogPageSize].RealMemoryPlace].Data + ((long) address & (1l << LogPageSize) - 1)),
                   bytesSize);
            return result;
        }
        ++cnt;
    }
    void Write(int address, void* val, int bytesSize){
//        long long a = (long) address >> PageSize;
//        if (Pages[a].Loaded){
//            std::cout << "hi\n";
//        }
        if (Pages[((long) address >> LogPageSize)].Loaded){
            memcpy((void*)((char*)RealMemory[Pages[(long) address >> LogPageSize].RealMemoryPlace].Data + ((long) address & ((long) address & (1l << LogPageSize) - 1))),
                val, bytesSize);
            ++cnt;
            return;
        } else {
            SwapPage((long) address >> LogPageSize);
            if (!Pages[(long) address >> LogPageSize].Loaded){
                std::cout << "oops\n";
                return;
            }
            memcpy((void*)((char*)RealMemory[Pages[(long) address >> LogPageSize].RealMemoryPlace].Data + ((long) address & ((long) address & (1l << LogPageSize) - 1))),
                   val, bytesSize);
        }
        ++cnt;
    }
};

//Memory CreateMemory(void* memory, int pageSize, int memorySize, int virtualSize, char* virtualMemoryFilePath){
//    Memory res;
//    res(memory, pageSize, memorySize, virtualSize, virtualMemoryFilePath);
//}

#endif //OS_KP_MEMORY_H
