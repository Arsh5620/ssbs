#ifndef SERIALIZER_H_
#define SERIALIZER_H_
#include <string.h>
#include <malloc.h>

typedef char bool_t;

typedef struct binary_serializer
{
    char *memory;
    long index;
    long size;
} serializer_t;

enum binary_serializer_result
{
    SERIALIZED,
    FAILED_SERIALIZING
};

typedef struct serializer_key
{
    char *string;
    char size;
} serializer_key_t;

// Cannot have more than 16 types, as we are using 4 bits for this field
typedef enum binary_serialization_types
{
    SERIALIZATION_TYPE_NONE = 0x00, // mostly similar to blob, but not actually a type
    SERIALIZATION_TYPE_EOF = 0x10,
    SERIALIZATION_TYPE_CHAR = 0x20,
    SERIALIZATION_TYPE_SHORT = 0x31,
    SERIALIZATION_TYPE_INT = 0x42,
    SERIALIZATION_TYPE_LONG = 0x53,
    SERIALIZATION_TYPE_FLOAT = 0x62,
    SERIALIZATION_TYPE_DOUBLE = 0x73,
    SERIALIZATION_TYPE_BLOB = 0x80,
} serialization_types_t;

// Keep in mind that we need to add 1 to all of these values to get byte size
#define SIZE_BYTE 0
#define SIZE_2BYTES 1
#define SIZE_4BYTES 2
#define SIZE_8BYTES 3
#define SERIALIZER_TRUE 1
#define SERIALIZER_FALSE 0

serializer_t
serializer_init (long default_size);
void
serializer_free (serializer_t serializer);
void
serializer_reset (serializer_t *serializer);

void
serializer_add_char (serializer_t *serializer, char *key, int key_length, char value);
void
serializer_add_short (serializer_t *serializer, char *key, int key_length, short value);
void
serializer_add_int (serializer_t *serializer, char *key, int key_length, int value);
void
serializer_add_long (serializer_t *serializer, char *key, int key_length, long value);
void
serializer_add_float (serializer_t *serializer, char *key, int key_length, float value);
void
serializer_add_double (serializer_t *serializer, char *key, int key_length, double value);
void
serializer_add_blob (
  serializer_t *serializer, char *key, unsigned char key_length, char *value, long size);
void
serializer_add_eof (serializer_t *serializer);

bool_t
endianness_test ();

void
serializer_add_key (serializer_t *serializer, char *key, unsigned char key_length);
int
serializer_get_typesize (serialization_types_t type);
void
serializer_add_type (
  serializer_t *serializer, serialization_types_t type, bool_t has_key, long size);
void
serializer_add_binary (serializer_t *serializer, serialization_types_t type, char *data, long size);
void
serializer_allocate_if_required (serializer_t *serializer, long additional_size);
#endif