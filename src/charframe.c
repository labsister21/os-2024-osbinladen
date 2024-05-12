#include "header/driver/charframe.h"
#include "header/stdlib/string.h"

int pos = 0;
char charBuffer[TEXT_WIDTH*TEXT_HEIGHT];

char current_char();
void handle_newline();
void handle_tab();
void handle_backspace();
void handle_up_arrow();
void handle_left_arrow();
void handle_right_arrow();
void handle_down_arrow();

char get_char_buffer_at(int idx){
    return charBuffer[idx];
}

char current_char(){
  return charBuffer[pos];
}

int next_row_index(int pos){
    return ((pos/TEXT_WIDTH) + 1)*TEXT_WIDTH;
}

void handle_newline(){
  charBuffer[pos] = '\n';
  pos = next_row_index(pos);     
}

int next_tab_index(int pos){
    return ((pos/4) + 1) * 4;
}

void handle_tab(){
  charBuffer[pos] = '\t';
  pos = next_tab_index(pos);  
}

void handle_backspace(){

  if (pos == 0){return;}

  bool normalBackspace = (
    charBuffer[pos - 1] >= 32 && 
    charBuffer[pos - 1] <= 126
    );

  if (normalBackspace){
    charBuffer[pos - 1] = 0;
    pos--;
  }
  else {
    
    while(charBuffer[pos] == 0x0){
      pos--;
    }

    if (charBuffer[pos] != '\n' && charBuffer[pos] != '\t'){
      pos++;
    }
    else{
      charBuffer[pos] = 0;
    }
  }
}

void handle_others(char key){
  if (key >= 32 && key <= 126){
    charBuffer[pos] = key;
    pos++;
  }
}

void handle_up_arrow(){
  uint16_t origin_row = pos/TEXT_WIDTH;
  if (origin_row == 0) {return;}
  pos = (pos/TEXT_WIDTH)*TEXT_WIDTH - 1;

  while (current_char() == 0x0 || current_char() == '\n'){

    pos--;

    if (origin_row*TEXT_WIDTH - pos == TEXT_WIDTH){
      return;
    }
  }
  pos++;
}

void handle_down_arrow(){
  uint16_t origin_row = pos/TEXT_WIDTH;
  if (origin_row == TEXT_HEIGHT - 1) {return;}
  pos = (pos/TEXT_WIDTH + 1)*TEXT_WIDTH;

  while (current_char() != 0x0 && current_char() != '\n' && current_char() != '\t'){

    pos++;

    if (pos - origin_row*TEXT_WIDTH == TEXT_WIDTH){
      return;
    }
  }
}


void handle_left_arrow(){
    if (charBuffer[pos-2] != '$') {
        pos--; 
        char temp = charBuffer[pos];
        charBuffer[pos+1] = charBuffer[pos];
        draw_char_at(temp, (pos + 1) / TEXT_WIDTH, (pos + 1) % TEXT_WIDTH, WHITE, BLACK);
       
    }
}

void handle_right_arrow(){
  if(charBuffer[pos+1]!= '\0'){
    pos++;
    char temp = charBuffer[pos];
    charBuffer[pos-1] = temp;
     draw_char_at(temp, (pos - 1) / TEXT_WIDTH, (pos - 1) % TEXT_WIDTH, WHITE, BLACK);
  }
}

void key_handler(char c){
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

void reset_buffer(){
  memset(charBuffer, 0x0, TEXT_SIZE);
  pos = 0;
}

int get_cursor_pos(){
    return pos;
}

void buffered_draw(char c, Color16 fg, Color16 bg){
    int last_pos = pos;
    key_handler(c);
    while (pos != last_pos){
        draw_char_at(charBuffer[last_pos], last_pos/TEXT_WIDTH, last_pos % TEXT_WIDTH, fg, bg);
        last_pos += (pos > last_pos) ? 1 : -1;
    }
}

void putchar(char c, uint16_t textColor){
    buffered_draw(c, int_to_color(textColor), BLACK);
}

void puts(char* buffer, int charCount, uint16_t textColor){
    for(int i = 0; i < charCount; i++){
        putchar(buffer[i], textColor);
    }
}

void draw_cursor(){
    draw_char_at('|', pos/TEXT_WIDTH, pos % TEXT_WIDTH, BLUE, BLACK);
}