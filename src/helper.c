#include "../include/helper.h"

size_t bin_search(const int arr[], size_t size, int number)
{
    size_t left = 0, right = size;
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        if (arr[mid] >= number) {
            left = mid + 1;
        } else
            right = mid;
    }
    return left;
}
