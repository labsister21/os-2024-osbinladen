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

static struct KeyboardDriverState keyboard_state = {
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

void keyboard_isr(void) {
  if (!keyboard_state.keyboard_input_on){
    keyboard_state.buffer_index = 0;
  }else {
    uint8_t scancode = in(KEYBOARD_DATA_PORT);
    char key = keyboard_scancode_1_to_ascii_map[scancode];
    if (key == '\n'){
      keyboard_state.keyboard_buffer[keyboard_state.buffer_index] = key;
      keyboard_state.buffer_index = next_row_index(keyboard_state.buffer_index);      
    } else if(key == '\t'){
      keyboard_state.keyboard_buffer[keyboard_state.buffer_index] = key;
      keyboard_state.buffer_index = next_tab_index(keyboard_state.buffer_index);  
    } else if(key == '\b'){
      keyboard_state.keyboard_buffer[keyboard_state.buffer_index] = key;
      keyboard_state.keyboard_buffer[keyboard_state.buffer_index - 1] = 0;

      while(keyboard_state.keyboard_buffer[keyboard_state.buffer_index] == 0x0){
        keyboard_state.buffer_index--;
      }

      keyboard_state.buffer_index++;
    }
  }

  pic_ack(PIC1_OFFSET + IRQ_KEYBOARD);
    // TODO : Implement scancode processing
}

int get_cursor_pos(){
  return keyboard_state.buffer_index;
}

char get_char_buffer_at(int idx){
  return keyboard_state.keyboard_buffer[idx];
}
