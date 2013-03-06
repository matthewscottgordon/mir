/*
 * Copyright © 2012 Canonical Ltd.
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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

class AndroidInterpreterTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }
};

#if 0
//tests that should go somewhere else
TEST_F(AndroidInterpreterTest, native_window_dequeue_calls_surface_get_current)
{
    using namespace testing;
    ANativeWindow* anw;
    ANativeWindowBuffer* tmp;

    EXPECT_CALL(*mock_surface, get_current_buffer())
        .Times(1)
        .WillOnce(Return(mock_client_buffer));
    anw = new mcla::MirNativeWindow(mock_surface.get());

    anw->dequeueBuffer_DEPRECATED(anw, &tmp);

    delete anw;
}

TEST_F(AndroidInterpreterTest, native_window_dequeue_gets_native_handle_from_returned_buffer)
{
    using namespace testing;
    ANativeWindow* anw;
    native_handle_t handle;
    ANativeWindowBuffer buffer;
    buffer.handle = &handle;

    ANativeWindowBuffer* tmp;

    EXPECT_CALL(*mock_client_buffer, get_native_handle())
        .Times(1)
        .WillOnce(Return(&buffer));
    EXPECT_CALL(*mock_surface, get_current_buffer())
        .Times(1)
        .WillOnce(Return(mock_client_buffer));

    anw = new mcla::MirNativeWindow(mock_surface.get());

    anw->dequeueBuffer_DEPRECATED(anw, &tmp);

    EXPECT_EQ(tmp, &buffer);
    delete anw;
}

TEST_F(AndroidInterpreterTest, native_window_dequeue_has_proper_rc)
{
    using namespace testing;
    ANativeWindow* anw;

    ANativeWindowBuffer* tmp;

    anw = new mcla::MirNativeWindow(mock_surface.get());

    auto ret = anw->dequeueBuffer_DEPRECATED(anw, &tmp);
    EXPECT_EQ(0, ret);

    delete anw;
}


TEST_F(AndroidInterpreterTest, native_window_queue_advances_buffer)
{
    using namespace testing;
    ANativeWindow* anw;
    ANativeWindowBuffer* tmp = 0x0;

    EXPECT_CALL(*mock_surface, next_buffer(_,_))
        .Times(1);
    anw = new mcla::MirNativeWindow(mock_surface.get());

    anw->queueBuffer_DEPRECATED(anw, tmp);

    delete anw;
}

/* format is an int that is set by the driver. these are not the HAL_PIXEL_FORMATS in android */
TEST_F(AndroidInterpreterTest, native_window_perform_remembers_format)
{
    ANativeWindow* anw;
    int format = 945;

    anw = new mcla::MirNativeWindow(mock_surface.get());

    anw->perform(anw, NATIVE_WINDOW_SET_BUFFERS_FORMAT , format);

    int tmp_format = 0;
    anw->query(anw, NATIVE_WINDOW_FORMAT, &tmp_format);

    EXPECT_EQ(tmp_format, format);

    delete anw;
}

TEST_F(AndroidInterpreterTest, native_window_hint_query_hook)
{
    using namespace testing;
    ANativeWindow* anw;
    /* transform hint is a bitmask of a few options for rotation/flipping buffer. a value
       of zero is no transform */
    int transform_hint_zero = 0;
    int value;

    anw = new mcla::MirNativeWindow(mock_surface.get());

    auto rc = anw->query(anw, NATIVE_WINDOW_TRANSFORM_HINT ,&value);

    EXPECT_EQ(0, rc);
    EXPECT_EQ(transform_hint_zero, value);

    delete anw;
}

TEST_F(AndroidInterpreterTest, native_window_default_width_query_hook)
{
    using namespace testing;
    ANativeWindow* anw;
    int value;

    anw = new mcla::MirNativeWindow(mock_surface.get());

    auto rc = anw->query(anw, NATIVE_WINDOW_DEFAULT_WIDTH ,&value);

    EXPECT_EQ(0, rc);
    EXPECT_EQ(surf_params.width, value);

    delete anw;
}

TEST_F(AndroidInterpreterTest, native_window_default_height_query_hook)
{
    using namespace testing;
    ANativeWindow* anw;
    int value;

    anw = new mcla::MirNativeWindow(mock_surface.get());

    auto rc = anw->query(anw, NATIVE_WINDOW_DEFAULT_HEIGHT ,&value);

    EXPECT_EQ(0, rc);
    EXPECT_EQ(surf_params.height, value);

    delete anw;
}

TEST_F(AndroidInterpreterTest, native_window_width_query_hook)
{
    using namespace testing;
    ANativeWindow* anw;
    int value;

    anw = new mcla::MirNativeWindow(mock_surface.get());

    EXPECT_CALL(*mock_surface, get_parameters())
        .Times(1);

    auto rc = anw->query(anw, NATIVE_WINDOW_WIDTH ,&value);

    EXPECT_EQ(0, rc);
    EXPECT_EQ(surf_params.width, value);

    delete anw;
}

TEST_F(AndroidInterpreterTest, native_window_height_query_hook)
{
    using namespace testing;
    ANativeWindow* anw;
    int value;

    anw = new mcla::MirNativeWindow(mock_surface.get());
    EXPECT_CALL(*mock_surface, get_parameters())
        .Times(1);

    auto rc = anw->query(anw, NATIVE_WINDOW_HEIGHT ,&value);

    EXPECT_EQ(0, rc);
    EXPECT_EQ(surf_params.height, value);

    delete anw;
}
#endif
