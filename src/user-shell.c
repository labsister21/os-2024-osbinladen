#include <stdint.h>
#include "header/filesystem/fat32.h"


uint32_t cwd_cluster_number = ROOT_CLUSTER_NUMBER;


void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

int main(void) {
    struct ClusterBuffer      cl[2]   = {0};
    struct FAT32DriverRequest request = {
        .buf                   = &cl,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = CLUSTER_SIZE,
    };
    int32_t retcode;
    syscall(0, (uint32_t) &request, (uint32_t) &retcode, 0);
    if (retcode == 0)
        syscall(6, (uint32_t) "owo\n", 4, 0xF);

    char buf;
    syscall(7, 0, 0, 0);
    while (true) {
        syscall(4, (uint32_t) &buf, 0, 0);
        syscall(5, (uint32_t) buf, 0xF, 0);
        syscall(8, 0, 0, 0);
    }

    return 0;
}

/*
* cd	- Mengganti current working directory
* goal berisikan nama folder tujuan
*
* return 0: operasi berhasil
* return 1: Bukan sebuah folder
* return 2: Folder tidak ditemukan
* return 3: error lain
*/
int cd(char* goal, int goalLength){
    return 3;
}

/*
* ls	- Menuliskan isi current working directory
*
* return 0: operasi berhasil
* return 3: error lain
*/
int ls(){
    char req_buf[BLOCK_SIZE*4];
    // struct FAT32DirectoryTable dir_table;
    syscall(9, (uint32_t) req_buf, cwd_cluster_number, 0);
    syscall(6, (uint32_t) req_buf, req_buf, 0);

}

/*
* mkdir	- Membuat sebuah folder kosong baru pada current working directory
* goal berisikan nama folder yang ingin dibuat
*
* return 0: operasi berhasil
* return 1: directory penuh
* return 3: error lain
*/
int mkdir(char* goal, int goalLength){
    return 3;
}

/*
* cat	- Menuliskan sebuah file sebagai text file ke layar (Gunakan format LF newline)
* goal berisikan nama file tujuan
*
* return 0: operasi berhasil
* return 1: Bukan sebuah file
* return 2: File tidak ditemukan
* return 3: error lain
*/
int cat(char* goal, int goalLength){
    return 3;
}

/*
* cp	- Mengcopy suatu file (Folder menjadi bonus)
* goal berisikan nama file
*
* return 0: operasi berhasil
* return 1: Bukan sebuah file
* return 2: File tidak ditemukan
* return 3: error lain
*/
int cp(char* goal, int goalLength){
    return 3;
}

/*
* rm    - Menghapus suatu file (Folder menjadi bonus)
* goal berisikan nama file
*
* return 0: operasi berhasil
* return 1: Bukan sebuah file
* return 2: File tidak ditemukan
* return 3: error lain
*/
int rm(char* goal, int goalLength){
    return 3;
}

/*
* mv	- Memindah dan merename lokasi file/folder
* goal1 berisikan nama file tujuan
* goal2 berisikan tujuan folder dan nama file baru setelah dipindah
*
* return 0: operasi berhasil
* return 1: Bukan sebuah file
* return 2: file tidak ditemukan
* return 3: folder tujuan tidak ada
* return 4: error lain
*/
int mv(char* goal1, int goal1Length, char* goal2, int goal2Length){
    return 3;
}

/*
* find	- Mencari file/folder dengan nama yang sama diseluruh file system
* goal berisikan nama file/folder yang dicari
*
* return 0: operasi berhasil
* return 1: tujuan tidak ditemukan
* return 3: error lain
*/
int find(char* goal, int goalLength){
    return 3;
}

