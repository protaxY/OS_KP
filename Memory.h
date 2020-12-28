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

long long timer = 8;

struct PageNote{
    long long LastRead = -1;
    long long LastWrite = -1;
    int RealMemoryPlace = -1;
    bool Loaded = false;
};

struct Page{
    long Data[4];
//    long& operator[] (int pos){
//        return Data[pos];
//    }
};

class Memory{
private:
    Page* RealMemory;
    std::vector<PageNote> Pages;
    long long cnt = 0;
    int SwapPageType = -1;
    int NRU(){
        int candidate = -1;
        for (int i = 0; i < Pages.size(); ++i){
            if (cnt - Pages[i].LastWrite > timer && cnt - Pages[i].LastRead > timer && Pages[i].Loaded){
                candidate = i;
                break;
            }
            if (cnt - Pages[i].LastRead > timer && Pages[i].Loaded){
                candidate = i;
                continue;
            } else if (cnt - Pages[i].LastWrite > timer && Pages[i].Loaded){
                candidate = i;
                continue;
            } else if (candidate == -1 && Pages[i].Loaded){
                candidate = i;
                continue;
            }
        }
        return candidate;
    }

    void SwapPage(int newPage){
        if (SwapPageType == 1){
            int candidate = NRU();
        }

        int id = open("swapFile", O_RDWR);

        lseek(id, candidate * sizeof(Page), 0);
        write(id, &RealMemory[Pages[candidate].RealMemoryPlace], sizeof(Page));

        Pages[newPage].Loaded = true;
        Pages[newPage].LastRead = cnt;
        Pages[newPage].LastWrite = cnt;
        Pages[newPage].RealMemoryPlace = Pages[candidate].RealMemoryPlace;

        lseek(id, newPage * sizeof(Page), 0);
        read(id, &RealMemory[Pages[newPage].RealMemoryPlace], sizeof(Page));

        Pages[candidate].Loaded = false;
        Pages[candidate].LastRead = -1;
        Pages[candidate].LastWrite = -1;
        Pages[candidate].RealMemoryPlace = -1;

        close(id);
    }

public:
    Memory(int type){
        SwapPageType = type;
        RealMemory = new Page[16];
        for (int i = 0; i < 16; ++i){
            for (int j = 0; j < 4; ++j){
                RealMemory[i].Data[j] = 0;
            }
        }

        FILE* swapFile = fopen("swapFile", "wb");
        if (swapFile == NULL){
            std::cout << "fopen error\n";
        }

        for (int i = 0; i < 64; ++i){
            for (int j = 0; j < 4; ++j){
                long tmp = 0;
                fwrite(&tmp, sizeof(long), 1, swapFile);
            }
        }

        fclose(swapFile);

        Pages.resize(64);
        for (int i = 0; i < 16; ++i){
            Pages[i].Loaded = true;
            Pages[i].LastRead = cnt;
            Pages[i].LastWrite = cnt;
            Pages[i].RealMemoryPlace = i;
        }
    }

    long Read(int address){
        if (Pages[address >> 2].Loaded == true){
            return RealMemory[Pages[address >> 2].RealMemoryPlace].Data[address & 3];
            ++cnt;
        } else {
            SwapPage(address >> 2);
            if (!Pages[address >> 2].Loaded){
                std::cout << "oops\n";
                return 0;
            }
            return RealMemory[Pages[address >> 2].RealMemoryPlace].Data[address & 3];
        }
        ++cnt;
    }
    void Write(int address, long val){
        if (Pages[address >> 2].Loaded){
            RealMemory[Pages[address >> 2].RealMemoryPlace].Data[address & 3] = val;
            ++cnt;
            return;
        } else {
            SwapPage(address >> 2);
            if (!Pages[address >> 2].Loaded){
                std::cout << "oops\n";
                return;
            }
            RealMemory[Pages[address >> 2].RealMemoryPlace].Data[address & 3] = val;
        }
        ++cnt;
    }
};

#endif //OS_KP_MEMORY_H
