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

#include "url.h"

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cctype>
#include <sstream>
#include <algorithm>
#include<cerrno>

namespace net {

Url::Url(const string& absUrl){
	if( (absUrl.size() && absUrl[0]=='/')
		|| (absUrl.find("://") != string::npos)
		|| (absUrl.size() > 1 && absUrl[1] ==':')
        || (absUrl[0] == '.') //for win32 or win64
		){
		initUrl(absUrl);
	}else{
		cerr<<"the absUrl is wrong"<<endl;
	}
}

//public member function
const string& Url::protocol() const{ return _protocol;}
const string& Url::hostName() const {return _hostName;}
const string& Url::port() const {return _port;}
const string& Url::path() const {return _path;}
const string& Url::anchor() const {return _anchor;}
const string& Url::queryString() const {return _queryString;}
 void Url::setQueryString(const string& value){_queryString = value;}

string Url::str() const{
	string ret = _protocol + "://" + _hostName;
	if(!_port.empty()){
		ret += ":" + _port;
	}
	ret += _path;
	if(!_queryString.empty()){
		ret += "?" + _queryString;
	}

	if(!_anchor.empty()){
		ret += "#" + _anchor;
	}
	return ret;
}

//static function
void Url::parseQueryString(const string& str, map<string, string>& tMap){
	if( str.empty()) return;

	string qStr = str;
	if(qStr[0] == '?'){
		qStr = qStr.substr(1);
	}
	string::size_type cur, pre, subCur;
	string name,value;
	for(cur = 0; cur!=string::npos; cur=qStr.find('&',cur) ){
		if(!cur){
			cur = qStr.find('&');
			subCur = qStr.find('=');
			name = qStr.substr(0,subCur);
			value = qStr.substr(subCur+1,cur);
			pre = cur;
		}else{
			subCur = qStr.find('=',pre+1);
			name = qStr.substr(pre+1,subCur);
			value = qStr.substr(subCur+1,cur);
			pre = cur;
		}

		decode(name);
		decode(value);
		tMap[name] = value;
	}//for(cur =NULL; cur!=string::npos;)
	subCur = qStr.find("=",pre+1);
	name = qStr.substr(pre+1,subCur);
	value = qStr.substr(subCur+1);

	decode(name);
	decode(value);
	tMap[name] = value;
}
void Url::decode(string& input){
	int hexcode;

	for(unsigned int i=0; i<input.length(); i++){
		if(input[i]=='%' && (input.length()>i+2)
				&& isxdigit(input[i+1])
				&& isxdigit(input[i+2])){
			input[i+1] = toupper(input[i+1]);
			input[i+2] = toupper(input[i+2]);
			if(isdigit(input[i=1])){
				hexcode = (input[i+1] - '0') *16;
			}else{
				hexcode = (input[i+2] - 'A'+10) *16;
			}

			if(isdigit(input[i+2])){
				hexcode += (input[i+2] - '0');
			}else{
				hexcode += (input[i+2] - 'A'+10) ;
			}
			input[i] = hexcode;
			input.erase(i+1,2);
		}else if(input[i] == '+'){
			input[i] = ' ';
		}
	}//for
}

string Url::encode(const string& str){
	string tmpStr(str);
	encode(tmpStr);
	return tmpStr;
}
void Url::encode(string& input){
	const string symbol = " \"#$%&+,/:;<=>?@[\\]^`{|}~_.!-(')";
	const string hexdigits="0123456789ABCDEF";

	for(unsigned int i=0; i<input.length(); i++){
		unsigned c = input[i] &0xFF;

		if(c<32 || c>126 || symbol.find(char(c))!=string::npos){
			input[i] = '%';
			input.insert(++i,hexdigits.substr(c>>4,1));
			input.insert(++i,hexdigits.substr(c&0xF,1));
		}else if(c==' '){
			input[i] = '+';
		}
	}
}
//private  member functions
void Url::initUrl(const string& absUrl){
  original_url_ = absUrl;
	string::size_type pos = absUrl.find("://");
	if(pos != string::npos){
		_protocol = absUrl.substr(0,pos);
		//advance input pointer to past the :// part
		pos += 3;
		if( pos == absUrl.size()){
			cerr<<"protocol-only url"<<endl;
			return ;
		}
		//find host name
		string::size_type hostPos = absUrl.find('/',pos);
		if(hostPos == string::npos){
			_hostName = absUrl.substr(pos);
			_path = "/";

			splitPort();
			return ;
		}

		_hostName = absUrl.substr(pos, hostPos-pos);

		_path = absUrl.substr(hostPos);
	}else{
		_protocol = "file";
		_path = absUrl;
	}

	splitPort();
	splitQueryString();
}

void Url::splitAnchor(){
	assert(_anchor =="");
	string::size_type  pos = _path.find("#");
	if(pos != string::npos){
		_anchor = _path.substr(pos + 1);
		_path.erase(pos);
	}
}

void Url::splitPort(){
	assert(_port == "");
	string::size_type pos = _hostName.find(":");
	if(pos != string::npos){
		_port = _hostName.substr(pos+1);
		_hostName.erase(pos);
	}
}

void Url::splitQueryString(){
	assert(_queryString == "");
	string::size_type pos = _path.find("?");
	if(pos != string::npos){
		_queryString = _path.substr(pos+1);
		_path.erase(pos);
	}
}

string Url::OriginalUrl() const {
  return original_url_;
}

ostream& operator<< (ostream& o, const Url& u){
	return o<<u.str();
}


} // namespace
