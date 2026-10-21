#ifdef TEST

#include <stdlib.h>

#include "helper.h"
#include "mock_logic.h"
#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}
/* TODO: more test's*/
void test_bin_search(void)
{
    int test[6] = {100, 45, 33, 29, 15, 5};
    size_t res = bin_search(test, 6, 36);
    printf("%zu\n", res);
    TEST_ASSERT_TRUE(2 == res);
    int arr[6] = {9, 9, 5, 4, 4, 2};
    TEST_ASSERT_TRUE(2 == bin_search(arr, 6, 7));
}

#endif  // TEST
