/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

/*
ref_dx12
2019 Bleeding Eye Studios
*/

#include "dx12_local.hpp"

namespace dx12
{
	std::unique_ptr<Log> log = nullptr;
}

dx12::Log::Log()
{
	LOG_FUNC();

	m_initialized = false;

	try
	{
		boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");
		boost::log::core::get()->add_global_attribute("Scope", boost::log::attributes::named_scope());

		boost::log::add_common_attributes();

		auto logSink = boost::log::add_file_log
		(
			boost::log::keywords::file_name = LOG_FILE_NAME,                                        /*< file name pattern >*/
			boost::log::keywords::target = LOG_PATH,
			boost::log::keywords::rotation_size = 10 * 1024 * 1024,                                   /*< rotate files every 10 MiB... >*/
			boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0), /*< ...or at midnight >*/
			boost::log::keywords::auto_flush = true,
			boost::log::keywords::format = "[%TimeStamp%] <%Severity%> %Scope% - %Message%"
		);

		//logSink->locked_backend()->auto_flush(true);

		boost::log::add_console_log();

		boost::log::core::get()->set_filter
		(
#ifdef _DEBUG
			boost::log::trivial::severity >= debug
#else
			boost::log::trivial::severity >= info
#endif
		);

#ifdef _DEBUG

		// Courtesy https://stackoverflow.com/questions/48242414/boostlog-output-to-visual-studio-output-console-adding-extra-lf-cr-to-format
		// Add Visual Studio logging
		boost::shared_ptr< boost::log::core > core = boost::log::core::get();

		// Create the sink. The backend requires synchronization in the frontend.
		boost::shared_ptr< boost::log::sinks::synchronous_sink< boost::log::sinks::debug_output_backend > > debuggerSink(new boost::log::sinks::synchronous_sink< boost::log::sinks::debug_output_backend >);

		// Set the special filter to the frontend
		// in order to skip the sink when no debugger is available
		debuggerSink->set_filter(boost::log::expressions::is_debugger_present());

		debuggerSink->set_formatter
		(
			boost::log::expressions::format("%1%: [%2%] - %3%\r\n")
			% boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
			% boost::log::trivial::severity
			% boost::log::expressions::smessage
		);
#endif

		BOOST_LOG_TRIVIAL(info) << "Log Created Successfully.";

		m_initialized = true;

		LOG(info) << "nullptr address: " << static_cast<void*>(nullptr);

		logSink->flush();
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}
