#include <string.h>

#include <zephyr/ztest.h>

#include <lib/codec/example_json_codec.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(codec_tests);

ZTEST_SUITE(codec_tests, NULL, NULL, NULL, NULL, NULL);

/**
 * @brief Test encoding of sensor data
 *
 * Tests the encoding of single and multiple sensor data packages
 *
 */
ZTEST(codec_tests, test_encoding_of_sensor_data)
{
	int res;

	struct example_json_payload payload =
		{
			.timestamp = 1234,
			.sensor1_value = 112233,
			.sensor2_value = {
				.x_value = 1,
				.y_value = 2,
				.z_value = -9,
			},
		};

	uint8_t buf[128];
	uint8_t small_buf[8];

	/* Test successful */
	res = example_json_codec_encode(&payload, buf, sizeof(buf));
	LOG_INF("Encoded size: %i", res);
	LOG_INF("%s", (char *)buf);
	zassert_equal(res, 110);

	char *payload_encoded = "{\"timestamp\":1234,\"sensor1_value\":112233,\"sensor2_value\":{\"x_value\":1,\"y_value\":2,\"z_value\":-9}}";

	/* Compare strings */
	res = strncmp(payload_encoded, buf, res);
	zassert_equal(res, 0);

	/* Not enough memory provided */
	res = example_json_codec_encode(&payload, small_buf, sizeof(small_buf));
	zassert_equal(res, -ENOMEM);
}
