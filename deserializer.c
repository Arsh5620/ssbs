#include "deserializer.h"
#include <inttypes.h>
#include <assert.h>

extern bool_t is_little_endian;

inline deserializer_t
deserializer_init (char *memory, long size, bool_t copy_strings)
{
    deserializer_t deserializer = {0};
    deserializer.memory = memory;
    deserializer.size = size;
    deserializer.copy_strings = copy_strings;
    if (copy_strings)
    {
        deserializer.copy_string_memory = malloc (4096);
        deserializer.copy_string_length = 4096;
    }
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
        if (deserializer->copy_strings)
        {
            long size_1 = value + 1024;
            if (deserializer->copy_string_index + size_1 > deserializer->copy_string_length)
            {
                deserializer_allocate_if_required (deserializer, size_1);
            }

            memcpy (
              deserializer->copy_string_memory + deserializer->copy_string_index,
              deserializer->memory + deserializer->index,
              value);

            deserializer->copy_string_index += value;
        }
        else
        {
            deserialized.value_pointer = deserializer->memory + deserializer->index;
        }
        deserializer->index += value;
    }

    return deserialized;
}

__attribute__ ((noinline)) void
deserializer_allocate_if_required (deserializer_t *deserializer, long additional_size)
{
    size_t copy = deserializer->copy_string_index;
    deserializer->copy_string_length *= 1.4;
    deserializer->copy_string_length += additional_size;

    char *address = realloc (deserializer->copy_string_memory, deserializer->copy_string_length);

    if (NULL == address)
    {
        address = malloc (deserializer->copy_string_length);
        assert (address != NULL);

        memcpy (address, deserializer->copy_string_memory, copy);
        free (deserializer->copy_string_memory);
    }

    deserializer->copy_string_memory = address;
}

void
deserialize_all (deserializer_t *deserializer, my_list_s *list)
{
    while (deserializer->index < deserializer->size)
    {
        deserializer_value_t value = deserialize_next (deserializer);
        if (value.type == DESERIALIZATION_TYPE_EOF || value.type == DESERIALIZATION_TYPE_NONE)
        {
            if (
              deserializer->copy_strings
              && deserializer->copy_string_index < deserializer->copy_string_length)
            {
                // https://stackoverflow.com/questions/9575122/can-i-assume-that-calling-realloc-with-a-smaller-size-will-free-the-remainder
                // reallocing smaller size frees the rest of the size
                char *address
                  = realloc (deserializer->copy_string_memory, deserializer->copy_string_index);
                assert (address != NULL);
                deserializer->copy_string_length = deserializer->copy_string_index;
            }
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

void
deserializer_free (deserializer_t deserializer)
{
    if (deserializer.copy_strings)
    {
        free (deserializer.copy_string_memory);
    }
}