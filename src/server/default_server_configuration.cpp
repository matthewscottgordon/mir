/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "mir/default_server_configuration.h"
#include "mir/abnormal_exit.h"
#include "mir/asio_main_loop.h"
#include "mir/default_server_status_listener.h"

#include "mir/options/program_option.h"
#include "mir/frontend/null_message_processor_report.h"
#include "mir/frontend/session_authorizer.h"
#include "mir/shell/surface_configurator.h"
#include "mir/graphics/cursor.h"
#include "mir/shell/null_session_listener.h"
#include "mir/graphics/display.h"
#include "mir/input/cursor_listener.h"
#include "mir/input/nested_input_configuration.h"
#include "mir/input/null_input_configuration.h"
#include "mir/input/null_input_report.h"
#include "mir/input/display_input_region.h"
#include "mir/input/event_filter_chain.h"
#include "mir/input/nested_input_relay.h"
#include "mir/input/vt_filter.h"
#include "mir/input/android/default_android_input_configuration.h"
#include "mir/input/input_manager.h"
#include "mir/logging/logger.h"
#include "mir/logging/input_report.h"
#include "mir/logging/dumb_console_logger.h"
#include "mir/logging/glog_logger.h"
#include "mir/logging/message_processor_report.h"
#include "mir/lttng/message_processor_report.h"
#include "mir/lttng/input_report.h"
#include "mir/time/high_resolution_clock.h"
#include "mir/geometry/rectangles.h"
#include "mir/default_configuration.h"

#include <map>

namespace mc = mir::compositor;
namespace geom = mir::geometry;
namespace mf = mir::frontend;
namespace mg = mir::graphics;
namespace ml = mir::logging;
namespace ms = mir::surfaces;
namespace msh = mir::shell;
namespace mi = mir::input;
namespace mia = mi::android;

mir::DefaultServerConfiguration::DefaultServerConfiguration(int argc, char const* argv[]) :
    DefaultConfigurationOptions(argc, argv),
    default_filter(std::make_shared<mi::VTFilter>())
{
}


std::string mir::DefaultServerConfiguration::the_socket_file() const
{
    auto socket_file = the_options()->get(server_socket_opt, mir::default_server_socket);

    // Record this for any children that want to know how to connect to us.
    // By both listening to this env var on startup and resetting it here,
    // we make it easier to nest Mir servers.
    setenv("MIR_SOCKET", socket_file.c_str(), 1);

    return socket_file;
}

std::shared_ptr<msh::SessionListener>
mir::DefaultServerConfiguration::the_shell_session_listener()
{
    return shell_session_listener(
        [this]
        {
            return std::make_shared<msh::NullSessionListener>();
        });
}

std::shared_ptr<mi::CompositeEventFilter>
mir::DefaultServerConfiguration::the_composite_event_filter()
{
    return composite_event_filter(
        [this]() -> std::shared_ptr<mi::CompositeEventFilter>
        {
            std::initializer_list<std::shared_ptr<mi::EventFilter> const> filter_list {default_filter};
            return std::make_shared<mi::EventFilterChain>(filter_list);
        });
}

std::shared_ptr<mi::InputReport>
mir::DefaultServerConfiguration::the_input_report()
{
    return input_report(
        [this]() -> std::shared_ptr<mi::InputReport>
        {
            auto opt = the_options()->get(input_report_opt, off_opt_value);
            
            if (opt == log_opt_value)
            {
                return std::make_shared<ml::InputReport>(the_logger());
            }
            else if (opt == lttng_opt_value)
            {
                return std::make_shared<mir::lttng::InputReport>();
            }
            else
            {
                return std::make_shared<mi::NullInputReport>();
            }
        });
}

std::shared_ptr<mi::CursorListener>
mir::DefaultServerConfiguration::the_cursor_listener()
{
    struct DefaultCursorListener : mi::CursorListener
    {
        DefaultCursorListener(std::weak_ptr<mg::Cursor> const& cursor) :
            cursor(cursor)
        {
        }

        void cursor_moved_to(float abs_x, float abs_y)
        {
            if (auto c = cursor.lock())
            {
                c->move_to(geom::Point{abs_x, abs_y});
            }
        }

        std::weak_ptr<mg::Cursor> const cursor;
    };
    return cursor_listener(
        [this]() -> std::shared_ptr<mi::CursorListener>
        {
            return std::make_shared<DefaultCursorListener>(the_display()->the_cursor());
        });
}

std::shared_ptr<mi::InputConfiguration>
mir::DefaultServerConfiguration::the_input_configuration()
{
    return input_configuration(
    [this]() -> std::shared_ptr<mi::InputConfiguration>
    {
        auto const options = the_options();
        if (!options->get("enable-input", enable_input_default))
        {
            return std::make_shared<mi::NullInputConfiguration>();
        }
        // TODO (default-nested): don't fallback to standalone if host socket is unset in 14.04
        else if (options->is_set(standalone_opt) || !options->is_set(host_socket_opt))
        {
            return std::make_shared<mia::DefaultInputConfiguration>(
                the_composite_event_filter(),
                the_input_region(),
                the_cursor_listener(),
                the_input_report());
        }
        else
        {
            return std::make_shared<mi::NestedInputConfiguration>(
                the_nested_input_relay(),
                the_composite_event_filter(),
                the_input_region(),
                the_cursor_listener(),
                the_input_report());
        }
    });
}

