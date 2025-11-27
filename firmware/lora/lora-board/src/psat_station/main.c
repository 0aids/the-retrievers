#include <stdio.h>
#include "tremo_delay.h"
#include "tremo_it.h"
#include "psat_lora.h"
#include "init.h"

int main(void)
{
    // Target board initialization
    board_init();

    PsatRadioInit();

    while (1) {
        printf("Looping!\r\n");
        delay_ms(500);
        PsatRadioMain();
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(void* file, uint32_t line)
{
    (void)file;
    (void)line;

    while (1) { }
}
#endif
