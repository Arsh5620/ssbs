#include "serializer.h"
#include <assert.h>

bool_t is_little_endian = 0;

inline bool_t
endianness_test ()
{
    char value = 77;
    int endian_test = value;

    if (*(char *) &endian_test == value)
    {
        return SERIALIZER_TRUE;
    }
    else
    {
        return SERIALIZER_FALSE;
    }
}

inline serializer_t
serializer_init ()
{
    serializer_t serializer = {0};
    serializer.memory = (char *) malloc (4096);
    serializer.size = 4096;

    is_little_endian = endianness_test ();
    return serializer;
}

inline void
serializer_reset (serializer_t *serializer)
{
    serializer->index = 0;
}

inline void
serializer_free (serializer_t serializer)
{
    free (serializer.memory);
}

inline char
serializer_min (int value, int min)
{
    return (char) (value > min ? min : value);
}

void
serializer_add_char (serializer_t *serializer, char *key, int key_length, char value)
{
    serializer_add_type (serializer, SERIALIZATION_TYPE_CHAR, key != NULL, 0);
    serializer_add_key (serializer, key, key_length);
    serializer_add_binary (serializer, SERIALIZATION_TYPE_CHAR, (char *) &value, sizeof (char));
}

void
serializer_add_short (serializer_t *serializer, char *key, int key_length, short value)
{
    serializer_add_type (serializer, SERIALIZATION_TYPE_SHORT, key != NULL, 0);
    serializer_add_key (serializer, key, key_length);
    serializer_add_binary (serializer, SERIALIZATION_TYPE_SHORT, (char *) &value, sizeof (short));
}

void
serializer_add_int (serializer_t *serializer, char *key, int key_length, int value)
{
    serializer_add_type (serializer, SERIALIZATION_TYPE_INT, key != NULL, 0);
    serializer_add_key (serializer, key, key_length);
    serializer_add_binary (serializer, SERIALIZATION_TYPE_INT, (char *) &value, sizeof (int));
}

void
serializer_add_long (serializer_t *serializer, char *key, int key_length, long value)
{
    serializer_add_type (serializer, SERIALIZATION_TYPE_LONG, key != NULL, 0);
    serializer_add_key (serializer, key, key_length);
    serializer_add_binary (serializer, SERIALIZATION_TYPE_LONG, (char *) &value, sizeof (long));
}

void
serializer_add_float (serializer_t *serializer, char *key, int key_length, float value)
{
    serializer_add_type (serializer, SERIALIZATION_TYPE_FLOAT, key != NULL, 0);
    serializer_add_key (serializer, key, key_length);
    serializer_add_binary (serializer, SERIALIZATION_TYPE_FLOAT, (char *) &value, sizeof (float));
}

void
serializer_add_double (serializer_t *serializer, char *key, int key_length, double value)
{
    serializer_add_type (serializer, SERIALIZATION_TYPE_DOUBLE, key != NULL, 0);
    serializer_add_key (serializer, key, key_length);
    serializer_add_binary (serializer, SERIALIZATION_TYPE_DOUBLE, (char *) &value, sizeof (double));
}

void
serializer_add_blob (serializer_t *serializer, char *key, int key_length, char *value, long size)
{
    serializer_add_type (serializer, SERIALIZATION_TYPE_BLOB, key != NULL, size);
    serializer_add_key (serializer, key, key_length);
    serializer_add_binary (serializer, SERIALIZATION_TYPE_BLOB, value, size);
}

inline void
serializer_add_eof (serializer_t *serializer)
{
    serializer_add_type (serializer, SERIALIZATION_TYPE_EOF, SERIALIZER_FALSE, 0);
}

inline void
serializer_add_key (serializer_t *serializer, char *key, int key_length)
{
    if (key != NULL)
    {
        char length = serializer_min (key_length, 255);
        serializer->memory[serializer->index++] = length;
        serializer_add_binary (serializer, SERIALIZATION_TYPE_NONE, key, length);
    }
}

inline void
serializer_add_binary (serializer_t *serializer, serialization_types_t type, char *data, int size)
{
    if (type == SERIALIZATION_TYPE_BLOB)
    {
        int copy_size = 1 << serializer_get_intsize (size);
        memcpy (serializer->memory + serializer->index, &size, copy_size);
        serializer->index += copy_size;
    }

    // To avoid calling this check everywhere instead add 32
    if (serializer->index + size + 32 > serializer->size)
    {
        serializer_allocate_if_required (serializer, size);
    }

    memcpy (serializer->memory + serializer->index, data, size);
    serializer->index += size;
}

inline void
serializer_allocate_if_required (serializer_t *serializer, long additional_size)
{
    size_t copy = serializer->index;
    serializer->size += (4096 + additional_size);

    char *address = realloc (serializer->memory, serializer->size);

    if (NULL == address)
    {
        address = calloc (serializer->size, 1);
        assert (address != NULL);

        memcpy (address, serializer->memory, copy);
        free (serializer->memory); /* free the original memory space after copy */
    }
    else
    {
        /* zero the memory after list->count elements */
        memset (address + copy, 0, serializer->size - copy);
    }
    serializer->memory = address;
}

inline void
serializer_add_type (
  serializer_t *serializer, serialization_types_t type, bool_t has_key, long size)
{
    // First four bits will be the type of the data
    // Next byte is for the byte order lsf(0) or msf(1)
    // Next byte is used to check if the its a key value pair or an indexed value
    // And the last 3 bits are used for the size of the raw data information

    unsigned char value = (unsigned char) type << 4;
    value |= (is_little_endian > 0) << 3;
    value |= (has_key > 0) << 2;
    value |= (serializer_get_typesize (type, size) & 0b11);

    serializer->memory[serializer->index++] = value;
}

inline int
serializer_get_typesize (serialization_types_t type, long size)
{
    char byte_size;
    switch (type)
    {
    case SERIALIZATION_TYPE_NONE:
    case SERIALIZATION_TYPE_EOF:
        byte_size = -1;
        break;
    case SERIALIZATION_TYPE_CHAR:
        byte_size = SIZE_BYTE;
        break;
    case SERIALIZATION_TYPE_SHORT:
        byte_size = SIZE_2BYTES;
        break;
    case SERIALIZATION_TYPE_INT:
    case SERIALIZATION_TYPE_FLOAT:
        byte_size = SIZE_4BYTES;
        break;
    case SERIALIZATION_TYPE_LONG:
    case SERIALIZATION_TYPE_DOUBLE:
        byte_size = SIZE_8BYTES;
        break;
    case SERIALIZATION_TYPE_BLOB:
        byte_size = serializer_get_intsize (size);
        break;
    }
    return byte_size;
}

inline int
serializer_get_intsize (int size)
{
    int copy_size;
    if (size < 0x100)
    {
        copy_size = 0;
    }
    else if (size < 0x10000)
    {
        copy_size = 1;
    }
    else if (size < 0x100000000)
    {
        copy_size = 2;
    }
    else
    {
        copy_size = 3;
    }

    return copy_size;
}