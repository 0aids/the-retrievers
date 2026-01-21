#ifndef printWrapper_h_INCLUDED
#define printWrapper_h_INCLUDED

#define printWrapper_preamble_d 0x11

// Pads the print statement with a byte at the beginning to signify that it's the lora module having an error.
void printw(const char* fmt, ...);

#endif // printWrapper_h_INCLUDED
