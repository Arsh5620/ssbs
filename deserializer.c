#include "deserializer.h"
#include <inttypes.h>

extern bool_t is_little_endian;

inline deserializer_t
deserializer_init (char *memory, long size)
{
    deserializer_t deserializer = {0};
    deserializer.memory = memory;
    deserializer.size = size;

    is_little_endian = endianness_test ();
    return deserializer;
}

// NOTE
// Do not inline absurdly big functions as they hurt performance
// due to either page faults/instruction cache misses/register spilling
deserializer_value_t
deserialize_next (deserializer_t *deserializer)
{
    deserializer_value_t deserialized = {0};

    char magic = deserializer->memory[deserializer->index++];
    deserialized.type = magic & 0xF0;

    bool_t has_key = magic >> 2 & 1;
    char size = (1 << (magic & 0b11));

    if (deserialized.type == DESERIALIZATION_TYPE_EOF)
    {
        return deserialized;
    }

    if (has_key)
    {
        char key_size = deserializer->memory[deserializer->index++];
        deserialized.key = deserializer->memory + deserializer->index;
        deserializer->index += key_size;
        deserialized.key_length = key_size;
    }

    long value = 0;
    memcpy (&value, deserializer->memory + deserializer->index, size);
    deserializer->index += size;

    if (is_little_endian != (magic >> 3 & 1))
    {
        value = __builtin_bswap64 (value);
    }

    deserialized.value_long = value;

    if (deserialized.type == DESERIALIZATION_TYPE_BLOB)
    {
        deserialized.value_pointer = deserializer->memory + deserializer->index;
        deserializer->index += value;
    }

    return deserialized;
}

void
deserialize_all (deserializer_t *deserializer, my_list_s *list)
{
    while (deserializer->index < deserializer->size)
    {
        deserializer_value_t value = deserialize_next (deserializer);
        if (value.type == DESERIALIZATION_TYPE_EOF || value.type == DESERIALIZATION_TYPE_NONE)
        {
            break;
        }

        // value.index = deserializer->current_index;
        value.absolute_index = deserializer->current_absolute_index;

        my_list_push (list, (char *) &value);

        if (value.key != NULL)
        {
            deserializer->current_index++;
        }
        deserializer->current_absolute_index++;
    }
}