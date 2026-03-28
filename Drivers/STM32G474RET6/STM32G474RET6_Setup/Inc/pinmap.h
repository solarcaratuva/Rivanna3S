#ifndef PINMAP_H
#define PINMAP_H

#include <stdbool.h>
#include "stm32g474xx.h"

#ifdef Pin
#error Pin_is_a_macro
#endif

typedef struct Pin {
    GPIO_TypeDef* block;        // Pointer to GPIO peripheral block
    uint16_t block_mask;        // Mask for pins within the block
    uint64_t universal_mask;    // Unique global pin mask

    #ifdef __cplusplus
    bool operator==(const Pin& a) const {
        return (a.block == block) && (a.block_mask == block_mask) && (a.universal_mask == universal_mask);
    }
    #endif
} Pin;

#define NC (Pin){NULL, 0, 0} // Not connected pin

#define PC_13 (Pin){GPIOC, 1 << 13, 1ULL << 1}
#define PC_0 (Pin){GPIOC, 1 << 0, 1ULL << 7}
#define PC_1 (Pin){GPIOC, 1 << 1, 1ULL << 8}
#define PC_2 (Pin){GPIOC, 1 << 2, 1ULL << 9}
#define PC_3 (Pin){GPIOC, 1 << 3, 1ULL << 10}
#define PA_0 (Pin){GPIOA, 1 << 0, 1ULL << 11}
#define PA_1 (Pin){GPIOA, 1 << 1, 1ULL << 12}
#define PA_2 (Pin){GPIOA, 1 << 2, 1ULL << 13}
#define PA_3 (Pin){GPIOA, 1 << 3, 1ULL << 16}
#define PA_4 (Pin){GPIOA, 1 << 4, 1ULL << 17}
#define PA_5 (Pin){GPIOA, 1 << 5, 1ULL << 18}
#define PA_6 (Pin){GPIOA, 1 << 6, 1ULL << 19}
#define PA_7 (Pin){GPIOA, 1 << 7, 1ULL << 20}
#define PC_4 (Pin){GPIOC, 1 << 4, 1ULL << 21}
#define PC_5 (Pin){GPIOC, 1 << 5, 1ULL << 22}
#define PB_0 (Pin){GPIOB, 1 << 0, 1ULL << 23}
#define PB_1 (Pin){GPIOB, 1 << 1, 1ULL << 24}
#define PB_2 (Pin){GPIOB, 1 << 2, 1ULL << 25}
#define PB_10 (Pin){GPIOB, 1 << 10, 1ULL << 29}
#define PB_11 (Pin){GPIOB, 1 << 11, 1ULL << 32}
#define PB_12 (Pin){GPIOB, 1 << 12, 1ULL << 33}
#define PB_13 (Pin){GPIOB, 1 << 13, 1ULL << 34}
#define PB_14 (Pin){GPIOB, 1 << 14, 1ULL << 35}
#define PB_15 (Pin){GPIOB, 1 << 15, 1ULL << 36}
#define PC_6 (Pin){GPIOC, 1 << 6, 1ULL << 37}
#define PC_7 (Pin){GPIOC, 1 << 7, 1ULL << 38}
#define PC_8 (Pin){GPIOC, 1 << 8, 1ULL << 39}
#define PC_9 (Pin){GPIOC, 1 << 9, 1ULL << 40}
#define PA_8 (Pin){GPIOA, 1 << 8, 1ULL << 41}
#define PA_9 (Pin){GPIOA, 1 << 9, 1ULL << 42}
#define PA_10 (Pin){GPIOA, 1 << 10, 1ULL << 43}
#define PA_11 (Pin){GPIOA, 1 << 11, 1ULL << 44}
#define PA_12 (Pin){GPIOA, 1 << 12, 1ULL << 45}
#define PA_13 (Pin){GPIOA, 1 << 13, 1ULL << 48}
#define PA_14 (Pin){GPIOA, 1 << 14, 1ULL << 49}
#define PA_15 (Pin){GPIOA, 1 << 15, 1ULL << 50}
#define PC_10 (Pin){GPIOC, 1 << 10, 1ULL << 51}
#define PC_11 (Pin){GPIOC, 1 << 11, 1ULL << 52}
#define PC_12 (Pin){GPIOC, 1 << 12, 1ULL << 53}
#define PD_2 (Pin){GPIOD, 1 << 2, 1ULL << 54}
#define PB_3 (Pin){GPIOB, 1 << 3, 1ULL << 55}
#define PB_4 (Pin){GPIOB, 1 << 4, 1ULL << 56}
#define PB_5 (Pin){GPIOB, 1 << 5, 1ULL << 57}
#define PB_6 (Pin){GPIOB, 1 << 6, 1ULL << 58}
#define PB_7 (Pin){GPIOB, 1 << 7, 1ULL << 59}
#define PB_9 (Pin){GPIOB, 1 << 9, 1ULL << 61}

#endif /* PINMAP */
