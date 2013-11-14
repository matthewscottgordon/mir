/*
 * Copyright © 2013 Canonical Ltd.
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
 * Authored by: Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 */

#include "mir_test_framework/udev_environment.h"
#include "src/server/graphics/gbm/udev_wrapper.h"

#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>

// At some point gnu libstdc++ will have a <regex> header that contains
// actual functions. Until then, Boost to the rescue.
#include <boost/regex.hpp>

#include <umockdev.h>
#include <libudev.h>

namespace mg=mir::graphics;
namespace mgg=mir::graphics::gbm;
namespace mtf=mir::mir_test_framework;

class UdevWrapperTest : public ::testing::Test
{
public:
    mtf::UdevEnvironment udev_environment;
};

TEST_F(UdevWrapperTest, IteratesOverCorrectNumberOfDevices)
{
    udev_environment.add_device("drm", "fakedev1", NULL, {}, {});
    udev_environment.add_device("drm", "fakedev2", NULL, {}, {});
    udev_environment.add_device("drm", "fakedev3", NULL, {}, {});
    udev_environment.add_device("drm", "fakedev4", NULL, {}, {});
    udev_environment.add_device("drm", "fakedev5", NULL, {}, {});

    auto ctx = std::make_shared<mgg::UdevContext>();
    mgg::UdevEnumerator enumerator(ctx);

    enumerator.scan_devices();

    int device_count = 0;
    for (auto& device : enumerator)
    {
        // Silence unused variable warning
        static_cast<void>(device);
        ++device_count;
    }

    ASSERT_EQ(device_count, 5);
}

TEST_F(UdevWrapperTest, EnumeratorMatchSubsystemIncludesCorrectDevices)
{
    udev_environment.add_device("drm", "fakedrm1", NULL, {}, {});
    udev_environment.add_device("scsi", "fakescsi1", NULL, {}, {});
    udev_environment.add_device("drm", "fakedrm2", NULL, {}, {});
    udev_environment.add_device("usb", "fakeusb1", NULL, {}, {});
    udev_environment.add_device("usb", "fakeusb2", NULL, {}, {});

    auto ctx = std::make_shared<mgg::UdevContext>();
    mgg::UdevEnumerator devices(ctx);

    devices.match_subsystem("drm");
    devices.scan_devices();
    for (auto& device : devices)
    {
        ASSERT_STREQ("drm", device.subsystem());
    }
}

TEST_F(UdevWrapperTest, UdevDeviceHasCorrectDevType)
{
    auto sysfs_path = udev_environment.add_device("drm", "card0", NULL, {}, {"DEVTYPE", "drm_minor"});

    auto ctx = std::make_shared<mgg::UdevContext>();

    mgg::UdevDevice dev(ctx, sysfs_path);
    ASSERT_STREQ("drm_minor", dev.devtype());
}

TEST_F(UdevWrapperTest, UdevDeviceHasCorrectDevPath)
{
    auto sysfs_path = udev_environment.add_device("drm", "card0", NULL, {}, {});

    auto ctx = std::make_shared<mgg::UdevContext>();

    mgg::UdevDevice dev(ctx, sysfs_path);
    ASSERT_STREQ("/devices/card0", dev.devpath());
}

TEST_F(UdevWrapperTest, UdevDeviceHasCorrectDevNode)
{
    auto sysfs_path = udev_environment.add_device("drm", "card0", NULL, {}, {"DEVNAME", "/dev/dri/card0"});

    auto ctx = std::make_shared<mgg::UdevContext>();
    mgg::UdevDevice card0(ctx, sysfs_path);

    ASSERT_STREQ("/dev/dri/card0", card0.devnode());
}

TEST_F(UdevWrapperTest, UdevDeviceComparisonIsReflexive)
{
    auto sysfs_path = udev_environment.add_device("drm", "card0", NULL, {}, {});

    auto ctx = std::make_shared<mgg::UdevContext>();
    mgg::UdevDevice dev(ctx, sysfs_path);

    EXPECT_TRUE(dev == dev);
}

