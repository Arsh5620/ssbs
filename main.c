#include "serializer.h"
#include "deserializer.h"
#include <time.h>

int
main (int argc, char **argv)
{
    // for (int i = 0; i < 1000000; ++i)
    {
        float random = rand () * 22.0f / 7.0f;
        long long_value = rand () * rand ();
        char *string = "some value!!";

        serializer_t serializer = serializer_init ();
        serializer_add_float (&serializer, "NULL", random);
        serializer_add_long (&serializer, NULL, long_value);
        serializer_add_blob (&serializer, "Something more", string, strlen (string));
        serializer_add_eof (&serializer);

        FILE *file = fopen ("serialized", "wb+");
        fwrite (serializer.memory, sizeof(char), serializer.index, file);
        fflush (file);
        fclose (file);

        deserializer_t deserializer = deserializer_init (serializer.memory, serializer.index);

        volatile my_list_s result = deserialize_all (&deserializer);
        volatile deserializer_value_t *first_result
          = (deserializer_value_t *) my_list_get (result, 0);
        volatile deserializer_value_t *second_result
          = (deserializer_value_t *) my_list_get (result, 1);
        volatile deserializer_value_t *third_result
          = (deserializer_value_t *) my_list_get (result, 2);

        float result_float = *(float *) &first_result->value_1;
        if (result_float != random)
        {
            printf ("FAILED on float, expected %f, found %f\n", random, result_float);
        }

        if (second_result->value_1 != long_value)
        {
            printf (
              "FAILED on long, expected %ld, found %ld\n", long_value, second_result->value_1);
        }

        if (memcmp (third_result->value_2, string, strlen (string)))
        {
            printf ("FAILED on string !!\n");
        }

        my_list_free (result);
        serializer_free (serializer);
    }
}