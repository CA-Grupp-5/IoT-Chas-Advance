#include "unity.h"
#include "MockNetwork.h"
#include "MockClient.h"
#include "WiFiMockType.h"

void setUp() {}
void tearDown() {}

void test_connect_on_ready() {
    MockNetwork wifi;
    MockClient client;
    int result = client.connect(0x4A7D2B63, 5000);
    TEST_ASSERT_EQUAL(MOCK_WL_CONNECTED, result);
}

// test connection attempt is made with SSID and password
// test connection status is checked repeatedly until WL_CONNECTED
// test connection timeout (30 seconds)

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_connect_on_ready);
    return UNITY_END();
}
