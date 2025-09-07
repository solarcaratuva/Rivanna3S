// struct
// GPIO_TypeDef block (peripheral block memory address)
// uint16_t block_mask (in-block pin mask)
// uint64_t universal_mask (unique pin mask)

typedef struct {
    GPIO_TypeDef* block;       // Pointer to GPIO peripheral block
    uint16_t block_mask;       // Mask for pins within the block
    uint64_t universal_mask;   // Unique global pin mask
} GPIO_PinDescriptor;

// constgruct

// define
