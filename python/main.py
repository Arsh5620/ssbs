import io
import os
import deserialize
import serialize

if __name__ == "__main__":
    for i in range(0, 100000):
        serialize_memory = bytearray(0)
        serialize.serializer_add(
            serialize_memory, "some key", 111.00)
        serialize.serializer_add_eof(serialize_memory)

        deserialized = deserialize.deserialize_all(serialize_memory)

    print(deserialized)
