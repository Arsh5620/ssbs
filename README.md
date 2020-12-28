# Super simple binary serializer

This is a really simple serializer that only supports basic inbuilt C data types namely (char, short, int, long, float, double and blob).<BR>
The serializer supports index based serialization along with key-value pair serialization.

So if you want to serialize the values 'A', 100, 22/7 and key value pair "Hello":"World!", all you have to do is:

>serializer_t serializer = serializer_init();<BR>
>serializer_add_char(&serializer, NULL, 'A');<BR>
>serializer_add_int(&serializer, NULL, 100);<BR>
>serializer_add_double(&serializer, NULL, 22/7);<BR>
>serializer_add_blob(&serializer, "Hello", "World!");<BR>
>serializer_add_eof(&serializer);<BR>

This would serialize all the data to the serializer_t buffers which you will have to use manually. 

In the deserialization end all you do is:<BR>
>deserializer_t deserializer = deserializer_init(memory, size);<BR>
>my_list_s deserialized_list = deserialize_all(&deserializer);<BR>

And you can iterate over all the values in the my_list_s with my_list_get(&deserialized_list, index) which are of the type deserializer_value_t;
