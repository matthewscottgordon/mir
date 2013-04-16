/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by:
 *   Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "hwc10_device.h"
#include "hwc_layerlist.h"
#include "fb_device.h"
#include <boost/throw_exception.hpp>
#include <stdexcept>

namespace mga=mir::graphics::android;
namespace geom=mir::geometry;

TEST_F(HWC10Device, test_hwc_gles_set_empty_layerlist)
{
    using namespace testing;

    mga::HWC10Device device(mock_device, mock_organizer, mock_fbdev);

    mga::LayerList empty_list;
    EXPECT_CALL(*mock_organizer, native_list())
        .Times(1)
        .WillOnce(ReturnRef(empty_list));
    EXPECT_CALL(*mock_device, set_interface(mock_device.get(), HWC_NUM_DISPLAY_TYPES, _))
        .Times(1);

    device.commit_frame();

    EXPECT_EQ(empty_list.size(), mock_device->display0_content.numHwLayers);

    EXPECT_EQ(-1, mock_device->display0_content.retireFenceFd);
}

TEST_F(HWC10Device, test_hwc_gles_set_gets_layerlist)
{
    using namespace testing;

    mga::HWC10Device device(mock_device, mock_organizer, mock_fbdev);

    mga::LayerList fb_list;
    fb_list.push_back(std::make_shared<HWCDummyLayer>());

    EXPECT_CALL(*mock_organizer, native_list())
        .Times(1)
        .WillOnce(ReturnRef(fb_list));
    EXPECT_CALL(*mock_device, set_interface(mock_device.get(), HWC_NUM_DISPLAY_TYPES, _))
        .Times(1);

    device.commit_frame();

    EXPECT_EQ(fb_list.size(), mock_device->display0_content.numHwLayers);
}

TEST_F(HWC10Device, test_hwc_gles_set_error)
{
    using namespace testing;

    mga::HWC10Device device(mock_device, mock_organizer, mock_fbdev);
    mga::LayerList fb_list;
    fb_list.push_back(std::make_shared<HWCDummyLayer>());

    EXPECT_CALL(*mock_organizer, native_list())
        .Times(1)
        .WillOnce(ReturnRef(fb_list));
    EXPECT_CALL(*mock_device, set_interface(mock_device.get(), HWC_NUM_DISPLAY_TYPES, _))
        .Times(1)
        .WillOnce(Return(-1));

    EXPECT_THROW({
        device.commit_frame();
    }, std::runtime_error);
}

TEST_F(HWC10Device, test_hwc_device_display_config)
{
    using namespace testing;

    unsigned int hwc_configs = 0xA1;
    EXPECT_CALL(*mock_device, getDisplayConfigs_interface(mock_device.get(),HWC_DISPLAY_PRIMARY,_,Pointee(1)))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<2>(hwc_configs), Return(0)));

    mga::HWC10Device device(mock_device, mock_organizer, mock_fbdev);
}

//apparently this can happen if the display is in the 'unplugged state'
TEST_F(HWC10Device, test_hwc_device_display_config_failure_throws)
{
    using namespace testing;

    EXPECT_CALL(*mock_device, getDisplayConfigs_interface(mock_device.get(),HWC_DISPLAY_PRIMARY,_,_))
        .Times(1)
        .WillOnce(Return(-1));

    EXPECT_THROW({
        mga::HWC10Device device(mock_device, mock_organizer, mock_fbdev);
    }, std::runtime_error);
}

namespace
{
static int const display_width = 180;
static int const display_height = 1010101;

static int display_attribute_handler(struct hwc_composer_device_1*, int, uint32_t,
                                     const uint32_t* attribute_list, int32_t* values)
{
    EXPECT_EQ(attribute_list[0], HWC_DISPLAY_WIDTH);
    EXPECT_EQ(attribute_list[1], HWC_DISPLAY_HEIGHT);
    EXPECT_EQ(attribute_list[2], HWC_DISPLAY_NO_ATTRIBUTE);

    values[0] = display_width;
    values[1] = display_height;
    return 0;
}

TEST_F(HWC10Device, test_hwc_device_display_width_height)
{
    using namespace testing;

    int hwc_configs = 0xA1;
    EXPECT_CALL(*mock_device, getDisplayConfigs_interface(mock_device.get(),HWC_DISPLAY_PRIMARY,_,_))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<2>(hwc_configs), Return(0)));

    mga::HWC10Device device(mock_device, mock_organizer, mock_fbdev);
 
    EXPECT_CALL(*mock_device, getDisplayAttributes_interface(mock_device.get(), HWC_DISPLAY_PRIMARY,hwc_configs,_,_))
        .Times(1)
        .WillOnce(Invoke(display_attribute_handler));

    auto size = device.display_size();
    EXPECT_EQ(size.width.as_uint32_t(),  static_cast<unsigned int>(display_width));
    EXPECT_EQ(size.height.as_uint32_t(), static_cast<unsigned int>(display_height));
}
}

