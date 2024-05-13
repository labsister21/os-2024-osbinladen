#ifndef SHELL_CMD_H
#define SHELL_CMD

#include <stdint.h>
#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"
#include "header/driver/graphics.h"
#include "header/user/user-buffer.h"
#include "header/user/user-shell.h"
#include "header/user/user-syscall.h"

#define PROCESS_NAME_LENGTH_MAX                     32
#define PROCESS_COUNT_MAX                           16
#define PROCESS_CREATE_SUCCESS                      0
#define PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED    1
#define PROCESS_CREATE_FAIL_INVALID_ENTRYPOINT      2
#define PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY       3
#define PROCESS_CREATE_FAIL_FS_READ_FAILURE         4

extern struct ShellState main_state;

typedef enum PROCESS_STATE {
    TERMINATED,
    RUNNING,
    WAITING,
    NEW,
    READY
} PROCESS_STATE;

typedef struct {
    uint32_t pid;                      
    char process_name[PROCESS_NAME_LENGTH_MAX];        
    int priority;   
    PROCESS_STATE state;               
} ProcessMetadata;

void print_path_from_root(int cluster_number, uint16_t Color);
int get_final_parent_cluster(char* path, int current_folder_cluster);
int get_deep_folder_cluster (char* path, int current_folder_cluster);
void parse_filename(char* fileName, int nameLength, char* name, char* ext);
void get_only_filename(char* filePath, int nameLength, char* name, char* ext);

/*
* cd	- Mengganti current working directory
* goal berisikan nama folder tujuan
*
* return 0: operasi berhasil
* return 1: Bukan sebuah folder
* return 2: Folder tidak ditemukan
* return 3: error lain
*/
int cd(char* goal, int goalLength);

/*
* ls	- Menuliskan isi current working directory
*
* return 0: operasi berhasil
* return 3: error lain
*/
int ls();


void print_cur_dir(struct FAT32DirectoryTable dir_table);


/*
* mkdir	- Membuat sebuah folder kosong baru pada current working directory
* goal berisikan nama folder yang ingin dibuat
*
* return 0: operasi berhasil
* return 1: directory penuh
* return 3: error lain
*/
int mkdir(char *goal, int goalLength);

/*
* cat	- Menuliskan sebuah file sebagai text file ke layar (Gunakan format LF newline)
* goal berisikan nama file tujuan
*
* return 0: operasi berhasil
* return 1: Bukan sebuah file
* return 2: File tidak ditemukan
* return 3: error lain
*/
int cat(char* goal, int goalLength);

/*
* cp	- Mengcopy suatu file (Folder menjadi bonus)
* goal berisikan nama file
*
* return 0: operasi berhasil
* return 1: Bukan sebuah file
* return 2: File tidak ditemukan
* return 3: error lain
*/
int cp(char* goal, int goalLength, char* dest, int destLength);

/*
* rm    - Menghapus suatu file (Folder menjadi bonus)
* goal berisikan nama file
*
* return 0: operasi berhasil
* return 1: Bukan sebuah file
* return 2: File tidak ditemukan
* return 3: error lain
*/
int rm(char* goal, int goalLength);

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
int mv(char* goal1, int goal1Length, char* goal2, int goal2Length);

/*
* find	- Mencari file/folder dengan nama yang sama diseluruh file system
* goal berisikan nama file/folder yang dicari
*
* return 0: operasi berhasil
* return 1: tujuan tidak ditemukan
* return 3: error lain
*/
int find(char* goal, int goalLength);

void dfs_find(uint32_t cluster_number, char* goal, int goalLength, char* path, char paths[64][256], int* pathCount);

/*
 * exec - Membuat proses baru, dengan executable ada pada filePath
 *
 *  return 0 : operasi berhasil
 *  return 1 : file tidak ditemukan
 *  return 2 : bukan file
 *  return 3 : anomali
 */
int exec(char* filePath);

/*
 *  ps - Menunjukkan informasi proses yang sedang berjalan
 *
 */
void ps();

/*
 *  kill - Menghentikan proses dengan process id = pid
 *
 *  return 0 : operasi berhasil
 *  return 1 : pid tidak ditemukan
 *  return 3 : anomali
 */
int kill(uint32_t pid);

void clear();
int rmr(char* goal, int goalLength);


#endif