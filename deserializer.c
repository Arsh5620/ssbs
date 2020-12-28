#include "deserializer.h"
#include <inttypes.h>

extern bool_t is_little_endian;

deserializer_t
deserializer_init (char *memory, int size)
{
    deserializer_t deserializer = {0};
    deserializer.memory = memory;
    deserializer.size = size;

    is_little_endian = endianness_test ();
    return deserializer;
}

deserializer_value_t
deserialize_next (deserializer_t *deserializer)
{
    deserializer_value_t deserialized = {0};

    char magic = deserializer->memory[deserializer->index++];
    deserialized.read_type = magic >> 4 & 0b1111;
    deserialized.little_endian = magic >> 3 & 1;

    bool_t has_key = magic >> 2 & 1;
    char size = (1 << (magic & 0b11));

    if (deserialized.read_type == SERIALIZATION_TYPE_EOF)
    {
        return deserialized;
    }

    if (has_key)
    {
        char key_size = deserializer->memory[deserializer->index++];
        deserialized.key = deserializer->memory + deserializer->index;
        deserializer->index += key_size;
        deserialized.key_size = key_size;
    }

    long value = 0;
    memcpy (&value, deserializer->memory + deserializer->index, size);
    deserializer->index += size;

    if (is_little_endian != deserialized.little_endian)
    {
        value = __builtin_bswap64 (value);
    }

    deserialized.value_1 = value;

    if (deserialized.read_type == SERIALIZATION_TYPE_BLOB)
    {
        deserialized.value_2 = deserializer->memory + deserializer->index;
        deserializer->index += value;
    }

    return deserialized;
}

my_list_s
deserialize_all (deserializer_t *deserializer)
{
    my_list_s list = my_list_new (16, sizeof (deserializer_value_t));

    while (deserializer->index < deserializer->size)
    {
        deserializer_value_t value = deserialize_next (deserializer);
        if (value.read_type == SERIALIZATION_TYPE_EOF)
        {
            break;
        }

        value.index = deserializer->current_index;
        value.absolute_index = deserializer->current_absolute_index;

        my_list_push (&list, (char *) &value);

        if (value.key != NULL)
        {
            deserializer->current_index;
        }
        deserializer->current_absolute_index++;
    }

    return list;
}