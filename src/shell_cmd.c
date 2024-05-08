#include "header/user/shell_cmd.h"

void print_path_from_root(int cluster_number, uint16_t Color){
    if (cluster_number == ROOT_CLUSTER_NUMBER){
        printToScreen("root/", Color);
        return;
    }
    struct FAT32DirectoryTable dir_table;
    syscall(10, cluster_number, (uint32_t)&dir_table, 0);
    uint32_t new_cluster = (uint16_t)dir_table.table[1].cluster_low + ((uint32_t)dir_table.table[1].cluster_high << 16);
    print_path_from_root(new_cluster, Color);
    printToScreen(dir_table.table[0].name, Color);
    printToScreen("/", Color);
}

void get_parent(char* path, char* parent){
    int i = 0;
    while(path[i] != '\\' && path[i] != '/' && path[i] != '\0'){
        parent[i] = path[i];
        i++;
    }
}

int get_final_parent_cluster(char* path, int current_folder_cluster){
    char parent[10] = {0};
    get_parent(path, parent);
    if (isStrEqual(parent, path)){
        return current_folder_cluster;
    }
    struct FAT32DirectoryTable dir_table;
    syscall(10, current_folder_cluster, (uint32_t)&dir_table, 0);
    
    for(int i = 0; i < 64; i++){
        if (isStrEqual(parent, dir_table.table[i].name) && dir_table.table[i].attribute == ATTR_SUBDIRECTORY){
            return get_final_parent_cluster(path + strlen(parent) + 1, 
                                (uint16_t) dir_table.table[i].cluster_low + ((uint32_t) dir_table.table[i].cluster_high << 16));
        }
    }
    return 0;
}

int get_deep_folder_cluster (char* path, int current_folder_cluster){

    if (path[0] == '\0'){
        return current_folder_cluster;
    }

    char parent[10] = {0};
    get_parent(path, parent);

    struct FAT32DirectoryTable dir_table;
    syscall(10, current_folder_cluster, (uint32_t)&dir_table, 0);
    
    for(int i = 0; i < 64; i++){
        if (isStrEqual(parent, dir_table.table[i].name) && dir_table.table[i].attribute == ATTR_SUBDIRECTORY){
            if (isStrEqual(path, parent)){
                return (uint16_t) dir_table.table[i].cluster_low + ((uint32_t) dir_table.table[i].cluster_high << 16);
            }
            else{
                return get_deep_folder_cluster(path + strlen(parent) + 1, 
                        (uint16_t) dir_table.table[i].cluster_low + ((uint32_t) dir_table.table[i].cluster_high << 16));
            }
        }
    }

    return 0;
}

void parse_filename(char* file, int nameLength, char* nameDest, char* extDest){
    int i = 0; 
    int dot_found = 0;

    while (i < nameLength && file[i] != '\0') {
        if (file[i] == '.') {
            dot_found = 1;
            break;
        } else {
            nameDest[i] = file[i];
        }
        i++;
    }

    if (dot_found) {
        i++;
        int ext_index = 0;
        while (i < nameLength && file[i] != '\0') {
            extDest[ext_index] = file[i];
            ext_index++;
            i++;
        }
    }
}

