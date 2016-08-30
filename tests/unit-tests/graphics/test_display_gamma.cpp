/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Brandon Schaefer <brandon.schaefer@gmail.com>
 */

#include "mir/graphics/display_gamma.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace mg = mir::graphics;

namespace
{
std::vector<uint16_t> const r{1};
std::vector<uint16_t> const g{2};
std::vector<uint16_t> const b{3};
}

class MockDisplayGamma : public testing::Test
{
public:
    MockDisplayGamma() :
        gamma(r, g, b)
    {
    }

    mg::DisplayGamma gamma;
};

TEST_F(MockDisplayGamma, test_uint16_display_gamma_size)
{
    EXPECT_THAT(gamma.size(), r.size());
}

TEST_F(MockDisplayGamma, test_uint16_display_gamma_rgb_correct)
{
    ASSERT_THAT(gamma.size(), gamma.size());
    EXPECT_THAT(gamma.red()[0], r[0]);
    EXPECT_THAT(gamma.green()[0], g[0]);
    EXPECT_THAT(gamma.blue()[0], b[0]);
}

TEST_F(MockDisplayGamma, test_display_gamma_copy_ctor)
{
    mg::DisplayGamma gamma_second(gamma);

    ASSERT_THAT(gamma_second.size(), gamma.size());
    EXPECT_THAT(gamma_second.red()[0], gamma.red()[0]);
    EXPECT_THAT(gamma_second.green()[0], gamma.green()[0]);
    EXPECT_THAT(gamma_second.blue()[0], gamma.blue()[0]);
}

TEST_F(MockDisplayGamma, test_display_gamma_assign_copy_ctor)
{
    mg::DisplayGamma gamma_second = gamma;

    ASSERT_THAT(gamma_second.size(), gamma.size());
    EXPECT_THAT(gamma_second.red()[0], gamma.red()[0]);
    EXPECT_THAT(gamma_second.green()[0], gamma.green()[0]);
    EXPECT_THAT(gamma_second.blue()[0], gamma.blue()[0]);
}

TEST_F(MockDisplayGamma, test_display_gamma_move_ctor)
{
    mg::DisplayGamma gamma_second(mg::DisplayGamma(r, g, b));

    ASSERT_THAT(gamma_second.size(), r.size());
    EXPECT_THAT(gamma_second.red()[0], r[0]);
    EXPECT_THAT(gamma_second.green()[0], g[0]);
    EXPECT_THAT(gamma_second.blue()[0], b[0]);
}

TEST_F(MockDisplayGamma, test_display_gamma_assign_move_ctor)
{
    mg::DisplayGamma gamma_second = mg::DisplayGamma(r, g, b);

    ASSERT_THAT(gamma_second.size(), r.size());
    EXPECT_THAT(gamma_second.red()[0], r[0]);
    EXPECT_THAT(gamma_second.green()[0], g[0]);
    EXPECT_THAT(gamma_second.blue()[0], b[0]);
}

TEST_F(MockDisplayGamma, test_display_gamma_move_vector_ctor)
{
    mg::DisplayGamma gamma_second = mg::DisplayGamma(std::vector<uint16_t>{1}, 
                                                     std::vector<uint16_t>{2},
                                                     std::vector<uint16_t>{3});

    ASSERT_THAT(gamma_second.size(), r.size());
    EXPECT_THAT(gamma_second.red()[0], r[0]);
    EXPECT_THAT(gamma_second.green()[0], g[0]);
    EXPECT_THAT(gamma_second.blue()[0], b[0]);
}

TEST(DisplayGammaEmpty, test_display_gamma_empty)
{
    mg::DisplayGamma gamma;

    EXPECT_THAT(gamma.size(), 0);
}
