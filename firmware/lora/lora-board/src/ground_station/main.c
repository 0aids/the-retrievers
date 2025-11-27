#include <stdio.h>
#include "tremo_delay.h"
#include "init.h"
#include "ground_lora.h"

int main(void)
{
    // Target board initialization
    board_init();

    GroundRadioInit();

    while (1) {
        printf("Looping!\r\n");
        GroundRadioMain();
        delay_ms(500);
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