void get_only_filename(char* filePath, int nameLength, char* name, char* ext){
    char currentCandidate[12] = {0};
    int i = 0;
    int candidateIdx = 0;
    while(filePath[i] != '\0'){
        memset(currentCandidate, 0x0, 10);
        while(filePath[i] != '\\' && filePath[i] != '/' && filePath[i] != 0x0){
            currentCandidate[candidateIdx] = filePath[i];
            i++;
            candidateIdx++;
        }
        i++;
        candidateIdx = 0;
    }
    parse_filename(currentCandidate, strlen(currentCandidate), name, ext);
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

    struct FAT32DirectoryTable dir_table;
    syscall(10, main_state.cwd_cluster_number, (uint32_t)&dir_table, 0);
    for(int i = 1; i < 64; i++){
        if(dir_table.table[i].user_attribute == UATTR_NOT_EMPTY){
            if (memcmp(dir_table.table[i].name, goal, goalLength) == 0){
                if(dir_table.table[i].attribute != ATTR_SUBDIRECTORY){ // If not a folder
                    printToScreen("\n", color_to_int(WHITE));
                    printToScreen(goal, color_to_int(GREEN));
                    printToScreen(" Bukanlah sebuah folder", color_to_int(GREEN));
                    return 1;
                }
                else{
                    main_state.cwd_cluster_number = (uint16_t) dir_table.table[i].cluster_low + ((uint32_t) dir_table.table[i].cluster_high << 16);
                    return 0;
                }
            }
        }
    }
    printToScreen("\n", color_to_int(WHITE));
    printToScreen(main_state.currentWord, color_to_int(GREEN));
    printToScreen(" tidak ditemukan", color_to_int(GREEN));
    return 2;
    return 3;
}

/*
* ls	- Menuliskan isi current working directory
*
* return 0: operasi berhasil
* return 3: error lain
*/
int ls(){
    // print_cur_dir(main_state.cwd_cluster_number);
    struct FAT32DirectoryTable dir_table;
    syscall(10, main_state.cwd_cluster_number, (uint32_t)&dir_table, 0);
    print_cur_dir(dir_table);
    return 0;
}


void print_cur_dir(struct FAT32DirectoryTable dir_table){
   
    printToScreen("\n", color_to_int(WHITE));

    for(int i = 1; i < 64; i++){
        if(dir_table.table[i].user_attribute == UATTR_NOT_EMPTY){
      
                if(dir_table.table[i].attribute == ATTR_SUBDIRECTORY){
                    printToScreen(dir_table.table[i].name, color_to_int(BLUE));
               
                    printToScreen("    ", color_to_int(WHITE));
                }
                else{
                    printToScreen(dir_table.table[i].name, color_to_int(WHITE));
                    printToScreen(dir_table.table[i].ext, color_to_int(WHITE));
                    printToScreen("    ", color_to_int(WHITE));

                }
            }
     }
    printToScreen("\n", color_to_int(WHITE));

}


