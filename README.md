# Super Simple Binary Serializer

This is a really simple binary serializer which supports serializing basic C data types to in-memory appropriate for writing to files or sending over network.

- It does not support random access for deserialization.
- It does support both index-based and key value pairs serialization.
- Also it does not copies the strings using memcpy but only provides you the address and size in the original memory location.
- Supported Types:
  - char, short, int, long
  - float, double
  - blobs
  - structs - with hacks

See [this file](./main.c) for example usage

NOTE: the python version of the serialization is not compatible with the C version