std::shared_ptr<mi::InputManager>
mir::DefaultServerConfiguration::the_input_manager()
{
    return input_manager(
        [&, this]() -> std::shared_ptr<mi::InputManager>
        {
            if (the_options()->get(legacy_input_report_opt, off_opt_value) == log_opt_value)
                    ml::legacy_input_report::initialize(the_logger());
            return the_input_configuration()->the_input_manager();
        });
}

std::shared_ptr<mi::InputRegion> mir::DefaultServerConfiguration::the_input_region()
{
    return input_region(
        [this]()
        {
            return std::make_shared<mi::DisplayInputRegion>(the_display());
        });
}

std::shared_ptr<msh::SurfaceConfigurator> mir::DefaultServerConfiguration::the_shell_surface_configurator()
{
    struct DefaultSurfaceConfigurator : public msh::SurfaceConfigurator
    {
        int select_attribute_value(msh::Surface const&, MirSurfaceAttrib, int requested_value)
        {
            return requested_value;
        }
        void attribute_set(msh::Surface const&, MirSurfaceAttrib, int)
        {
        }
    };
    return shell_surface_configurator(
        [this]()
        {
            return std::make_shared<DefaultSurfaceConfigurator>();
        });
}

std::shared_ptr<mf::SessionAuthorizer>
mir::DefaultServerConfiguration::the_session_authorizer()
{
    struct DefaultSessionAuthorizer : public mf::SessionAuthorizer
    {
        bool connection_is_allowed(pid_t /* pid */)
        {
            return true;
        }

        bool configure_display_is_allowed(pid_t /* pid */)
        {
            return true;
        }
    };
    return session_authorizer(
        [&]()
        {
            return std::make_shared<DefaultSessionAuthorizer>();
        });
}

std::shared_ptr<mf::MessageProcessorReport>
mir::DefaultServerConfiguration::the_message_processor_report()
{
    return message_processor_report(
        [this]() -> std::shared_ptr<mf::MessageProcessorReport>
        {
            auto mp_report = the_options()->get(msg_processor_report_opt, off_opt_value);
            if (mp_report == log_opt_value)
            {
                return std::make_shared<ml::MessageProcessorReport>(the_logger(), the_time_source());
            }
            else if (mp_report == lttng_opt_value)
            {
                return std::make_shared<mir::lttng::MessageProcessorReport>();
            }
            else
            {
                return std::make_shared<mf::NullMessageProcessorReport>();
            }
        });
}


std::shared_ptr<ml::Logger> mir::DefaultServerConfiguration::the_logger()
{
    return logger(
        [this]() -> std::shared_ptr<ml::Logger>
        {
            if (the_options()->is_set(glog))
            {
                return std::make_shared<ml::GlogLogger>(
                    "mir",
                    the_options()->get(glog_stderrthreshold, glog_stderrthreshold_default),
                    the_options()->get(glog_minloglevel, glog_minloglevel_default),
                    the_options()->get(glog_log_dir, glog_log_dir_default));
            }
            else
            {
                return std::make_shared<ml::DumbConsoleLogger>();
            }
        });
}

std::shared_ptr<mi::InputChannelFactory> mir::DefaultServerConfiguration::the_input_channel_factory()
{
    return the_input_manager();
}

std::shared_ptr<msh::InputTargeter> mir::DefaultServerConfiguration::the_input_targeter()
{
    return input_targeter(
        [&]() -> std::shared_ptr<msh::InputTargeter>
        {
            return the_input_configuration()->the_input_targeter();
        });
}

std::shared_ptr<ms::InputRegistrar> mir::DefaultServerConfiguration::the_input_registrar()
{
    return input_registrar(
        [&]() -> std::shared_ptr<ms::InputRegistrar>
        {
            return the_input_configuration()->the_input_registrar();
        });
}

std::shared_ptr<mir::time::TimeSource> mir::DefaultServerConfiguration::the_time_source()
{
    return time_source(
        []()
        {
            return std::make_shared<mir::time::HighResolutionClock>();
        });
}

std::shared_ptr<mir::MainLoop> mir::DefaultServerConfiguration::the_main_loop()
{
    return main_loop(
        []()
        {
            return std::make_shared<mir::AsioMainLoop>();
        });
}


std::shared_ptr<mir::ServerStatusListener> mir::DefaultServerConfiguration::the_server_status_listener()
{
    return server_status_listener(
        []()
        {
            return std::make_shared<mir::DefaultServerStatusListener>();
        });
}

auto mir::DefaultServerConfiguration::the_nested_input_relay()
-> std::shared_ptr<mi::NestedInputRelay>
{
    return nested_input_relay([]{ return std::make_shared<mi::NestedInputRelay>(); });
}
