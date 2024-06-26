#include "header/user/user-buffer.h"
#include "header/user/shell_cmd.h"

uint16_t color_to_int(Color16 color){
    return color.red + (color.green << 5) + (color.blue << 11);
}

Color16 int_to_color(uint16_t color16){
    return (Color16){color16 & 0b11111, (color16 >> 5) & 0b111111, (color16 >> 11) & 0b11111};
}

void printToScreen(char* msg, uint16_t color){
    syscall(6, (uint32_t) msg, (uint32_t) strlen(msg) , color);
}

void reset_user_buffer(){
    memset((void*)main_state.userBuffer, 0, TEXT_HEIGHT*TEXT_WIDTH);
    main_state.userBufferPos = 0;
    main_state.readPointer = 0;
}

bool isBlank(char c){
    return c == ' ' || c == '\n' || c == 0x0 || c == '\t';
}

void get_next_word(){
    memset(main_state.currentWord, 0, 64);
    int charPointer = 0;

    while(isBlank(main_state.userBuffer[main_state.readPointer]) && main_state.userBuffer[main_state.readPointer] != 0x0){
        main_state.readPointer++;
    }

    while (!isBlank(main_state.userBuffer[main_state.readPointer])){
        main_state.currentWord[charPointer] = main_state.userBuffer[main_state.readPointer];
        charPointer++;
        main_state.readPointer++;
    }
}

void cmdHandler(){
    while(main_state.currentWord[0] != 0x0){
        if (isStrEqual(main_state.currentWord, "ls")){
            ls();
        }
        else if (isStrEqual(main_state.currentWord, "mkdir")){
            get_next_word();
            mkdir(main_state.currentWord, strlen(main_state.currentWord));
        }
        else if (isStrEqual(main_state.currentWord, "cat")){
            get_next_word();
            cat(main_state.currentWord, strlen(main_state.currentWord));
        }
        else if (isStrEqual(main_state.currentWord, "cd")){
            get_next_word();
            cd(main_state.currentWord, strlen(main_state.currentWord));
        }

        else if (isStrEqual(main_state.currentWord, "rm")){
            get_next_word();
            if(isStrEqual(main_state.currentWord, "-r")){
                get_next_word();
                rmr(main_state.currentWord, strlen(main_state.currentWord));
            }
            else{
                rm(main_state.currentWord, strlen(main_state.currentWord)); 
            }
        }

        else if (isStrEqual(main_state.currentWord, "cp")){
            char goal[64] = {0};
            get_next_word();
            memcpy(goal, main_state.currentWord, 64);
            get_next_word();
            cp(goal, strlen(goal), main_state.currentWord, strlen(main_state.currentWord));
        }
        else if (isStrEqual(main_state.currentWord, "mv")){
            char goal[64] = {0};
            get_next_word();
            memcpy(goal, main_state.currentWord, 64);
            get_next_word();
            mv(goal, strlen(goal), main_state.currentWord, strlen(main_state.currentWord));
        }
        else if (isStrEqual(main_state.currentWord, "rm")){
            get_next_word();
            rm(main_state.currentWord, strlen(main_state.currentWord));
        }
        else if (isStrEqual(main_state.currentWord, "debug")){
            ;
        }
        else if(isStrEqual(main_state.currentWord, "find")){
            get_next_word();
            find(main_state.currentWord, strlen(main_state.currentWord));
        }
        else if(isStrEqual(main_state.currentWord, "clear")){
            clear();
        }
        else if(isStrEqual(main_state.currentWord, "exec")){
            get_next_word();
            exec(main_state.currentWord);
        }
        else if(isStrEqual(main_state.currentWord, "ps")){
            ps();
        }
        else if(isStrEqual(main_state.currentWord, "kill")){
            get_next_word();
            kill(strToInt(main_state.currentWord));
        }
        else{
            printToScreen("\n", color_to_int(BLACK));
            printToScreen(main_state.currentWord, color_to_int(GREEN));
            printToScreen(" is not a valid command", color_to_int(GREEN));
        }
        get_next_word();
    }
    reset_user_buffer();
    syscall(5, (uint32_t) '\n', color_to_int(WHITE), 0);
}

void handle_newline(){
  main_state.userBuffer[main_state.userBufferPos] = '\n';
//   appendChar();
//   main_state.last_index_arrow = getArrayLength(main_state.stringBuffer,256);
  main_state.userBufferPos++;
  get_next_word();
  cmdHandler();
  syscall(6, (uint32_t) "user@OSBINLADEN: ",18 , color_to_int(GREEN));
  print_path_from_root(main_state.cwd_cluster_number, color_to_int(LIGHT_BLUE));
  printToScreen(" ", color_to_int(GREEN));
}

void handle_tab(){
    main_state.userBuffer[main_state.userBufferPos] = '\t';
    main_state.userBufferPos++;  
    syscall(5, (uint32_t) main_state.userBuffer[main_state.userBufferPos], color_to_int(WHITE), 0);
}

void handle_backspace(){
    if (main_state.userBufferPos == 0){return;}
    main_state.userBuffer[main_state.userBufferPos - 1] = 0;
    main_state.userBufferPos--;
    syscall(5, (uint32_t) '\b', color_to_int(WHITE), 0);
}

void handle_others(char key){
  if (key >= 32 && key <= 126){
    main_state.userBuffer[main_state.userBufferPos] = key;
    main_state.userBufferPos++;
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
        // case 0x11:
        //   handle_up_arrow();
        //   break;
        default:
          handle_others(c);
          break;
      }
    }
}

// void appendChar(){
//     int current_string = 0;
//     int current_char = 0;
//     memset(main_state.stringBuffer, 0, strlen(main_state.userBuffer));
//     for (int i = 0; i < strlen(main_state.userBuffer); i++) {
//         if (main_state.userBuffer[i] == '\n') {
//             if (current_char != 0) { 
//                 main_state.stringBuffer[current_string][current_char] = '\0'; 
//                 current_char = 0;
//             }
//         } else {
//             if (current_char < 256) {
//                 main_state.stringBuffer[current_string][current_char++] = main_state.userBuffer[i];
//             }
//         }
//     }
// }

int getArrayLength(char *arr[], int maxSize) {
    int count = 0;
    for (int i = 0; i < maxSize; i++) {
        if (arr[i] != NULL) {
            count++;
        }
    }
    return count;
}

void deleteLine(){
    if (main_state.userBufferPos == 0){return;}
    while(main_state.userBufferPos != 0){
        main_state.userBuffer[main_state.userBufferPos - 1] = 0;
        main_state.userBufferPos--;
        syscall(5, (uint32_t) '\b', color_to_int(WHITE), 0);
    }
}

// void handle_up_arrow(){
//     if(main_state.last_index_arrow == 0){
//         return;
//     }
//     deleteLine();
//     main_state.last_index_arrow--;
//     int length = strlen(main_state.stringBuffer[main_state.last_index_arrow]);
//     for (int i = 0; i < length; i++) {
//         inputChar(main_state.stringBuffer[main_state.last_index_arrow][i]);
//     }
// }

// void handle_down_arrow(){
//     if(main_state.last_index_arrow == getArrayLength(main_state.stringBuffer, 256)){
//         return;
//     }
//     deleteLine();
//     main_state.last_index_arrow++;
//     int length = strlen(main_state.stringBuffer[main_state.last_index_arrow]);
//     for (int i = 0; i < length; i++) {
//         inputChar (main_state.stringBuffer[main_state.last_index_arrow][i]);
//     }
// }