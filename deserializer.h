#ifndef DESERIALIZER_H_
#define DESERIALIZER_H_
#include "serializer.h"
#include "list.h"

typedef struct deserializer_value
{
    int read_type;
    bool_t little_endian;
    char *key;
    long key_size;
    long value_1;
    char *value_2;
    long index;          // index is only for the index-based values (ones which don't have keys)
    long absolute_index; // absolute_index is index for all the values
} deserializer_value_t;

typedef struct binary_deserializer
{
    char *memory;
    long index;
    long size;
    long current_index;
    long current_absolute_index;
} deserializer_t;

deserializer_t
deserializer_init (char *memory, int size);
deserializer_value_t
deserialize_next (deserializer_t *deserializer);
my_list_s
deserialize_all (deserializer_t *deserializer);
#endif