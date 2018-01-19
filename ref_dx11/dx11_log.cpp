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
ref_dx11
2017 Bleeding Eye Studios
*/

#include "dx11_local.hpp"

namespace dx11
{
	std::unique_ptr<Log> log = nullptr;
}

dx11::Log::Log()
{
	//BOOST_LOG_NAMED_SCOPE("Log");
	LOG_FUNC();

	boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");
	boost::log::core::get()->add_global_attribute("Scope", boost::log::attributes::named_scope());

	boost::log::add_common_attributes();

	auto logSink = boost::log::add_file_log
	(
		boost::log::keywords::file_name = LOG_FILE_NAME,                                        /*< file name pattern >*/
		boost::log::keywords::target = LOG_PATH,
		boost::log::keywords::auto_flush = true,
		boost::log::keywords::format = "[%TimeStamp%]: <%Severity%> %Scope% %Message%"
	);

	boost::log::add_console_log();

	boost::log::core::get()->set_filter
	(
		boost::log::trivial::severity >= info
	);

	BOOST_LOG_TRIVIAL(info) << "Log Created Successfully.";

	logSink->flush();
}
