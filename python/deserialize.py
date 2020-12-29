from collections import namedtuple
import serializetypes
import struct

deserializer_value = namedtuple(
    "deserializer_value", "read_type key value index index_absolute")


def deserialize_all(byte_data):
    """
    Use this function to binary deserialize all the stream
    """
    list = []
    index = 0
    size = len(byte_data)

    while (index < size):
        parse, index = deserialize_next(byte_data, index, size)
        if (parse.read_type == serializetypes.SERIALIZATION_TYPE_EOF):
            break
        list.append(parse)

    return list


def deserialize_next(byte_data, index, size):
    magic = byte_data[index]
    read_type = (magic >> 4 & 0b1111)
    little_endian = (magic >> 3 & 0b1) == 1
    has_key = (magic >> 2 & 0b1) == 1
    byte_size = 1 << (magic & 0b11)
    index += 1

    if (read_type == serializetypes.SERIALIZATION_TYPE_EOF):
        return deserializer_value(serializetypes.SERIALIZATION_TYPE_EOF, 0, 0, 0, 0), index

    key = 0
    if (has_key):
        key_length = byte_data[index]
        index += 1
        key = byte_data[index: index + key_length].decode("ascii")
        index += key_length

    value = byte_data[index: index + byte_size]
    index += byte_size

    if (read_type == serializetypes.SERIALIZATION_TYPE_BLOB):
        long_buffer = bytearray(8)
        long_buffer[0:len(value)] = value
        if (little_endian == True):
            size_value = struct.unpack("<q", long_buffer)[0]
        else:
            size_value = struct.unpack(">q", long_buffer)[0]
        value = byte_data[index: index + size_value]
        index += size_value
    else:
        value = deserialize_primitive(little_endian, read_type, value)

    return deserializer_value(read_type, key, value, 0, 0), index


def deserialize_primitive(little_endian, type, value):
    endianess = "<" if little_endian else ">"
    if (type == serializetypes.SERIALIZATION_TYPE_CHAR):
        return struct.unpack(endianess + "c", value)[0]
    elif (type == serializetypes.SERIALIZATION_TYPE_SHORT):
        return struct.unpack(endianess + "h", value)[0]
    elif (type == serializetypes.SERIALIZATION_TYPE_INT):
        return struct.unpack(endianess + "i", value)[0]
    elif (type == serializetypes.SERIALIZATION_TYPE_LONG):
        return struct.unpack(endianess + "q", value)[0]
    elif (type == serializetypes.SERIALIZATION_TYPE_FLOAT):
        return struct.unpack(endianess + "f", value)[0]
    elif(type == serializetypes.SERIALIZATION_TYPE_DOUBLE):
        return struct.unpack(endianess + "d", value)[0]
