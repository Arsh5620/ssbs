#include "serializer.h"
#include <assert.h>

bool_t is_little_endian = 0;

__attribute__ ((always_inline)) inline bool_t
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

serializer_t
serializer_init (long default_size)
{
    serializer_t serializer = {0};
    serializer.memory = (char *) malloc (default_size);
    serializer.size = default_size;

    is_little_endian = endianness_test ();
    return serializer;
}

__attribute__ ((always_inline)) inline void
serializer_reset (serializer_t *serializer)
{
    serializer->index = 0;
}

void
serializer_free (serializer_t serializer)
{
    free (serializer.memory);
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
serializer_add_blob (
  serializer_t *serializer, char *key, unsigned char key_length, char *value, long size)
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

__attribute__ ((always_inline)) inline void
serializer_add_key (serializer_t *serializer, char *key, unsigned char key_length)
{
    if (key != NULL)
    {
        serializer->memory[serializer->index++] = key_length;
        char *dest_address = serializer->memory + serializer->index;

        // Not the fastest code, but will be vectorized by GCC
        if (key_length & 0x3)
        {
            for (size_t i = 0; i < key_length; i++)
            {
                *(dest_address + i) = key[i];
            }
        }
        else if (key_length & 0x7)
        {
            for (size_t i = 0; i < key_length >> 2; i++)
            {
                *((int *) (dest_address) + i) = ((int *) key)[i];
            }
        }
        else
        {
            for (size_t i = 0; i < key_length >> 2; i++)
            {
                *((long *) (dest_address) + i) = ((long *) key)[i];
            }
        }

        // For some reason memcpy is way too slow here
        // memcpy (serializer->memory + serializer->index, key, (long) key_length);
        // memmove (serializer->memory + serializer->index, key, (long) key_length);
        serializer->index += key_length;
    }
}

__attribute__ ((always_inline)) inline void
serializer_add_binary (serializer_t *serializer, serialization_types_t type, char *data, long size)
{
    if (__builtin_expect (type == SERIALIZATION_TYPE_BLOB, 0))
    {
        *(long *) (serializer->memory + serializer->index) = size;
        serializer->index += sizeof (long);
    }

    long size_1 = size + 1024;
    if (__builtin_expect (serializer->index + size_1 > serializer->size, 0))
    {
        serializer_allocate_if_required (serializer, size_1);
    }

    memcpy (serializer->memory + serializer->index, data, size);
    serializer->index += size;
}

void __attribute__ ((noinline))
serializer_allocate_if_required (serializer_t *serializer, long additional_size)
{
    size_t copy = serializer->index;
    serializer->size *= 1.4;
    serializer->size += additional_size;

    char *address = realloc (serializer->memory, serializer->size);

    if (NULL == address)
    {
        address = malloc (serializer->size);
        assert (address != NULL);

        memcpy (address, serializer->memory, copy);
        free (serializer->memory); /* free the original memory space after copy */
    }
    // This should be no longer needed if everything else is correct.
    // else
    // {
    //     /* zero the memory after list->count elements */
    //     memset (address + copy, 0, serializer->size - copy);
    // }
    serializer->memory = address;
}

__attribute__ ((always_inline)) inline void
serializer_add_type (
  serializer_t *serializer, serialization_types_t type, bool_t has_key, long size)
{
    // First four bits will be the type of the data
    // Next byte is for the byte order lsf(0) or msf(1)
    // Next byte is used to check if the its a key value pair or an indexed value
    // And the last 3 bits are used for the size of the raw data information

    unsigned char value = (type & 0xF0) | (is_little_endian > 0) << 3;
    value |= (has_key > 0) << 2;
    value |= serializer_get_typesize (type);

    serializer->memory[serializer->index++] = value;
}

__attribute__ ((always_inline)) inline int
serializer_get_typesize (serialization_types_t type)
{
    if (type != SERIALIZATION_TYPE_BLOB)
    {
        return type & 0X0F;
    }
    else
    {
        return 3;
    }
}