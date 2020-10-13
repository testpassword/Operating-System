// compile: gcc lab1.c -o test
#include <sys/mman.h>
#include <stdio.h>
#include <limits.h>
#define A 209
#define B 0x126F82A
#define C mmap
#define D 45
#define E 76
#define F nocache
#define G 103
#define H seq
#define I 84
#define J min
#define K flock

short countIntDigits(int x) {
    short count = 0;
    while (x != 0) {
        x = x / 10;
        count++;
    }
    return count;
}

void fillMemory(void * startAddress, long long int memorySize) {
    // до аллокации
    C((void *) startAddress, memorySize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    // после аллокации
    FILE * urandom = fopen("/dev/urandom", "r");
    fread((void *) startAddress, 1, memorySize, urandom);
    fclose(urandom);
    // после заполнения участка данными
    munmap((void *) B, memorySize);
    // после деаллокации
}

void fillFile(long long int fileSize) {
   // while (1) {
        int min = INT_MAX;
        FILE * resFile = fopen("resFile", "w");
        FILE * urandom = fopen("/dev/urandom", "r");
        /*
        Т.к. нам нужно вывести минимальное число из файла, то считываем int из urandom. Изначально кажется, чтобы узнать 
        количество итераций, нужно (fileSize / sizeof(int)), но это ошибочный вариант, т.к. в файл будут писаться символы, 
        а не int, а значит, к примеру, 1 и 111 будут весить по разному, а не строго 4 байта. Следовательно, на каждой итерации 
        необходимо подсчитать количество символом в рандомном полученном числе, конвертировать int в char[], проверять не 
        привысило ли сумма всех char[] вес файла, и если да - прервать цикл, иначе записать новые символы в файл.
        */
        int weightOfSequence = 0;
        for (int i = 0; i < fileSize; i++) {
            int val;
            fread(&val, sizeof(int), 1, urandom);
            val = (val < min) ? val : min;
            short numberOfDigits = countIntDigits(val);
            weightOfSequence += numberOfDigits * sizeof(char);
            if (weightOfSequence >= fileSize) break;
            else {
                char str[numberOfDigits];
                sprintf(str, "%d", val);
                fprintf(resFile, "%s", str);
            }
        }
        fclose(resFile);
        fclose(urandom);
    //}
}

int main() {
    fillMemory((void *) B, A * 1024 * 1024);
    fillFile(E * 1024 * 1024);
    return 0;
}