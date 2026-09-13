
#ifdef TEST

#include "fuzzy.h"
#include "unity.h"

/* fuzzy.c calls get_word() from logic.c. Ceedling links only the source
 * matching this test's name, so we link the generated mock instead of the
 * real logic module (nothing here calls get_word, it only needs to exist). */
#include "mock_logic.h"

void setUp(void)
{
}

void tearDown(void)
{
}
void test_levenshtein(void)
{
    // sanity check
    TEST_ASSERT_TRUE(levenshtein_distance("fernseher", "fernseher") == 0);
    TEST_ASSERT_TRUE(levenshtein_distance("cat", "cut") == 1);
    TEST_ASSERT_TRUE(levenshtein_distance("cats", "cat") == 1);
    TEST_ASSERT_TRUE(levenshtein_distance("cat", "cats") == 1);
    TEST_ASSERT_TRUE(levenshtein_distance("kitten", "sitting") == 3);
    TEST_ASSERT_TRUE(levenshtein_distance("ab", "ba") == 2);
    TEST_ASSERT_TRUE(levenshtein_distance("cat", "cats") == 1);
    TEST_ASSERT_TRUE(levenshtein_distance("cat", "cats") == 1);
}
/*
void test_fuzzy_search(void)
{
    TEST_ASSERT_TRUE(strcmp("Fernseher", find_near_match("test.txt")) == 0);
}
*/

#endif  // TEST
