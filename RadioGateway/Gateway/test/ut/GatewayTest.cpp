#include "GatewayTester.hpp"

TEST(RadioGateway, ReassemblyAndUartOwnership) {
    RadioGateway::GatewayTester tester;
    tester.testReassemblyAndUartOwnership();
}

TEST(RadioGateway, DropsIncompleteFrame) {
    RadioGateway::GatewayTester tester;
    tester.testMissingFragment();
}

TEST(RadioGateway, ReportsDiagnostics) {
    RadioGateway::GatewayTester tester;
    tester.testDiagnostics();
}

TEST(RadioGateway, RejectsMixedAndInvalidFragments) {
    RadioGateway::GatewayTester tester;
    tester.testMixedAndInvalidFragments();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