/*
* mkdir	- Membuat sebuah folder kosong baru pada current working directory
* goal berisikan nama folder yang ingin dibuat
*
* return 0: operasi berhasil
* return 1: directory penuh
* return 3: error lain
*/
int mkdir(char *goal, int goalLength){

    struct ClusterBuffer temp = {0};
    struct FAT32DriverRequest req = {
        .buf                   = &temp,
        .name                  = "\0\0\0\0",
        .ext                   = "\0\0\0",
        .parent_cluster_number = main_state.cwd_cluster_number,
        .buffer_size           = 0,
    };

    if(memcmp(goal, "", 0)){
        return 3;
    }

    memcpy(req.name, goal, goalLength);

    uint8_t retcode;
    syscall(2, (uint32_t) &req, (uint32_t) &retcode, 0);
    char name[6144] = {0};
    switch (retcode)
    {
    case 0:
        /* folder doesn't exist, thus make folder */
        // char temp[8];
        // strncpy(temp, goal, 8);
        strcat(name, "Folder \'");
        strcat(name, goal);
        strcat(name, "\' has been made");
        
        printToScreen("\n", color_to_int(WHITE));
        printToScreen(name, color_to_int(WHITE));
        printToScreen("\n", color_to_int(WHITE));
        printToScreen("\n", color_to_int(WHITE));
        break;
    case 1:
        /* folder already exists */
        printToScreen("\n", 0xc);
        printToScreen("A folder with the same name already exists", 0xa);
        break;
    }
    return 0;
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
    struct ClusterBuffer temp[4] = {0};
    struct FAT32DriverRequest req = {
        .buf                   = &temp,
        .name                  = {0},
        .ext                   = {0},
        .parent_cluster_number = main_state.cwd_cluster_number,
        .buffer_size           = 4*CLUSTER_SIZE,
    };

    if(goal[0] == '\0'){
        return 3;
    }

    parse_filename(goal, goalLength, req.name, req.ext);

    int retcode;
    syscall(0, (uint32_t) &req, (uint32_t) &retcode, 0);
    
    switch (retcode){
        case 1 :
            printToScreen("\n", color_to_int(WHITE));
            printToScreen(goal, color_to_int(GREEN));
            printToScreen(" bukan sebuah file\n", color_to_int(GREEN));
            return 1;
        case 2 :
            printToScreen("\nfile ", color_to_int(GREEN));
            printToScreen(goal, color_to_int(GREEN));
            printToScreen(" tidak ditemukan\n", color_to_int(GREEN));
            return 2;
        case -1 :
            printToScreen("\nterjadi anomali pada pembacaan file ", color_to_int(GREEN));
            printToScreen(goal, color_to_int(GREEN));
            printToScreen("\n", color_to_int(GREEN));
            return 3;
        case 0:
            printToScreen("\n", color_to_int(WHITE));
            printToScreen(goal, color_to_int(GREEN));
            printToScreen(":\n", color_to_int(GREEN));
            printToScreen((char *)temp->buf, color_to_int(WHITE));
            printToScreen("\n", color_to_int(WHITE));
            return 0;
        default:
            return 3;
    }
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
int cp(char* goal, int goalLength, char* dest, int destLength){
    int goal_cluster = get_final_parent_cluster(goal, main_state.cwd_cluster_number);
    int dest_cluster = get_final_parent_cluster(dest, main_state.cwd_cluster_number);

    if (goal_cluster == 0){
        printToScreen("\nFile tidak ditemukan\n", color_to_int(GREEN));
        return 2;
    }
    else if (dest_cluster == 0){
        printToScreen("\nFolder tujuan tidak ditemukan\n", color_to_int(GREEN));
        return 3;
    }

    struct ClusterBuffer temp = {0};
    struct FAT32DriverRequest req = {
        .buf                   = &temp,
        .name                  = {0},
        .ext                   = {0},
        .parent_cluster_number = goal_cluster,
        .buffer_size           = CLUSTER_SIZE,
    };

    get_only_filename(goal, goalLength, req.name, req.ext);

    int retcode;
    syscall(0, (uint32_t) &req, (uint32_t) &retcode, 0);

    switch (retcode){
        case 1 :
            printToScreen("\n", color_to_int(WHITE));
            printToScreen(goal, color_to_int(GREEN));
            printToScreen(" bukan sebuah file\n", color_to_int(GREEN));
            return 1;
        case 2 :
            printToScreen("\nfile ", color_to_int(GREEN));
            printToScreen(goal, color_to_int(GREEN));
            printToScreen(" tidak ditemukan\n", color_to_int(GREEN));
            return 2;
        case -1 :
            printToScreen("\nterjadi anomali pada pembacaan file ", color_to_int(GREEN));
            printToScreen(goal, color_to_int(GREEN));
            printToScreen("\n", color_to_int(GREEN));
            return 3;
        case 0:
            break;
        default:
            return 3;
    }

    req.parent_cluster_number = dest_cluster;
    memset(req.name, 0, 8);
    memset(req.ext, 0, 3);
    get_only_filename(dest, destLength, req.name, req.ext);
    syscall(2, (uint32_t) &req, (uint32_t) &retcode, 0);

    switch (retcode){
        case 1 :
            printToScreen("\nFile dengan nama ", color_to_int(WHITE));
            printToScreen(goal, color_to_int(GREEN));
            printToScreen(" sudah ada\n", color_to_int(GREEN));
            return 1;
        case 2 :
            printToScreen("\nanomali\n", color_to_int(GREEN));
            return 2;
        case -1 :
            printToScreen("\nterjadi anomali pada pembacaan file ", color_to_int(GREEN));
            printToScreen(goal, color_to_int(GREEN));
            printToScreen("\n", color_to_int(GREEN));
            return 3;
        case 0:
            printToScreen("\nCopy berhasil\n", color_to_int(GREEN));
            return 0;
        default:
            return 3;
    }

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

// ============================================ BATAS SUCI ===========================================
void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}