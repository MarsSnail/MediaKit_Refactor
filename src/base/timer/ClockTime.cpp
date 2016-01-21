/*
 * Copyright (C) 2013, 2014 Mark Li (lixiaonan06@163.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "ClockTime.h"

#include "boost/date_time/microsec_time_clock.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>  

namespace MediaCore {
#include <Windows.h>
#if defined(_WIN32) || defined(WIN32)
	uint64_t getTicks() {
		static boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
		boost::posix_time::time_duration time_from_epoch =
			boost::posix_time::microsec_clock::universal_time() - epoch;
		return time_from_epoch.total_milliseconds();
	}
#else  // not the windows platform
#include <sys/time.h>
// unit millonSecond
uint64_t getTicks() {
  struct timeval tv;

  gettimeofday(&tv, 0);
  uint64_t result = static_cast<uint64_t>(tv.tv_sec) * 1000000L;
  // time Uint: microsecond
  result += tv.tv_usec;

  return static_cast<uint64_t>(result / 1000.0);
}
#endif  // not windows platform

}  // namespace
