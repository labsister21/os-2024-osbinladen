#include "header/user/shell_cmd.h"

void print_path_from_root(int cluster_number, uint16_t Color){
    if (cluster_number == ROOT_CLUSTER_NUMBER){
        printToScreen("~$", Color);
        return;
    }
    struct FAT32DirectoryTable dir_table;
    syscall(10, cluster_number, (uint32_t)&dir_table, 0);
    uint32_t new_cluster = (uint16_t)dir_table.table[1].cluster_low + ((uint32_t)dir_table.table[1].cluster_high << 16);
    print_path_from_root(new_cluster, Color);
    printToScreen(dir_table.table[0].name, Color);
    printToScreen("/", Color);
    printToScreen("$",Color);
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
    
    for(int i = 1; i < 64; i++){
        if (isStrEqual(parent, dir_table.table[i].name) && dir_table.table[i].attribute == ATTR_SUBDIRECTORY){
            if (dir_table.table[i].attribute != ATTR_SUBDIRECTORY && isStrEqual(dir_table.table[i].ext, '\0\0\0')){
                return -1;
            }
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
        for (int j = 0; j < 3; j++) {
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
    int new_cluster = get_deep_folder_cluster(goal, main_state.cwd_cluster_number);
    if (new_cluster == -1){
        printToScreen("\n", color_to_int(WHITE));
        printToScreen(goal, color_to_int(GREEN));
        printToScreen(" Bukanlah sebuah folder", color_to_int(GREEN));
        return 1;
    }
    else if (new_cluster != 0){
        main_state.cwd_cluster_number = new_cluster;
        return 0;
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
        if(isStrEqual(dir_table.table[i].name,"..")){
            continue;
        }
        if(dir_table.table[i].user_attribute == UATTR_NOT_EMPTY){
                if(dir_table.table[i].attribute == ATTR_SUBDIRECTORY){
                    printToScreen(dir_table.table[i].name, color_to_int(LIGHT_BLUE));
               
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
* return 1: folder sudah ada
* return 2: error lain
*/
int mkdir(char *goal, int goalLength){

    struct ClusterBuffer temp = {0};
    struct FAT32DriverRequest req = {
        .buf                   = &temp,
        .name                  = "\0\0\0\0\0\0\0\0",
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
        strcat(name, "Folder \'");
        strcat(name, goal);
        strcat(name, "\' has been made");
        
        printToScreen("\n", color_to_int(WHITE));
        printToScreen(name, color_to_int(WHITE));
        printToScreen("\n", color_to_int(WHITE));
        printToScreen("\n", color_to_int(WHITE));
        return 0;
    case 1:
        /* folder already exists */
        printToScreen("\n", 0xc);
        printToScreen("A folder with the same name already exists", 0xa);
        return 1;
    }

    return 2;
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

    int retcode = 0;
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
* return 1: folder tidak kosong
* return 2: File/folder tidak ditemukan
* return 3: error lain
*/
int rm(char* goal, int goalLength){
    struct ClusterBuffer res = {0};
    struct FAT32DriverRequest req = {
        .buf                   = &res,
        .name                  = "\0\0\0\0\0\0\0\0",
        .ext                   = "\0\0\0",
        .parent_cluster_number = main_state.cwd_cluster_number,
        .buffer_size           = 512,
    };

    parse_filename(goal, strlen(goal), req.name, req.ext);

    if (req.ext[0] == '\0') {
        req.buffer_size = 0;
    }

    uint8_t retcode;

    if (req.name[0] == '\0') {
        retcode = 1;
    } else {
        syscall(3, (uint32_t) &req, (uint32_t) &retcode, 0);
    }

    char name[6144] = {0};

    switch (retcode)
    {
    case 0:
        /* success */

        strcat(name, "File/folder \'");
        strcat(name, goal);
        strcat(name, "\' deleted");
    
        printToScreen("\n", color_to_int(WHITE));
        printToScreen(name, color_to_int(WHITE));
        printToScreen("\n", color_to_int(WHITE));
        return 0;
    case 1:
        /* File not found */
        strcat(name, "File/folder \'");
        strcat(name, goal);
        strcat(name, "\' not found");
        printToScreen("\n", color_to_int(WHITE));
        printToScreen(name, color_to_int(WHITE));
        printToScreen("\n", color_to_int(WHITE));
        return 2;
    case 2:
        /* File not found */
        strcat(name, "Folder \'");
        strcat(name, goal);
        strcat(name, "\' not empty");
        printToScreen("\n", color_to_int(WHITE));
        printToScreen(name, color_to_int(WHITE));
        printToScreen("\n", color_to_int(WHITE));
        return 1;
    default:
        return 3;
    }
    return 3;
}

/*
* rmr    - Menghapus suatu file atau folder (dan menghapus semua isi folder )
* goal berisikan nama file
*
* return 0: operasi berhasil
* return 2: File/folder tidak ditemukan
* return 3: error lain
*/
int rmr(char* goal, int goalLength){
    struct ClusterBuffer res = {0};
    struct FAT32DriverRequest req = {
        .buf                   = &res,
        .name                  = "\0\0\0\0\0\0\0\0",
        .ext                   = "\0\0\0",
        .parent_cluster_number = main_state.cwd_cluster_number,
        .buffer_size           = 512,
    };

    parse_filename(goal, strlen(goal), req.name, req.ext);

    if (req.ext[0] == '\0') {
        req.buffer_size = 0;
    }

    uint8_t retcode;

    if (req.name[0] == '\0') {
        retcode = 1;
    } else {
        syscall(9, (uint32_t) &req, (uint32_t) &retcode, 0);
    }

    char name[6144] = {0};

    switch (retcode)
    {
    case 0:
        /* success */

        strcat(name, "File/folder \'");
        strcat(name, goal);
        strcat(name, "\' deleted");
    
        printToScreen("\n", color_to_int(WHITE));
        printToScreen(name, color_to_int(WHITE));
        printToScreen("\n", color_to_int(WHITE));
        return 0;
    case 1:
        /* File not found */
        strcat(name, "File/folder \'");
        strcat(name, goal);
        strcat(name, "\' not found");
        printToScreen("\n", color_to_int(WHITE));
        printToScreen(name, color_to_int(WHITE));
        printToScreen("\n", color_to_int(WHITE));
        return 2;
    default:
        printToScreen("Anomaly in deleting",color_to_int(GREEN));
        return 3;
    }
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
    int source_cluster = get_final_parent_cluster(goal1, main_state.cwd_cluster_number);
    int dest_cluster = get_final_parent_cluster(goal2, main_state.cwd_cluster_number);
    char source_name[8], dest_name[8], source_ext[3], dest_ext[3];
    get_only_filename(goal1, goal1Length, source_name, source_ext);
    get_only_filename(goal2, goal2Length, dest_name, dest_ext);
    if(source_cluster == dest_cluster){
        if(isStrEqual(source_name, dest_name) && isStrEqual(source_ext, dest_ext)){
            printToScreen("\n'", color_to_int(GREEN));
            printToScreen(goal1, color_to_int(GREEN));
            printToScreen("' dan '", color_to_int(GREEN));
            printToScreen(goal2, color_to_int(GREEN));
            printToScreen("'merupakan file yang sama\n", color_to_int(GREEN));    
            return 4;
        }
    }
    if (source_cluster == 0){
        printToScreen("\nFile tidak ditemukan\n", color_to_int(GREEN));
        return 2;
    }
    else if (dest_cluster == 0){
        printToScreen("\nFolder tujuan tidak ditemukan\n", color_to_int(GREEN));
        return 3;
    }

    /* Pembacaan source file*/

    struct ClusterBuffer temp = {0};
    struct FAT32DriverRequest req = {
        .buf                   = &temp,
        .name                  = {0},
        .ext                   = {0},
        .parent_cluster_number = source_cluster,
        .buffer_size           = CLUSTER_SIZE,
    };
    memcpy(req.name, source_name, 8);
    memcpy(req.ext, source_ext, 3);

    int retcode;
    syscall(0, (uint32_t) &req, (uint32_t) &retcode, 0);

    switch (retcode){
        case 1 :
            printToScreen("\n", color_to_int(WHITE));
            printToScreen(goal1, color_to_int(GREEN));
            printToScreen(" bukan sebuah file\n", color_to_int(GREEN));
            return 1;
        case 2 :
            printToScreen("\nfile ", color_to_int(GREEN));
            printToScreen(goal1, color_to_int(GREEN));
            printToScreen(" tidak ditemukan\n", color_to_int(GREEN));
            return 2;
        case -1 :
            printToScreen("\nterjadi anomali pada pembacaan file ", color_to_int(GREEN));
            printToScreen(goal1, color_to_int(GREEN));
            printToScreen("\n", color_to_int(GREEN));
            return 3;
        case 0:
            break;
        default:
            return 3;
    }

    /*Pembuatan copy dari source file*/
    
    req.parent_cluster_number = dest_cluster;
    memcpy(req.name, dest_name, 8);
    memcpy(req.ext, dest_ext, 3);
    syscall(2, (uint32_t) &req, (uint32_t) &retcode, 0);

    switch (retcode){
        case 1 :
            printToScreen("\nFile dengan nama ", color_to_int(GREEN));
            printToScreen(goal2, color_to_int(GREEN));
            printToScreen(" sudah ada\n", color_to_int(GREEN));
            return 1;
        case 2 :
            printToScreen("\nanomali\n", color_to_int(GREEN));
            return 2;
        case -1 :
            printToScreen("\nterjadi anomali pada pembuatan file ", color_to_int(GREEN));
            printToScreen(goal2, color_to_int(GREEN));
            printToScreen("\n", color_to_int(GREEN));
            return 3;
        case 0:
            break;
        default:
            return 3;
    }

    /* Penghapusan file lama*/
    req.parent_cluster_number = source_cluster;
    memcpy(req.name, source_name, 8);
    memcpy(req.ext, source_ext, 3);
    syscall(3, (uint32_t) &req, (uint32_t) &retcode, 0);

    switch (retcode){
        case 0:
            printToScreen("\nMove berhasil\n", color_to_int(GREEN));
            return 0;
        default:
            printToScreen("\nAnomali saat penghapusan file lama", color_to_int(GREEN));
            return 3;
    }

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
    char paths[64][256]; // Array to store the paths
    int pathCount = 0; // Number of paths found
    char path[256] = "\nroot/"; // Start with the root directory
    dfs_find(ROOT_CLUSTER_NUMBER, goal, goalLength, path, paths, &pathCount);
    if(pathCount == 0){
        printToScreen("\n\nNo such file or directory\n",color_to_int(WHITE));
        return 1;
    }
    else{
        for(int i = 0; i < pathCount; i++){
            printToScreen(paths[i], color_to_int(WHITE)); // Print each path
            printToScreen("\n", color_to_int(WHITE));
        }
        return 0;
    }
}

void dfs_find(uint32_t cluster_number, char* goal, int goalLength, char* path, char paths[64][256], int* pathCount){
    if(*pathCount >= 64){
        return; 
    }
    struct FAT32DirectoryTable dir_table;
    syscall(10, cluster_number, (uint32_t)&dir_table, 0);
    for(int i = 1;i<64;i++){
        if(isStrEqual(dir_table.table[i].name,"..")){
            continue;
        }
        if(dir_table.table[i].user_attribute == UATTR_NOT_EMPTY){
            char subpath[256];
            if(dir_table.table[i].attribute == ATTR_SUBDIRECTORY){
                char entryname[9]; 
                memcpy(entryname,dir_table.table[i].name,8);
                entryname[9] = '\0'; 
                memcpy(subpath, path, strlen(path) + 1); 
                strcat(subpath, entryname); 
                if(isStrEqual(entryname,goal)){
                    memcpy(paths[*pathCount], subpath, strlen(subpath) + 1); 
                    (*pathCount)++;
                }
            }
            else{
                char entryname[13];
                memcpy(entryname,dir_table.table[i].name,8);
                if(dir_table.table[i].ext[0] == '\0'){
                    entryname[8] = '\0';
                }
                else{
                    strcat(entryname,".");
                    strcat(entryname,dir_table.table[i].ext);
                    entryname[13] = '\0';
                }
                memcpy(subpath, path, strlen(path) + 1); 
                strcat(subpath, entryname); 
                if(isStrEqual(entryname,goal)){
                    memcpy(paths[*pathCount], subpath, strlen(subpath) + 1); 
                    (*pathCount)++;
                }
            }
            if(dir_table.table[i].attribute == ATTR_SUBDIRECTORY){
                strcat(subpath, "/"); 
                uint32_t cluster_num = dir_table.table[i].cluster_high << 16 | dir_table.table[i].cluster_low;
                dfs_find(cluster_num, goal, goalLength, subpath, paths, pathCount);
            }
        }
    }
}


void clear(){

    memset(main_state.userBuffer, 0, TEXT_HEIGHT*TEXT_WIDTH);
    syscall(11,0,0,0);
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