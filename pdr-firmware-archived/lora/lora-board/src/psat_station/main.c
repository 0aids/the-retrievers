#include <stdio.h>
#include "tremo_delay.h"
#include "tremo_it.h"
#include "psat_lora.h"
#include "init.h"
// Because I'm lazy to add a header file.
extern void initBoard();
extern void appMain();

// TODO: Separate this into testing standalone (no esp32) and testing together (with esp32 over uart)
int main(void)
{
    // Target board initialization
#ifndef d_psatStandaloneMode
    initBoard();
    printf("Initializing! \r\n");
    appMain();

#else
    init_board();
    PsatRadioInit();
    while (1) {
        printf("Looping!\r\n");
        delay_ms(50);
        PsatRadioMain();
    }
#endif
}

#ifdef USE_FULL_ASSERT
void assert_failed(void* file, uint32_t line)
{
    (void)file;
    (void)line;

    while (1) { 
        DelayMs(1000);
        printf("Crashed!!\r\n");
    }
}
#endif