TEST_F(UdevWrapperTest, UdevDeviceComparisonIsSymmetric)
{
    auto sysfs_path = udev_environment.add_device("drm", "card0", NULL, {}, {});

    auto ctx = std::make_shared<mgg::UdevContext>();
    mgg::UdevDevice same_one(ctx, sysfs_path);
    mgg::UdevDevice same_two(ctx, sysfs_path);

    EXPECT_TRUE(same_one == same_two);
    EXPECT_TRUE(same_two == same_one);
}

TEST_F(UdevWrapperTest, UdevDeviceDifferentDevicesCompareFalse)
{
    auto path_one = udev_environment.add_device("drm", "card0", NULL, {}, {});
    auto path_two = udev_environment.add_device("drm", "card1", NULL, {}, {});

    auto ctx = std::make_shared<mgg::UdevContext>();
    mgg::UdevDevice dev_one(ctx, path_one);
    mgg::UdevDevice dev_two(ctx, path_two);

    EXPECT_FALSE(dev_one == dev_two);
    EXPECT_FALSE(dev_two == dev_one);
}

TEST_F(UdevWrapperTest, UdevDeviceDifferentDevicesAreNotEqual)
{
    auto path_one = udev_environment.add_device("drm", "card0", NULL, {}, {});
    auto path_two = udev_environment.add_device("drm", "card1", NULL, {}, {});

    auto ctx = std::make_shared<mgg::UdevContext>();
    mgg::UdevDevice dev_one(ctx, path_one);
    mgg::UdevDevice dev_two(ctx, path_two);

    EXPECT_TRUE(dev_one != dev_two);
    EXPECT_TRUE(dev_two != dev_one);    
}

TEST_F(UdevWrapperTest, UdevDeviceSameDeviceIsNotNotEqual)
{
    auto sysfs_path = udev_environment.add_device("drm", "card0", NULL, {}, {});

    auto ctx = std::make_shared<mgg::UdevContext>();
    mgg::UdevDevice same_one(ctx, sysfs_path);
    mgg::UdevDevice same_two(ctx, sysfs_path);

    EXPECT_FALSE(same_one != same_two);
    EXPECT_FALSE(same_two != same_one);
}

TEST_F(UdevWrapperTest, EnumeratorMatchParentMatchesOnlyChildren)
{
    auto card0_syspath = udev_environment.add_device("drm", "card0", NULL, {}, {});
    udev_environment.add_device("usb", "fakeusb", NULL, {}, {});

    udev_environment.add_device("drm", "card0-HDMI1", "/sys/devices/card0", {}, {});
    udev_environment.add_device("drm", "card0-VGA1", "/sys/devices/card0", {}, {});
    udev_environment.add_device("drm", "card0-LVDS1", "/sys/devices/card0", {}, {});

    auto ctx = std::make_shared<mgg::UdevContext>();

    mgg::UdevEnumerator devices(ctx);
    mgg::UdevDevice drm_device(ctx, card0_syspath);

    devices.match_parent(drm_device);
    devices.scan_devices();

    int child_count = 0;
    for (auto& device : devices)
    {
        EXPECT_STREQ("drm", device.subsystem());
        ++child_count;
    }
    EXPECT_EQ(4, child_count);
}

TEST_F(UdevWrapperTest, EnumeratorThrowsLogicErrorIfIteratedBeforeScanned)
{
    auto ctx = std::make_shared<mgg::UdevContext>();

    mgg::UdevEnumerator devices(ctx);

    EXPECT_THROW({ devices.begin(); },
                 std::logic_error);
}

TEST_F(UdevWrapperTest, EnumeratorLogicErrorHasSensibleMessage)
{
    auto ctx = std::make_shared<mgg::UdevContext>();

    mgg::UdevEnumerator devices(ctx);
    std::string error_msg;

    try {
        devices.begin();
    }
    catch (std::logic_error& e)
    {
        error_msg = e.what();
    }
    EXPECT_STREQ("Attempted to iterate over udev devices without first scanning", error_msg.c_str());
}

