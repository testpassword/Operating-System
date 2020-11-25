#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#include <limits.h>
#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h> 
#include <unistd.h>
#include <math.h>

#define A 130
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

void* test_thread() { while(1) { printf("next =>\n"); } }

short count_int_digits(int x) { return floor(log10(x) + 1); }

void fill_memory(void* start_address, long long int memory_size) {
    // до аллокации
    C((void*) start_address, memory_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    // после аллокации
    FILE* urandom = fopen("/dev/urandom", "r");
    void* thread_func() { fread((void*) start_address, 1, memory_size, urandom); }
    pthread_t mem_thrs[D];
    for (int i = 0; i < D; i++) pthread_create(&mem_thrs[i], NULL, thread_func, NULL);
    for (int i = 0; i < D; i++) pthread_join(mem_thrs[i], NULL); // здесь необходимо ждать завершения поток, иначе будет segmentation fault
    fclose(urandom);
    // после заполнения участка данными
    munmap((void*) start_address, memory_size);
    // после деаллокации
}

void fill_file(long long int file_size) {
    int restart_WThread = 1;
    void* write_thread() {
        printf("Write thread awaken\n");
        //fopen для записи в файл с данными не подходит, по причинам: это не системная функция, а из библиотеки C; нет возможности задать флаг; не использовать буфер.
        int flag = O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT;
        int fd = open("resFile", flag, 0666);
        FILE* urandom = fopen("/dev/urandom", "r");
        /*
        Изначально кажется, чтобы узнать количество итераций нужно для записи чисел, полученных из urandom нужно (file_size / sizeof(int)), 
        но это ошибочный вариант, т.к. в файл будут писаться символы, а не int, а значит, к примеру, 1 и 111 будут весить по разному, 
        а не строго 4 байта. Следовательно, на каждой итерации необходимо подсчитать количество символом в рандомном полученном числе, 
        конвертировать int в char[], проверять не привысило ли сумма всех char[] вес файла, и если да - прервать цикл, иначе записать новые символы в файл.
        P.S. Полученный файл может незначительно отличаться по размеру в большую сторону, т.к. ОС выделяет место под данные на диске секторами, размер 
        которых зависит от файловой системы.
        */
        int weight_of_sequence = 0;
        for (;;) {
            struct flock read_lock;
            memset(&read_lock, 0, sizeof(read_lock));
            read_lock.l_type = F_RDLCK;
            fcntl(fd, F_SETLKW, &read_lock);
            int val;
            fread(&val, sizeof(int), 1, urandom);
            short number_of_digits = count_int_digits(val) + 1; // + 1 для пробела: без этого, считывающие потоки не смогут разделить числа
            weight_of_sequence += number_of_digits * sizeof(char);
            if (weight_of_sequence >= file_size) break;
            else {
                char str[number_of_digits];
                sprintf(str, "%d ", val);
                write(fd, str, number_of_digits);
            }
            read_lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLKW, &read_lock);
        }
        close(fd);
        fclose(urandom);
        restart_WThread = 1;
    }
    void* agregate_thread() {
        FILE* f = fopen("resFile", "r");
        int frd = fileno(f);
        struct flock read_lock;
        memset(&read_lock, 0, sizeof(read_lock));
        read_lock.l_type = F_WRLCK;
        fcntl(frd, F_SETLKW, &read_lock);
        int num;
        int min = INT_MAX;
        while(fscanf(f, "%d ", &num) > 0) if (min > num) min = num;
        read_lock.l_type = F_UNLCK;
        fcntl(frd, F_SETLKW, &read_lock);
        fclose(f);
        //printf("The final min is %d\n", min); Выводить это нет смысла, т.к. чисел очень много, и почти сразу же будет получено INT_MIN
    }
    while (1) {
        if (restart_WThread == 1) {
            restart_WThread = 0;
            pthread_t w_thr;
            pthread_create(&w_thr, NULL, write_thread, NULL);
        }
        pthread_t a_thrs[I];
        for (int i = 0; i < I; i++) pthread_create(&a_thrs[i], NULL, agregate_thread, NULL);
        for (int i = 0; i < I; i++) pthread_join(a_thrs[i], NULL);
    }
}

int main() {
    fill_memory((void *) B, A * 1024 * 1024);
    fill_file(E * 1024 * 1024);
    return 0;
}

/*
https://stackoverflow.com/questions/51552414/how-to-disable-c-error-checking-in-vs-code/51552563
https://habr.com/ru/post/326138/
http://tetraquark.ru/archives/47
https://stackoverflow.com/questions/22008229/bitwise-or-in-linux-open-flags
https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/
https://stackoverflow.com/questions/11414191/what-are-the-main-differences-between-fwrite-and-write
https://stackoverflow.com/questions/1658530/load-numbers-from-text-file-in-c
https://stackoverflow.com/questions/35403892/creating-threads-in-a-loop
https://code.visualstudio.com/docs/cpp/config-wsl
https://it.wikireading.ru/34319
https://habr.com/ru/company/ruvds/blog/523872/
http://ru.manpages.org/fcntl/2
https://www.opennet.ru/docs/RUS/zlp/005.html
https://pubs.opengroup.org/onlinepubs/007908775/xsh/fileno.html
http://www.codenet.ru/progr/cpp/spru/open.php
https://www.tune-it.ru/documents/10136/828092/dtrace_stap_book_b10a.pdf/66cae909-5677-40d2-a306-ca265392d142
https://eax.me/systemtap/
https://www.thegeekstuff.com/2011/11/strace-examples/
*/