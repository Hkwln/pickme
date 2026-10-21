#pragma once

#include <stdio.h>
/*input: a list of numbers, your number
 * output: pos of the number in the list
 * (the previous num on the position is smaller)
 *
 * */
size_t bin_search(const int list[], size_t size, int number);
