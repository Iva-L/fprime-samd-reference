#include "RadioSupportTester.hpp"

TEST(RadioSupport, BufferOwnership) {
    CuriosityReference::RadioSupportTester tester;
    tester.testBufferOwnership();
}

TEST(RadioSupport, StartupFrequency) {
    CuriosityReference::RadioSupportTester tester;
    tester.testStartupFrequency();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