TEST_F(UdevWrapperTest, EnumeratorEnumeratesEmptyList)
{
    auto ctx = std::make_shared<mgg::UdevContext>();

    mgg::UdevEnumerator devices(ctx);

    devices.scan_devices();

    for (auto& device : devices)
        ADD_FAILURE() << "Unexpected udev device: " << device.devpath();
}

TEST_F(UdevWrapperTest, EnumeratorAddMatchSysnameIncludesCorrectDevices)
{
    auto drm_sysfspath = udev_environment.add_device("drm", "card0", NULL, {}, {});
    udev_environment.add_device("drm", "control64D", NULL, {}, {});
    udev_environment.add_device("drm", "card0-LVDS1", drm_sysfspath.c_str(), {}, {});
    udev_environment.add_device("drm", "card1", NULL, {}, {});

    auto ctx = std::make_shared<mgg::UdevContext>();

    mgg::UdevEnumerator devices(ctx);

    devices.match_sysname("card[0-9]");
    devices.scan_devices();
    for (auto& device : devices)
    {
        EXPECT_TRUE(boost::regex_match(device.devpath(), boost::regex(".*card[0-9].*")))
            << "Unexpected device with devpath:" << device.devpath();
    }
}

TEST_F(UdevWrapperTest, UdevMonitorDoesNotTriggerBeforeEnabling)
{
    auto ctx = std::make_shared<mgg::UdevContext>();

    auto monitor = mgg::UdevMonitor(ctx);
    bool event_handler_called = false;

    udev_environment.add_device("drm", "control64D", NULL, {}, {});

    monitor.process_events([&event_handler_called](mgg::UdevMonitor::EventType,
                                                   mgg::UdevDevice const&) {event_handler_called = true;});

    EXPECT_FALSE(event_handler_called);
}

TEST_F(UdevWrapperTest, UdevMonitorTriggersAfterEnabling)
{
    auto ctx = std::make_shared<mgg::UdevContext>();

    auto monitor = mgg::UdevMonitor(ctx);
    bool event_handler_called = false;

    monitor.enable();

    udev_environment.add_device("drm", "control64D", NULL, {}, {});

    monitor.process_events([&event_handler_called](mgg::UdevMonitor::EventType,
                                                   mgg::UdevDevice const&) {event_handler_called = true;});

    EXPECT_TRUE(event_handler_called);
}

TEST_F(UdevWrapperTest, UdevMonitorSendsRemoveEvent)
{
    auto ctx = std::make_shared<mgg::UdevContext>();

    auto monitor = mgg::UdevMonitor(ctx);
    bool remove_event_received = false;

    monitor.enable();

    auto test_sysfspath = udev_environment.add_device("drm", "control64D", NULL, {}, {});
    udev_environment.remove_device(test_sysfspath);

    monitor.process_events([&remove_event_received]
        (mgg::UdevMonitor::EventType action, mgg::UdevDevice const&)
            {
                if (action == mgg::UdevMonitor::EventType::REMOVED)
                    remove_event_received = true;
            });

    EXPECT_TRUE(remove_event_received);
}

TEST_F(UdevWrapperTest, UdevMonitorSendsChangedEvent)
{
    auto ctx = std::make_shared<mgg::UdevContext>();

    auto monitor = mgg::UdevMonitor(ctx);
    bool changed_event_received = false;

    monitor.enable();

    auto test_sysfspath = udev_environment.add_device("drm", "control64D", NULL, {}, {});
    udev_environment.emit_device_changed(test_sysfspath);

    monitor.process_events([&changed_event_received]
        (mgg::UdevMonitor::EventType action, mgg::UdevDevice const&)
            {
                if (action == mgg::UdevMonitor::EventType::CHANGED)
                    changed_event_received = true;
            });

    EXPECT_TRUE(changed_event_received);    
}
