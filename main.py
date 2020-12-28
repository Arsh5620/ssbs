import io
import deserialize

if __name__ == "__main__":
    input_file = open("serialized", "rb")
    byte_data = input_file.read()

    deserialized = deserialize.deserialize_all(byte_data)

    for value in deserialized:
        print("Key is {}, and value is {}".format(value.key, value.value))
