#include <string.h>

#include <zephyr/ztest.h>
#include <zephyr/random/rand32.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(random_tests);

#define TOP_NUMBER 12

ZTEST_SUITE(random_tests, NULL, NULL, NULL, NULL, NULL);

/**
 * @brief Test encoding of sensor data
 *
 * Tests the encoding of single and multiple sensor data packages
 *
 */
ZTEST(random_tests, test_random_number_between_1_and_x)
{

	uint32_t num = sys_rand32_get() % (TOP_NUMBER + 1);
	printk("Random: %i\n", num);
	zassert_between_inclusive(num, 1, TOP_NUMBER);
}
