#include <stdint.h>
#include <cstring>

constexpr uintptr_t VGA_A = 0xB8000;
constexpr uint8_t BG = 0x0F;
constexpr int SCREEN_W = 80;
constexpr int SCREEN_H = 25;

class VGA {
public:
    volatile uint16_t *vga_buffer;

    VGA() : vga_buffer(reinterpret_cast<uint16_t*>(VGA_A)) {}
    void outputChar(char c, int col, int row, uint8_t attr) {
        int offset = row * SCREEN_W + col;
        vga_buffer[offset] = (attr << 8) | c;
    }
    void outputString(const char* str, int col, int row, uint8_t attr) {
        int offset = row * SCREEN_W + col;
        for (int i = 0; str[i] != '\0'; i++) {
            vga_buffer[offset + i] = (attr << 8) | str[i];
        }
    }
    void clearLine(int row, uint8_t attr) {
        for (int col = 0; col < SCREEN_W; col++) {
            outputChar(' ', col, row, attr);
        }
    }
    void moveCursor(int col, int row) {
        uint16_t cursorPos = row * SCREEN_W + col;
        outb(0x3D4, 0x0E);
        outb(0x3D5, cursorPos >> 8);
        outb(0x3D4, 0x0F);
        outb(0x3D5, cursorPos & 0xFF);
    }
};
void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}


extern "C" [[noreturn]] void main() {
    VGA vga;
    vga.clearLine(0, BG);
    vga.outputString("[*] ===== Simple OS & Kernel ===== [*]", 1, 1, BG);
    vga.outputString("----- Type '$help' for commands -----", 1, 2, BG);

    int cursorX = 0;
    int cursorY = 3;
    vga.moveCursor(cursorX, cursorY);
    char inputBuffer[80];
    int inputIndex = 0;
    while (true) {
        uint8_t key = inb(0x60);
        if (key == 0x0E && inputIndex > 0) {
            inputIndex--;
            vga.outputChar(' ', cursorX - 1, cursorY, BG);
            cursorX--;
        }
        else if (key == 0x1C) {
            inputBuffer[inputIndex] = '\0';

            if (inputIndex > 0) {
                if (strcmp(inputBuffer, "$help") == 0) {
                    vga.clearLine(cursorY, BG);
                    vga.outputString("Available Commands: $help, $test", 0, cursorY, BG);
                } else if (strcmp(inputBuffer, "$test") == 0) {
                    vga.clearLine(cursorY, BG);
                    vga.outputString("wow, it works", 0, cursorY, BG);
                } else {
                    vga.clearLine(cursorY, BG);
                    vga.outputString("Unknown command - try '$help' for a list of commands", 0, cursorY, BG);
                }
            }
            for (int i = 0; i < inputIndex; i++) {
                inputBuffer[i] = '\0';
            }
            inputIndex = 0;
            cursorX = 0;
            vga.moveCursor(cursorX, cursorY);
        }
        else if (key >= 0x02 && key <= 0x0A) {
            if (inputIndex < 79) {
                inputBuffer[inputIndex++] = key;
                vga.outputChar(key, cursorX++, cursorY, BG);
            }
        }
    }
}
int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return (unsigned char)*str1 - (unsigned char)*str2;
}
