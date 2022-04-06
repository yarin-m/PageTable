#include "os.h"
#include <stdio.h>

uint16_t mask_9bits_and_shift(uint64_t address, int start);

uint16_t mask_9bits_and_shift(uint64_t address, int start)
{
    uint16_t result = address >> start;
    return result&511;
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn)
{
    uint64_t offsets[5];
    int starting_indices[5] = {36, 27, 18, 9, 0};
    for (int i = 0; i < 5; i++)
    {
        offsets[i] = mask_9bits_and_shift(vpn, starting_indices[i]);
    }
    uint64_t current_table = (pt << 12);
    uint64_t* current_table_address;
    uint64_t* current_entry;
    int isValid = 0;
    for (int i = 0; i < 5; i++)
    {
        current_table_address = (uint64_t*) phys_to_virt(current_table);
        current_entry = &current_table_address[offsets[i]];
        isValid = (int) *current_entry&1;
        if (isValid == 0)
        {
            return NO_MAPPING;
        }
        current_table = *current_entry & 0xfffff000;
    }
    return current_table >> 12;
}

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn)
{
    int toDestroy = 0;
    if(ppn == NO_MAPPING)
    {
        toDestroy = 1;
    }
    uint64_t offsets[5];
    int starting_indices[5] = {36, 27, 18, 9, 0};
    for (int i = 0; i < 5; i++)
    {
        offsets[i] = mask_9bits_and_shift(vpn, starting_indices[i]);
    }
    uint64_t current_table = pt << 12;
    uint64_t* current_table_address;
    uint64_t* current_entry;
    int isValid = 0;
    if (toDestroy)
    {
        for (int i = 0; i < 5; i++)
        {
            current_table_address = (uint64_t*) phys_to_virt(current_table);
            current_entry = &current_table_address[offsets[i]];
            isValid = (int) *current_entry&1;
            if (isValid == 0)
            {
                return;
            }
            if ((i == 4) & (isValid == 1))
            {
                *current_entry = 0;
                return;
            }
            current_table = *current_entry & 0xfffff000;
        }
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            current_table_address = (uint64_t*) phys_to_virt(current_table);
            current_entry = &current_table_address[offsets[i]];
            isValid = (int) *current_entry&1;
            if (i == 4)
            {
                *current_entry = (ppn << 12) | 1;
                return;
            }
            if (isValid == 0)
            {
                *current_entry = (alloc_page_frame() << 12) | 1;
            }
            current_table = *current_entry & 0xfffff000;
        }
    }
        return;
}