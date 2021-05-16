#ifndef DESERIALIZER_H_
#define DESERIALIZER_H_
#include "serializer.h"
#include "list.h"

// Cannot have more than 16 types, as we are using 4 bits for this field
typedef enum binary_deserialization_types
{
    DESERIALIZATION_TYPE_NONE = 0x00, // mostly similar to blob, but not actually a type
    DESERIALIZATION_TYPE_EOF = 0x10,
    DESERIALIZATION_TYPE_CHAR = 0x20,
    DESERIALIZATION_TYPE_SHORT = 0x30,
    DESERIALIZATION_TYPE_INT = 0x40,
    DESERIALIZATION_TYPE_LONG = 0x50,
    DESERIALIZATION_TYPE_FLOAT = 0x60,
    DESERIALIZATION_TYPE_DOUBLE = 0x70,
    DESERIALIZATION_TYPE_BLOB = 0x80,
} deserialization_types_t;

typedef struct deserializer_value
{
    unsigned char type;
    unsigned char *key;
    unsigned char key_length;
    char _unused;
    union
    {
        long value_long;
        int value_int;
        short value_short;
        char value_char;
        float value_float;
        double value_double;
    };

    char *value_pointer;
    int absolute_index; // absolute_index is index for all the values
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
deserializer_init (char *memory, long size);
deserializer_value_t
deserialize_next (deserializer_t *deserializer);
void
deserialize_all (deserializer_t *deserializer, my_list_s *list);
#endif