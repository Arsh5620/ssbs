#include "serializer.h"
#include "deserializer.h"
#include <time.h>
#include <gnu/libc-version.h>

#define CUSTOM_STRUCT_TYPE_CODE 0xF00D;

struct custom_struct_hack
{
    // If you need to add struct id then make sure that its is the first field in the struct
    int struct_type;
    int value1;
    int value2;
    int value3;
    char *string1;
    char *string2;
    double double1;
    float float1;
    char char1;
};

int
main (int argc, char **argv)
{
    printf ("GNU libc version: %s\n", gnu_get_libc_version ());
    char *string
      = "This is supposed to be a big text string that can be tested by adding to the serializer";
    int string_length = strlen (string);

    clock_t time1 = clock ();
    // Serializing in memory
    serializer_t serializer = serializer_init (4096);

    struct custom_struct_hack hack1;
    // When deserializing multiple structs you can check the first field to see the id of
    // the struct, but this information is only needed by your application and can be avoided if you
    // are only serializing a struct of one type
    //
    // You can check it like so *(int*)((void*)struct_address) == CUSTOM_STRUCT_TYPE_CODE
    hack1.struct_type = CUSTOM_STRUCT_TYPE_CODE;

    char *memory = "struct hack\0\0\0\0struct hack\0\0\0\0struct hack\0\0\0\0\0\0";
    char mem_strlen = 48;

    char buffer[mem_strlen];
    memcpy (buffer, memory, mem_strlen);

    serializer_reset (&serializer);
    for (int i = 0; i < 1200999; ++i)
    {
        serializer_add_int (&serializer, "my integer\0", 12, 9087987);
        serializer_add_blob (&serializer, "my string\0\0", 12, string, string_length);
        serializer_add_char (&serializer, "my character\0\0\0", 16, 'X');
        serializer_add_short (&serializer, "my short\0\0\0", 12, 65200);
        serializer_add_long (&serializer, "my long", 8, 878979879879879879);
        serializer_add_float (&serializer, "my float\0\0\0", 12, 10000.123456);
        serializer_add_double (&serializer, "my double\0\0", 12, 10000000.123456768);

        // This is how you can serialize structs, but when deserializing you will have to know
        // the type of the struct as this library would only give you binary data which
        // you will have interpret yourself as a struct, you can add the "id" to your struct
        // as the first field.
        serializer_add_blob (
          &serializer, buffer, mem_strlen, (char *) &hack1, sizeof (struct custom_struct_hack));

        // if (i % 100000)
        // {
        //     serializer_reset (&serializer);
        // }
    }

    serializer_add_eof (&serializer);

    printf ("Serializing total clocks taken: %ld\n", clock () - time1);
    // time1 = clock ();
    // FILE *file = fopen ("serialized", "wb");
    // fwrite (serializer.memory, serializer.index, 1, file);
    // fflush (file);
    // fclose (file);
    return 0;
    my_list_s list = my_list_new (4096, sizeof (deserializer_value_t));

    deserializer_t deserializer
      = deserializer_init (serializer.memory, serializer.index, SERIALIZER_TRUE);
    deserialize_all (&deserializer, &list);

    serializer_free (serializer);
    deserializer_free (deserializer);
    my_list_free (list);
    printf ("Deserializing total clocks taken: %ld\n", clock () - time1);
}