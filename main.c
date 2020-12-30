#include "serializer.h"
#include "deserializer.h"
#include <time.h>

int
main (int argc, char **argv)
{
    char *string = "some value!!";

    serializer_t serializer = serializer_init ();
    my_list_s list = my_list_new (16, sizeof (deserializer_value_t));

    for (int i = 0; i < 100000; ++i)
    {
        serializer_reset (&serializer);
        serializer_add_blob (&serializer, "NULL", 4, string, 13);
        serializer_add_eof (&serializer);

        deserializer_t deserializer = deserializer_init (serializer.memory, serializer.index);

        list.count = 0;
        my_list_s result = deserialize_all (&deserializer, list);
    }
}