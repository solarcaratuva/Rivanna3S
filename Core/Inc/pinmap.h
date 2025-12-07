#ifndef PINMAP_H
#define PINMAP_H


#include "stm32h743xx.h"

// struct
// GPIO_TypeDef block (peripheral block memory address)
// uint16_t block_mask (in-block pin mask)
// uint64_t universal_mask (unique pin mask)



typedef struct {
    GPIO_TypeDef* block;       // Pointer to GPIO peripheral block
    uint16_t block_mask;       // Mask for pins within the block
    uint64_t universal_mask;   // Unique global pin mask
} Pin;


// define
#define NC   (Pin){nullptr, 0, 0} // Not connected pin

// A's
#define PA_0 (Pin){GPIOA, 1 << 0, 1ULL << 33}
#define PA_1 (Pin){GPIOA, 1 << 1, 1ULL << 34}
#define PA_2 (Pin){GPIOA, 1 << 2, 1ULL << 35}
#define PA_3 (Pin){GPIOA, 1 << 3, 1ULL << 36}
#define PA_4 (Pin){GPIOA, 1 << 4, 1ULL << 39}
#define PA_5 (Pin){GPIOA, 1 << 5, 1ULL << 40}
#define PA_6 (Pin){GPIOA, 1 << 6, 1ULL << 41}
#define PA_7 (Pin){GPIOA, 1 << 7, 1ULL << 42}

// B's
#define PB_0 (Pin){GPIOB, 1 << 0, 1ULL << 45}
#define PB_1 (Pin){GPIOB, 1 << 1, 1ULL << 46}
#define PB_2 (Pin){GPIOB, 1 << 2, 1ULL << 47}
//fake D's for FDCAN1
#define PB_8 (Pin){GPIOB, 1 << 0, 1ULL << 60}
#define PB_9 (Pin){GPIOB, 1 << 1, 1ULL << 61}


// C's
#define PC_0 (Pin){GPIOC, 1 << 0, 1ULL << 25}
#define PC_1 (Pin){GPIOC, 1 << 1, 1ULL << 26}
#define PC_4 (Pin){GPIOC, 1 << 4, 1ULL << 43}
#define PC_5 (Pin){GPIOC, 1 << 5, 1ULL << 44}
#define PC_13 (Pin){GPIOC, 1 << 13, 1ULL << 6}


// E's
#define PE_2 (Pin){GPIOE, 1 << 2, 1ULL << 0}
#define PE_3 (Pin){GPIOE, 1 << 3, 1ULL << 1}
#define PE_4 (Pin){GPIOE, 1 << 4, 1ULL << 2}
#define PE_5 (Pin){GPIOE, 1 << 5, 1ULL << 3}
#define PE_6 (Pin){GPIOE, 1 << 6, 1ULL << 4}
#define PE_7 (Pin){GPIOE, 1 << 7, 1ULL << 57}
#define PE_8 (Pin){GPIOE, 1 << 8, 1ULL << 58}
#define PE_9 (Pin){GPIOE, 1 << 9, 1ULL << 59}
#define PE_10 (Pin){GPIOE, 1 << 10, 1ULL << 62}

// F's
#define PF_0 (Pin){GPIOF, 1 << 0, 1ULL << 9}
#define PF_1 (Pin){GPIOF, 1 << 1, 1ULL << 10}
#define PF_2 (Pin){GPIOF, 1 << 2, 1ULL << 11}
#define PF_3 (Pin){GPIOF, 1 << 3, 1ULL << 12}
#define PF_4 (Pin){GPIOF, 1 << 4, 1ULL << 13}
#define PF_5 (Pin){GPIOF, 1 << 5, 1ULL << 14}
#define PF_6 (Pin){GPIOF, 1 << 6, 1ULL << 17}
#define PF_7 (Pin){GPIOF, 1 << 7, 1ULL << 18}
#define PF_8 (Pin){GPIOF, 1 << 8, 1ULL << 19}
#define PF_9 (Pin){GPIOF, 1 << 9, 1ULL << 20}
#define PF_10 (Pin){GPIOF, 1 << 10, 1ULL << 21}
#define PF_11 (Pin){GPIOF, 1 << 11, 1ULL << 48}
#define PF_12 (Pin){GPIOF, 1 << 12, 1ULL << 49}
#define PF_13 (Pin){GPIOF, 1 << 13, 1ULL << 52}
#define PF_14 (Pin){GPIOF, 1 << 14, 1ULL << 53}
#define PF_15 (Pin){GPIOF, 1 << 15, 1ULL << 54}

// G's
#define PG_0 (Pin){GPIOG, 1 << 0, 1ULL << 55}
#define PG_1 (Pin){GPIOG, 1 << 1, 1ULL << 56}

#endif /* PINMAP */
