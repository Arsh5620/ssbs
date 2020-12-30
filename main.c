#include "serializer.h"
#include "deserializer.h"
#include <time.h>

#ifdef SSBS_DEBUG
int
main (int argc, char **argv)
{
    char *string = "f227631695ad97c6ae49a030f525fcaca592d84df963ad158d487783be9bccb604eb0e9696c97d8"
                   "119481fd56863ffc98b4a82de714666a2add29dc75df60331";
    int string_length = strlen (string);

    serializer_t serializer = serializer_init ();
    my_list_s list = my_list_new (16, sizeof (deserializer_value_t));

    serializer_reset (&serializer);
    for (int i = 0; i < 10000; ++i)
    {
        serializer_add_blob (&serializer, "NULL", 4, string, string_length);
    }

    serializer_add_eof (&serializer);

    FILE *file = fopen ("serialized", "rb+");
    fwrite (serializer.memory, serializer.index, 1, file);
    fflush (file);
    fclose (file);

    deserializer_t deserializer = deserializer_init (serializer.memory, serializer.index);

    list.count = 0;
    my_list_s result = deserialize_all (&deserializer, list);
}
#endif