from collections import namedtuple
import sys
import struct
from python import serializetypes

is_little_endian = sys.byteorder == 'little'


def serializer_add(serializer_memory, key, value):
    type = serializer_check_type(value)
    if isinstance(value, str):
        serializer_add_type(
            serializer_memory, type, key is not None, len(value))
    else:
        serializer_add_type(
            serializer_memory, type, key is not None, value)
    serializer_add_key(
        serializer_memory, key)

    if (type == serializetypes.SERIALIZATION_TYPE_BLOB):
        serializer_memory += serializer_pack(len(value))

    if isinstance(value, int):
        serializer_memory += value.to_bytes(1 <<
                                            serializer_get_intsize(value), sys.byteorder)
    elif isinstance(value, float):
        serializer_memory += struct.pack("d", value)
    elif isinstance(value, str):
        serializer_memory += (value.encode())

def serializer_add_eof(serializer_memory):
    serializer_memory.append(serializetypes.SERIALIZATION_TYPE_EOF << 4)

def serializer_add_key(serializer_memory, key):
    if not key is None:
        serializer_memory += len(key).to_bytes(1, sys.byteorder)
        serializer_memory += (key.encode())


def serializer_check_type(value):
    if isinstance(value, int):
        size = serializer_get_intsize(value)
        if size == 0:
            return serializetypes.SERIALIZATION_TYPE_CHAR
        elif size == 1:
            return serializetypes.SERIALIZATION_TYPE_SHORT
        elif size == 2:
            return serializetypes.SERIALIZATION_TYPE_INT
        elif size == 3:
            return serializetypes.SERIALIZATION_TYPE_LONG
    elif isinstance(value, float):
        return serializetypes.SERIALIZATION_TYPE_DOUBLE
    else:
        return serializetypes.SERIALIZATION_TYPE_BLOB


def serializer_pack(value):
    return value.to_bytes(8, sys.byteorder)


def serializer_add_type(serializer_memory, type, has_key, size):
    value = type << 4
    value |= (1 if is_little_endian else 0) << 3
    value |= (1 if has_key else 0) << 2
    value |= (serializer_get_typesize(type, size) & 0b11)

    serializer_memory.append(value)


def serializer_get_typesize(type, size):
    if (type == serializetypes.SERIALIZATION_TYPE_CHAR):
        return serializetypes.SIZE_BYTE
    elif (type == serializetypes.SERIALIZATION_TYPE_SHORT):
        return serializetypes.SIZE_2BYTES
    elif type == serializetypes.SERIALIZATION_TYPE_INT or type == serializetypes.SERIALIZATION_TYPE_FLOAT:
        return serializetypes.SIZE_4BYTES
    elif type == serializetypes.SERIALIZATION_TYPE_LONG or type == serializetypes.SERIALIZATION_TYPE_DOUBLE:
        return serializetypes.SIZE_8BYTES
    elif type == serializetypes.SERIALIZATION_TYPE_BLOB:
        return 3


# In python this is used to determine the size of integer
def serializer_get_intsize(size):
    if (size < 0x100):
        return 0
    elif (size < 0x10000):
        return 1
    elif (size < 0x1000000):
        return 2
    else:
        return 3
