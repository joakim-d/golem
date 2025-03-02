#include <core/Common.h>

#include <algorithm>
#include <array>

namespace
{
static const std::array<unsigned, 12 * 6> PULSE_PERIODS = {
    44,   156,  262,  362,  457,  546,  631,  710,  785,  856,  923,  986,
    1046, 1102, 1155, 1205, 1252, 1297, 1339, 1379, 1416, 1452, 1485, 1517,
    1547, 1575, 1601, 1626, 1650, 1672, 1693, 1713, 1732, 1750, 1766, 1782,
    1797, 1811, 1824, 1837, 1849, 1860, 1870, 1880, 1890, 1899, 1907, 1915,
    1922, 1929, 1936, 1942, 1948, 1954, 1959, 1964, 1969, 1973, 1977, 1981,
    1985, 1988, 1992, 1995, 1998, 2001, 2003, 2006, 2008, 2010, 2012, 2014
};

static const std::array<unsigned, 12 * 6> WAVE_PERIODS = {
    1046, 1102, 1155, 1205, 1252, 1297, 1339, 1379, 1416, 1452, 1485, 1517,
    1547, 1575, 1601, 1626, 1650, 1672, 1693, 1713, 1732, 1750, 1766, 1782,
    1797, 1811, 1824, 1837, 1849, 1860, 1870, 1880, 1890, 1899, 1907, 1915,
    1922, 1929, 1936, 1942, 1948, 1954, 1959, 1964, 1969, 1973, 1977, 1981,
    1985, 1988, 1992, 1995, 1998, 2001, 2003, 2006, 2008, 2010, 2012, 2014,
    2016, 2018, 2020, 2021, 2023, 2024, 2025, 2027, 2028, 2029, 2030, 2031,
};

}

unsigned noteToPulsePeriod(domain::NoteFrequency note)
{
    return PULSE_PERIODS[std::clamp<unsigned>(
        (unsigned)note, 0, PULSE_PERIODS.size() - 1)];
}

unsigned noteToWavePeriod(domain::NoteFrequency note)
{
    return WAVE_PERIODS[std::clamp<unsigned>(
        (unsigned)note, 0, PULSE_PERIODS.size() - 1)];
}
