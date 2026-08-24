// Copyright (C) 2026 The remage developers
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <https://www.gnu.org/licenses/>.

// Basic unit tests for pure helpers that do not need a full Geant4 run manager.

#include <cstdlib>
#include <iostream>
#include <string>

#include "RMGGeneratorUtil.hh"
#include "RMGTools.hh"

namespace remage_test {
  enum class Color {
    kRed,
    kGreen,
    kBlue
  };
} // namespace remage_test

namespace {

  using remage_test::Color;

  int g_failures = 0;

  void expect_true(bool cond, const char* msg) {
    if (!cond) {
      std::cerr << "FAIL: " << msg << std::endl;
      ++g_failures;
    }
  }

  void test_break_down_elapsed_time() {
    auto t0 = RMGTools::BreakDownElapsedTime(0);
    expect_true(t0.days == 0 && t0.hours == 0 && t0.minutes == 0 && t0.seconds == 0, "zero");

    auto t1 = RMGTools::BreakDownElapsedTime(3661); // 1h 1m 1s
    expect_true(t1.days == 0 && t1.hours == 1 && t1.minutes == 1 && t1.seconds == 1, "3661s");

    auto t2 = RMGTools::BreakDownElapsedTime(90061); // 1d 1h 1m 1s
    expect_true(t2.days == 1 && t2.hours == 1 && t2.minutes == 1 && t2.seconds == 1, "90061s");

    auto t3 = RMGTools::BreakDownElapsedTime(59);
    expect_true(t3.days == 0 && t3.hours == 0 && t3.minutes == 0 && t3.seconds == 59, "59s");
  }

  void test_is_sampleable() {
    expect_true(RMGGeneratorUtil::IsSampleable("G4Box"), "G4Box");
    expect_true(RMGGeneratorUtil::IsSampleable("G4Orb"), "G4Orb");
    expect_true(RMGGeneratorUtil::IsSampleable("G4Sphere"), "G4Sphere");
    expect_true(RMGGeneratorUtil::IsSampleable("G4Tubs"), "G4Tubs");
    expect_true(!RMGGeneratorUtil::IsSampleable("G4Torus"), "G4Torus unsupported");
    expect_true(!RMGGeneratorUtil::IsSampleable(""), "empty unsupported");
  }

  void test_enum_helpers() {
    expect_true(RMGTools::ToEnum<Color>("Red") == Color::kRed, "ToEnum Red");
    expect_true(RMGTools::ToEnum<Color>("kGreen") == Color::kGreen, "ToEnum kGreen");
    expect_true(RMGTools::GetCandidate(Color::kBlue) == "Blue", "GetCandidate");

    auto cand = RMGTools::GetCandidates<Color>(' ');
    expect_true(cand.find("Red") != std::string::npos, "candidates has Red");
    expect_true(cand.find("Green") != std::string::npos, "candidates has Green");
    expect_true(cand.find("Blue") != std::string::npos, "candidates has Blue");

    try {
      RMGTools::ToEnum<Color>("Purple");
      expect_true(false, "ToEnum Purple should throw");
    } catch (const std::bad_cast&) {
      // expected
    }
  }
} // namespace

int main() {
  test_break_down_elapsed_time();
  test_is_sampleable();
  test_enum_helpers();

  if (g_failures != 0) {
    std::cerr << g_failures << " failure(s)" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "all unit tests passed" << std::endl;
  return EXIT_SUCCESS;
}
