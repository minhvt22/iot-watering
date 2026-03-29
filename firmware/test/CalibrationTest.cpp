/**
 * @file CalibrationTest.cpp
 * @brief Unit tests for moisture sensor calibration and scaling logic.
 *
 * Target: Native (for CI/CD verification of business logic)
 */

#include "Hardware.h"
#include <unity.h>

// Mock Sensor Values
const int SENSOR_AIR_DRY = 4095;   // 12-bit ADC Max
const int SENSOR_WATER_WET = 1200; // 12-bit ADC Min for capacitive sensor

void test_moisture_scaling_dry() {
  // Input: 4095 (Air dry)
  // Expected Output: 0%
  float moisture = Hardware::calculateMoisturePercentage(4095);
  TEST_ASSERT_EQUAL_FLOAT(0.0, moisture);
}

void test_moisture_scaling_wet() {
  // Input: 1200 (Submerged)
  // Expected Output: 100%
  float moisture = Hardware::calculateMoisturePercentage(1200);
  TEST_ASSERT_EQUAL_FLOAT(100.0, moisture);
}

void test_moisture_scaling_midpoint() {
  // Input: (4095 + 1200) / 2 = 2647
  // Expected Output: 50%
  float moisture = Hardware::calculateMoisturePercentage(2647);
  TEST_ASSERT_EQUAL_FLOAT(50.0, moisture);
}

void test_invalid_ranges() {
  // Ensure values above AIR_DRY are capped at 0%
  TEST_ASSERT_EQUAL_FLOAT(0.0, Hardware::calculateMoisturePercentage(5000));

  // Ensure values below WATER_WET are capped at 100%
  TEST_ASSERT_EQUAL_FLOAT(100.0, Hardware::calculateMoisturePercentage(100));
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_moisture_scaling_dry);
  RUN_TEST(test_moisture_scaling_wet);
  RUN_TEST(test_moisture_scaling_midpoint);
  RUN_TEST(test_invalid_ranges);
  return UNITY_END();
}
