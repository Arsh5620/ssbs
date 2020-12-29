import io
import os
import deserialize
import serialize

if __name__ == "__main__":
    file_name = "serialized"
    if (os.path.isfile(file_name)):
        input_file = open(file_name, "rb")
        byte_data = input_file.read()

        deserialized = deserialize.deserialize_all(byte_data)

        for value in deserialized:
            print("Key is {}, and value is {}".format(value.key, value.value))

    serialize_memory = bytearray(0)
    serialize.serializer_add(
        serialize_memory, "some key", 111.00)
    serialize.serializer_add_eof(serialize_memory)
    file = open("pythonserialize", "wb")
    file.write(serialize_memory)
    file.close()

    print(serialize_memory)
