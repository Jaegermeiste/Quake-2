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

dx11::Web::Web()
{
	LOG_FUNC();

	LOG(info) << "Initializing";

}

bool dx11::Web::Initialize()
{
	LOG_FUNC();

	return true;
}

bool dx11::Web::DownloadFile(std::string downloadURL, std::string destinationPath)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	hr = URLDownloadToFile(NULL, downloadURL.c_str(), destinationPath.c_str(), 0, NULL);

	if (hr == E_OUTOFMEMORY) {
		LOG(error) << "Download Failed: Buffer length invalid, or insufficient memory";
		return false;
	}
	else if (hr == INET_E_DOWNLOAD_FAILURE) {
		LOG(error) << "Download Failed: URL is invalid";
		return false;
	}
	else if (FAILED(hr)) {
		LOG(error) << "Other error: " << hr;
		return false;
	}

	return true;
}

void dx11::Web::Shutdown()
{
	LOG_FUNC();

	LOG(info) << "Shutting down.";

	LOG(info) << "Shutdown complete.";
}
