#include "header/user-shell.h"

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

uint16_t color_to_int(Color16 color){
    return color.red + (color.green << 5) + (color.blue << 11);
}

Color16 int_to_color(uint16_t color16){
    return (Color16){color16 & 0b11111, (color16 >> 5) & 0b111111, (color16 >> 11) & 0b11111};
}

char userBuffer[TEXT_HEIGHT*TEXT_WIDTH];
int userBufferPos;
int readPointer;
char currentWord[64];
int cwd_cluster_number;

void reset_user_buffer(){
    memset((void*)userBuffer, 0, TEXT_HEIGHT*TEXT_WIDTH);
    userBufferPos = 0;
    readPointer = 0;
}

bool isBlank(char c){
    return c != ' ' && c != '\n' && c != 0x0 && c != '\t';
}

void get_next_word(){
    memset(currentWord, 0, 64);
    int charPointer = 0;
    while (!isBlank(userBuffer[readPointer])){
        currentWord[charPointer] = userBuffer[readPointer];
        charPointer++;
    }
}

void cmdHandler(){
    syscall(5, (uint32_t) '\n', color_to_int(WHITE), 0);
}

void handle_newline(){
  userBuffer[userBufferPos] = ' ';
  userBufferPos++;
  get_next_word();
  printToScreen(currentWord, color_to_int(GREEN));
  cmdHandler();
  syscall(6, (uint32_t) "ha", 2, color_to_int(GREEN));
}

void handle_tab(){
    userBuffer[userBufferPos] = '\t';
    userBufferPos++;  
    syscall(5, (uint32_t) userBuffer[userBufferPos], color_to_int(WHITE), 0);
}

void handle_backspace(){
    if (userBufferPos == 0){return;}
    userBuffer[userBufferPos - 1] = 0;
    userBufferPos--;
    syscall(5, (uint32_t) '\b', color_to_int(WHITE), 0);
}

void handle_others(char key){
  if (key >= 32 && key <= 126){
    userBuffer[userBufferPos] = key;
    userBufferPos++;
    syscall(5, (uint32_t) key, color_to_int(WHITE), 0);
  }
}

void inputChar(char c){
    if (c != 0){
      switch(c){
        case '\n':
          handle_newline(); 
          break;   
        case '\t':
          handle_tab();
          break;
        case '\b':
          handle_backspace();
          break;
        default:
          handle_others(c);
          break;
      }
    }
}

int mkdir(char *goal, int goalLength);
int ls();

int main(void) {
    struct ClusterBuffer      cl[4]   = {0};
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
        syscall(6, (uint32_t) "owo\n", 4, color_to_int(GREEN));

    char buffen;
    reset_user_buffer();
    syscall(7, 0, 0, 0);
    mkdir("test", 4);
    mkdir("crott", 5);
    // printToScreen("cek",3);
    ls();
    syscall(6, (uint32_t) "ha", 2, color_to_int(GREEN));
    while (true) {
        syscall(8, 0, 0, 0);
        syscall(4, (uint32_t) &buffen, 0, 0);
        inputChar(buffen);
    }

    return 0;
}


void printToScreen(char* msg, uint16_t color){
    if(strlen(msg) == 2 && msg[0] == '\' ' && msg[1] == 'n'){
         syscall(6, (uint32_t) "\n", 1 , color);
    }
    syscall(6, (uint32_t) msg, (uint32_t) strlen(msg) , color);
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
    // print_cur_dir(cwd_cluster_number);
    struct FAT32DirectoryTable dir_table;
    syscall(10, cwd_cluster_number, &dir_table, 0);
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
        .parent_cluster_number = cwd_cluster_number,
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

char userBuffer[TEXT_HEIGHT*TEXT_WIDTH] = {0};
int userBufferPos = 0;
int readPointer = 0;
char currentWord[64];
int cwd_cluster_number = 0x2;