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

#include "curl_adapter.h"
#include "net/io_channel.h"
#include "base/snail_exception.h"
#include "base/timer/wall_clock_timer.h"
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <cstdio>

using std::auto_ptr;
using std::string;
using std::map;
using std::cout;
using std::endl;
using MediaCore::WallClockTimer;

namespace net {

/******************************************************************
 * Statics and CurlStreamFile implementation
 *
 *****************************************************************/

// static private
size_t CurlStreamFile::recv(void *buf, size_t size, size_t nmemb, void *userp) {
  CurlStreamFile *stream = static_cast<CurlStreamFile *>(userp);
  return stream->cache(buf, size * nmemb);
}
std::streamsize CurlStreamFile::cache(void *from, std::streamsize size) {
  // take note of current position
  long curr_pos = std::ftell(_cache);

  // seek to the end
  std::fseek(_cache, 0, SEEK_END);

  std::streamsize wrote = std::fwrite(from, 1, size, _cache);
  if (wrote < 1) {
    throw base::SnailException(string("writting to cache file failed"));
  }
  // set the size of cache data
  _cached = std::ftell(_cache);
  // reset the position for next read
  std::fseek(_cache, curr_pos, SEEK_SET);
  return wrote;
}

void CurlStreamFile::fillCacheNonBlocking() {
  if (!_running) {
    return;
  }
  CURLMcode mcode;
  do {
    mcode = curl_multi_perform(_mCurlHandle, &_running);
  } while (mcode == CURLM_CALL_MULTI_PERFORM);
  if (mcode != CURLM_OK) {
    throw base::SnailException(curl_multi_strerror(mcode));
  }
  processMessages();
}

void CurlStreamFile::fillCache(std::streampos size) {
#if 1
  assert(size >= 0);
  if (!_running || _cached >= size) {
    return;
  }
  fd_set readfd, writefd, exceptfd;
  int maxfd;
  CURLMcode mcode;
  timeval tv;
  // hard-coded slect timeout
  // this number is kept low to give more thread switch
  // opportunities while waitting for a load
  const long maxSleepUsec = 10000;  // 1/100 of a second

  const unsigned int userTimeout = 60000;
  WallClockTimer lastProgress;
  while (_running) {
    fillCacheNonBlocking();
    if (_cached >= size || !_running) break;

    FD_ZERO(&readfd);
    FD_ZERO(&writefd);
    FD_ZERO(&exceptfd);
    mcode =
        curl_multi_fdset(_mCurlHandle, &readfd, &writefd, &exceptfd, &maxfd);
    if (mcode != CURLM_OK) {
      throw base::SnailException(curl_multi_strerror(mcode));
    }
    if (maxfd < 0) {
      // as of libcurl 7.21.x, the DNS resolving appears to be
      // going on in the background, so curl_multi_fdset fails to
      // return anything useful, So we use the user timeout value
      // to give DNS enough time to resolve the lookup
      if (userTimeout && lastProgress.Elapsed() > userTimeout) {
        return;
      } else {
        continue;
      }
    }  // if(maxfd<0)
    tv.tv_sec = 0;
    tv.tv_usec = maxSleepUsec;
    // wait for data on the filedescriptors until a timeout set in rc file
    int ret = select(maxfd + 1, &readfd, &writefd, &exceptfd, &tv);
#if !defined(WIN32)
    if (ret == -1) {
      if (errno == EINTR) {
        cout << "select() was interrupted by a singal" << endl;
        ret = 0;
      } else {
        std::ostringstream os;
        os << "error polling data from connection to" << _url << ":"
           << strerror(errno);
        throw base::SnailException(os.str());
      }
    }
#endif
    if (!ret) {
      // timeout check the clock to see
      // if we expired the user timeout
      if (userTimeout && lastProgress.Elapsed() > userTimeout) {
        cout << "timeout (" << userTimeout << ") while loading from URL" << _url
             << endl;
        return;
      }
    } else {
      lastProgress.Restart();
    }
  }  // while(....
  processMessages();
#endif
}
void CurlStreamFile::processMessages() {
  CURLMsg *curl_msg;

  // the number of messages left in the queue (not used by us)
  int msgs;
  while ((curl_msg = curl_multi_info_read(_mCurlHandle, &msgs))) {
    if (curl_msg->msg == CURLMSG_DONE) {
      // http transaction succeeded
      if (curl_msg->data.result == CURLE_OK) {
        long code;
        // check http response
        curl_easy_getinfo(curl_msg->easy_handle, CURLINFO_RESPONSE_CODE, &code);
        if (code >= 400) {
          cout << "error:http respose" << code << "from URL" << _url << endl;
          _error = true;
          _running = false;
        } else {
        }  // if(code>=400)
      } else {
        cout << __FILE__ << __LINE__
             << "-->ERR: " << curl_easy_strerror(curl_msg->data.result) << endl;
        _error = true;
      }  // if(curl_msg->data.result==CURLE_OK)
    }    // if(curl_msg->msg==CURLMSG_DONE)
  }      // while(.....
}

void CurlStreamFile::init(const string &url, const string &cachefile) {
  _customHeaders = 0;

  _url = url;
  _running = 1;
  _error = false;

  _cached = 0;
  _size = 0;
  _curlHandle = curl_easy_init();
  _mCurlHandle = curl_multi_init();

  // const RcInitFile& rcfile = RcInitFile::getDefaultInstance();
  if (!cachefile.empty()) {
    _cache = std::fopen(cachefile.c_str(), "w+b");
    if (!_cache) {
      _cache = std::tmpfile();
    }
  } else {
    _cache = std::tmpfile();
  }
  if (!_cache) {
    throw base::SnailException("Could not create temporary cache file");
  }
  _cachefd = fileno(_cache);

  CURLcode ccode;
// Overrid cURL's default verification of ssl certificates
// this is insecure. so log security warning.
// equivalent to curl -k or curl --insecure
#if 0
	if(rcfile.insecureSSL()){
		cout<<"Allowing connections to ssl sites with invalid certification"<<endl;
		ccode = curl_easy_setopt(_curlHandle, CURLOPT_SSL_VERIFYPEER,0);
		if(ccode != CURLE_OK){
			throw SnailException(curl_easy_strerror(ccode));
		}
		ccode = curl_easy_setopt(_curlHandle, CURLOPT_SSL_VERIFYHOST, 0);
		if(ccode != CRULE_OK){
			throw SnailException(curl_easy_strerror(ccode));
		}
	}//if(rcfile.insecureSSL())
#endif
// Get shared data
#ifdef CURLSESSION
  ccode = curl_easy_setopt(_curlHandle, CURLOPT_SHARE,
                           CurlSession::get().getSharedHandle());
  if (ccode != CURLE_OK) {
    throw base::SnailException(curl_easy_strerror(ccode));
  }
#endif
  // Set expiration time for dns cache entries, in seconds
  // 0 disables caching
  //-1 make cache entries never expire
  // default is 60
  /// NOTE: this snippet is here just as a placeholder for whoever
  // will feel a need to make this parametrizable
  ccode = curl_easy_setopt(_curlHandle, CURLOPT_DNS_CACHE_TIMEOUT, 60);
  if (ccode != CURLE_OK) {
    throw base::SnailException(curl_easy_strerror(ccode));
  }
  ccode = curl_easy_setopt(_curlHandle, CURLOPT_USERAGENT, "Snail-0.10");
  if (ccode != CURLE_OK) {
    throw base::SnailException(curl_easy_strerror(ccode));
  }
  // set url
  ccode = curl_easy_setopt(_curlHandle, CURLOPT_URL, _url.c_str());
  if (ccode != CURLE_OK) {
    throw base::SnailException(curl_easy_strerror(ccode));
  }

  // set write data and function
  ccode = curl_easy_setopt(_curlHandle, CURLOPT_WRITEDATA, this);
  if (ccode != CURLE_OK) {
    throw base::SnailException(curl_easy_strerror(ccode));
  }
  ccode = curl_easy_setopt(_curlHandle, CURLOPT_WRITEFUNCTION,
                           CurlStreamFile::recv);
  if (ccode != CURLE_OK) {
    throw base::SnailException(curl_easy_strerror(ccode));
  }

  ccode = curl_easy_setopt(_curlHandle, CURLOPT_FOLLOWLOCATION, 1);
  if (ccode != CURLE_OK) {
    throw base::SnailException(curl_easy_strerror(ccode));
  }
}

CurlStreamFile::CurlStreamFile(const string &url, const string &cachefile) {
  init(url, cachefile);

  CURLMcode mcode = curl_multi_add_handle(_mCurlHandle, _curlHandle);
  if (mcode != CURLM_OK) {
    throw base::SnailException(curl_multi_strerror(mcode));
  }
}

CurlStreamFile::~CurlStreamFile() {
  curl_multi_remove_handle(_mCurlHandle, _curlHandle);
  curl_easy_cleanup(_curlHandle);
  curl_multi_cleanup(_mCurlHandle);
  std::fclose(_cache);
  if (_customHeaders) {
    curl_slist_free_all(_customHeaders);
  }
}

std::streamsize CurlStreamFile::read(void *dst, std::streamsize bytes) {
  if (eof() || _error) return 0;
  fillCache(bytes + tell());
  if (_error) return 0;
  return std::fread(dst, 1, bytes, _cache);
}
std::streamsize CurlStreamFile::readNonBlocking(void *dst,
                                                std::streamsize bytes) {
  if (eof() || _error) return 0;
  fillCacheNonBlocking();
  if (_error) {
    std::cout << "curl adaptor's fill cache noblocking set_error rather then "
                 "throw an exception" << std::endl;
    return 0;
  }
  std::streamsize actuallyRead = std::fread(dst, 1, bytes, _cache);
  if (_running) {
    // if we're still running drop any eof flag
    // on the cache
    clearerr(_cache);
  }
  return actuallyRead;
}
bool CurlStreamFile::eof() const {
  bool ret = (!_running && feof(_cache));
  return ret;
}
bool CurlStreamFile::bad() const { return _error; }
std::streampos CurlStreamFile::tell() const {
  std::streampos ret = std::ftell(_cache);
  return ret;
}

bool CurlStreamFile::seek(std::streampos pos) {
  if (pos < 0) {
    std::ostringstream os;
    os << "CurlStreamFile: can't seek to negative absolute position" << pos;
    throw base::SnailException(os.str());
  }
  fillCache(pos);
  if (_error) return false;
  if (_cached < pos) {
    return false;
  }
  if (std::fseek(_cache, pos, SEEK_SET) == -1) {
    std::cout << "warning : fseek failed" << std::endl;
    return false;
  }
  return true;
}
void CurlStreamFile::go_to_end() {
  CURLMcode mcode;
  while (_running > 0) {
    do {
      mcode = curl_multi_perform(_mCurlHandle, &_running);
    } while (mcode == CURLM_CALL_MULTI_PERFORM);

    if (mcode != CURLM_OK) {
      throw base::SnailException(curl_multi_strerror(mcode));
    }
    long code;
    curl_easy_getinfo(_curlHandle, CURLINFO_RESPONSE_CODE, &code);
    if (code == 404) {
      throw base::SnailException("404(error):File not fond");
    }
  }  // while(_running>0)
  if (std::fseek(_cache, 0, SEEK_END) == -1) {
    throw base::SnailException("go_to_end: fseek to end failed");
  }
}

size_t CurlStreamFile::size() const {
  if (!_size) {
    double size;
    CURLcode ret =
        curl_easy_getinfo(_curlHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
    if (ret == CURLE_OK) {
      assert(size <= std::numeric_limits<size_t>::max());
      _size = static_cast<size_t>(size);
    }
  }
  return _size;
}

#if 0
/************************************************************************
 * CurlSession definition and implementation
 *
 * ******************************************************************/
//a libcurl session consists in a shared handle sharing DNS cache and cookies
class CurlSession {
public:
	///get CurlSession singleton
	static CurSession& get();
	///get the shared handle;
	CURLSH* getSharedHandle() {return _shandle;}
private:
	//initialize a libcurl session
	//a libcurl session consist in a shared handle sharing DNS cache and COOKIS.
	//Also global libcurl initialization funtion will be invoked , so MAKE SURE not to
	//construct multiple CurlSession instances in a multithreaded environment
	//Pay attention :this is not thread-safe
	CurlSession();
	~CurlSession();

	//the libcurl share handle, for sharing cookies
	CURLSH *_shandle;
	//mutex protecting share state
	boost::mutex _shareMutex;
	//mutex protecting share cookies
	boost::mutex _cookieMutex;
	//mutex protecting dns cache
	boost::mutex _dnscacheMutex;

	///Import cookies if request
	//this method will lookup SNAIL_COOKIES_IN  in your os environment, and
	//if existing ,will parse the file sending each line to a fake easy handle
	//created ad-hoc
	void importCookies();

	//import cookies if request
	///this method will lookup Snail_COOKIES_OUT in you os environment, and if
	//existing, will create the file writting any cookie currently in the jar
	void exportCookies();

	void lockSharedHandle(CURL *handle, curl_lock_data data,
			curl_lock_access access);
	void unlockSharedHandle(CURL *handle, curl_lock_data data);

};
#endif

}  // namespace
