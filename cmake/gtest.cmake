# GoogleTest: provides GTest::gtest, GTest::gtest_main, GTest::gmock and GTest::gmock_main.

include_guard(GLOBAL)

include(FetchDependency)

golem_fetch_dependency(googletest
  URL https://github.com/google/googletest/releases/download/v1.18.0/googletest-1.18.0.tar.gz
  URL_HASH SHA256=6e3191c1455468b3fc35a417fb565c1c5071aee1b7e7f85e30cf48a98d37d8b5
  OPTIONS
    "INSTALL_GTEST OFF"
    # Match the MSVC runtime of our targets (/MD) instead of gtest's static default.
    "gtest_force_shared_crt ON")

include(GoogleTest)
