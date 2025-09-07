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

// A's
const Pin PA_0 = {GPIOA, 1 << 0, 1 << 33};
const Pin PA_1 = {GPIOA, 1 << 1, 1 << 34};
const Pin PA_2 = {GPIOA, 1 << 2, 1 << 35};
const Pin PA_3 = {GPIOA, 1 << 3, 1 << 36};
const Pin PA_4 = {GPIOA, 1 << 4, 1 << 39};
const Pin PA_5 = {GPIOA, 1 << 5, 1 << 40};
const Pin PA_6 = {GPIOA, 1 << 6, 1 << 41};
const Pin PA_7 = {GPIOA, 1 << 7, 1 << 42};

// B's
const Pin PB_0 = {GPIOB, 1 << 0, 1 << 45};
const Pin PB_1 = {GPIOB, 1 << 1, 1 << 46};
const Pin PB_2 = {GPIOB, 1 << 2, 1 << 47};

// C's
const Pin PC_0 = {GPIOC, 1 << 0, 1 << 25};
const Pin PC_1 = {GPIOC, 1 << 1, 1 << 26};
const Pin PC_4 = {GPIOC, 1 << 4, 1 << 43};
const Pin PC_5 = {GPIOC, 1 << 5, 1 << 44};
const Pin PC_13 = {GPIOC, 1 << 13, 1 << 6};

// E's
const Pin PE_2 = {GPIOE, 1 << 2, 1 << 0};
const Pin PE_3 = {GPIOE, 1 << 3, 1 << 1};
const Pin PE_4 = {GPIOE, 1 << 4, 1 << 2};
const Pin PE_5 = {GPIOE, 1 << 5, 1 << 3};
const Pin PE_6 = {GPIOE, 1 << 6, 1 << 4};
const Pin PE_7 = {GPIOE, 1 << 7, 1 << 57};
const Pin PE_8 = {GPIOE, 1 << 8, 1 << 58};
const Pin PE_9 = {GPIOE, 1 << 9, 1 << 59};
const Pin PE_10 = {GPIOE, 1 << 10, 1 << 62};

// F's
const Pin PF_0 = {GPIOF, 1 << 0, 1 << 9};
const Pin PF_1 = {GPIOF, 1 << 1, 1 << 10};
const Pin PF_2 = {GPIOF, 1 << 2, 1 << 11};
const Pin PF_3 = {GPIOF, 1 << 3, 1 << 12};
const Pin PF_4 = {GPIOF, 1 << 4, 1 << 13};
const Pin PF_5 = {GPIOF, 1 << 5, 1 << 14};
const Pin PF_6 = {GPIOF, 1 << 6, 1 << 17};
const Pin PF_7 = {GPIOF, 1 << 7, 1 << 18};
const Pin PF_8 = {GPIOF, 1 << 8, 1 << 19};
const Pin PF_9 = {GPIOF, 1 << 9, 1 << 20};
const Pin PF_10 = {GPIOF, 1 << 10, 1 << 21};
const Pin PF_11 = {GPIOF, 1 << 11, 1 << 48};
const Pin PF_12 = {GPIOF, 1 << 12, 1 << 49};
const Pin PF_13 = {GPIOF, 1 << 13, 1 << 52};
const Pin PF_14 = {GPIOF, 1 << 14, 1 << 53};
const Pin PF_15 = {GPIOF, 1 << 15, 1 << 54};

// G's
const Pin PG_0 = {GPIOG, 1 << 0, 1 << 55};
const Pin PG_1 = {GPIOG, 1 << 1, 1 << 56};