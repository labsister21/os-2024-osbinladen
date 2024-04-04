#include "header/driver/keyboard.h"
#include "header/cpu/portio.h"
#include "header/stdlib/string.h"


const char keyboard_scancode_1_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',
    'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

struct KeyboardDriverState keyboard_state = {
    .read_extended_mode = false,
    .keyboard_input_on = false,
    .buffer_index = 0,
    .keyboard_buffer = {0}
};
// Activate keyboard ISR / start listen keyboard & save to buffer
void keyboard_state_activate(void){
  activate_keyboard_interrupt();
  keyboard_state.keyboard_input_on = true;
}

void keyboard_state_deactivate(void){
  keyboard_state.keyboard_input_on = false;
}

void get_keyboard_buffer(char *buf){
  *buf = *(keyboard_state.keyboard_buffer);
  keyboard_state.buffer_index = 0;
}

int next_row_index(int i){
  return ((i/TEXT_WIDTH) + 1) * TEXT_WIDTH;
}

int next_tab_index(int i){
  return ((i/4) + 1) * 4;
}

char current_char();

char current_char(){
  return keyboard_state.keyboard_buffer[keyboard_state.buffer_index];
}

void handle_newline();
void handle_tab();
void handle_backspace();
void handle_others(char key);
void handle_up_arrow();
void handle_left_arrow();
void handle_right_arrow();
void handle_down_arrow();

void handle_newline(){
  keyboard_state.keyboard_buffer[keyboard_state.buffer_index] = '\n';
  keyboard_state.buffer_index = next_row_index(keyboard_state.buffer_index);     
}

void handle_tab(){
  keyboard_state.keyboard_buffer[keyboard_state.buffer_index] = '\t';
  keyboard_state.buffer_index = next_tab_index(keyboard_state.buffer_index);  
}

void handle_backspace(){

  if (keyboard_state.buffer_index == 0){return;}

  bool normalBackspace = (
    keyboard_state.keyboard_buffer[keyboard_state.buffer_index - 1] >= 32 && 
    keyboard_state.keyboard_buffer[keyboard_state.buffer_index - 1] <= 126
    );

  if (normalBackspace){
    keyboard_state.keyboard_buffer[keyboard_state.buffer_index - 1] = 0;
    keyboard_state.buffer_index--;
  }
  else {
    
    while(keyboard_state.keyboard_buffer[keyboard_state.buffer_index] == 0x0){
      keyboard_state.buffer_index--;
    }

    if (keyboard_state.keyboard_buffer[keyboard_state.buffer_index] != '\n' && keyboard_state.keyboard_buffer[keyboard_state.buffer_index] != '\t'){
      keyboard_state.buffer_index++;
    }
    else{
      keyboard_state.keyboard_buffer[keyboard_state.buffer_index] = 0;
    }
  }
}

void handle_others(char key){
  if (key >= 32 && key <= 126){
    keyboard_state.keyboard_buffer[keyboard_state.buffer_index] = key;
    keyboard_state.buffer_index++;
  }
}

void handle_up_arrow(){
  uint16_t origin_row = keyboard_state.buffer_index/TEXT_WIDTH;
  if (origin_row == 0) {return;}
  keyboard_state.buffer_index = (keyboard_state.buffer_index/TEXT_WIDTH)*TEXT_WIDTH - 1;

  while (current_char() == 0x0 || current_char() == '\n'){

    keyboard_state.buffer_index--;

    if (origin_row*TEXT_WIDTH - keyboard_state.buffer_index == TEXT_WIDTH){
      return;
    }
  }
  keyboard_state.buffer_index++;
}

void handle_down_arrow(){
  uint16_t origin_row = keyboard_state.buffer_index/TEXT_WIDTH;
  if (origin_row == TEXT_HEIGHT - 1) {return;}
  keyboard_state.buffer_index = (keyboard_state.buffer_index/TEXT_WIDTH + 1)*TEXT_WIDTH;

  while (current_char() != 0x0 && current_char() != '\n' && current_char() != '\t'){

    keyboard_state.buffer_index++;

    if (keyboard_state.buffer_index - origin_row*TEXT_WIDTH == TEXT_WIDTH){
      return;
    }
  }
}

void handle_left_arrow(){
  keyboard_state.buffer_index--;
}

void handle_right_arrow(){
  keyboard_state.buffer_index++;
}

void keyboard_isr(void) {
  if (!keyboard_state.keyboard_input_on){
    keyboard_state.buffer_index = 0;
  }
  else {
    uint8_t scancode = in(KEYBOARD_DATA_PORT);
    char key = keyboard_scancode_1_to_ascii_map[scancode];

    if (key != 0){
      switch(key){
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
          handle_others(key);
          break;
      }
    }
    else{
      switch(scancode){
        case EXT_SCANCODE_UP:
          handle_up_arrow();
          break;
        case EXT_SCANCODE_LEFT:
          handle_left_arrow();
          break;
        case EXT_SCANCODE_RIGHT:
          handle_right_arrow();
          break;
        case EXT_SCANCODE_DOWN:
          handle_down_arrow();
          break;
      }
    }
  }

  pic_ack(PIC1_OFFSET + IRQ_KEYBOARD);
    // TODO : Implement scancode processing
}

void reset_keyboard_buffer(){
  memset(keyboard_state.keyboard_buffer, 0x0, TEXT_SIZE);
  keyboard_state.buffer_index = 0;
}