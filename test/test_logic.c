
#ifdef TEST

#include "logic.h"
#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}
void test_get_next_word(void)
{
    str_list* cheeses = malloc(sizeof(str_list));
    sl_init(cheeses);
    file_listing(cheeses);
    size_t d = 0;
    char* word = get_word(cheeses, d++);
    while (word != NULL) {
        printf("%s\n", word);
        word = get_word(cheeses, d++);
    }
    sl_free(cheeses);
    // if there are no errors: test passes
    TEST_ASSERT_TRUE(1);
}

#endif  // TEST
