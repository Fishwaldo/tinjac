/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdlib.h>

#include "Wt/WApplication"
#include "Wt/WTime"
#include "Wt/WLogger"

#include "WtException.h"
#include "Utils.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>

namespace Wt {

  namespace {
    const int MSECS_PER_DAY = 24 * 60 * 60 * 1000;
  }

InvalidTimeException::InvalidTimeException()
{ }

InvalidTimeException::~InvalidTimeException() throw()
{ }

const char *InvalidTimeException::what() const throw()
{ 
  return "Error: Attempted operation on an invalid WTime";
}

WTime::WTime()
  : valid_(false),
    time_(-1)
{ }

WTime::WTime(int h, int m, int s, int ms)
{
  setHMS(h, m, s, ms);
}

bool WTime::setHMS(int h, int m, int s, int ms)
{
  if (   h >= 0 && h <= 23
      && m >= 0 && m <= 59
      && s >= 0 && s <= 59
      && ms >= 0 && ms <= 999) {
    valid_ = true;
    time_ = ((h * 60 + m) * 60 + s) * 1000 + ms;
  } else {
    WApplication *app = wApp;
    if (app)
      app->log("warn") << "Invalid time: "
		       << h << ":" << m << ":" << s << "." << ms;
    valid_ = false;
    time_ = 0; // !null
  }

  return valid_;
}

WTime WTime::addMSecs(int ms) const
{
  if (valid_) {
    long t = ((time_ + ms) % MSECS_PER_DAY + MSECS_PER_DAY) % MSECS_PER_DAY;
    return WTime(t);
  } else
    return *this;
}

WTime WTime::addSecs(int s) const
{
  return addMSecs(1000 * s);
}

bool WTime::isNull() const
{
  return time_ == -1;
}

int WTime::hour() const
{
  return time_ / (1000 * 60 * 60);
}

int WTime::minute() const
{
  return (time_ / (1000 * 60)) % 60;
}

int WTime::second() const
{
  return (time_ / 1000) % 60;
}

int WTime::msec() const
{
  return time_ % 1000;
}

int WTime::secsTo(const WTime& t) const
{
  return msecsTo(t) / 1000;
}

int WTime::msecsTo(const WTime& t) const
{
  if (!isValid() || !t.isValid())
    throw InvalidTimeException();

  return t.time_ - time_;
}

bool WTime::operator> (const WTime& other) const
{
  return other < *this;
}

bool WTime::operator< (const WTime& other) const
{
  if (!isValid() && !other.isValid())
    return false;

  if (!isValid() || !other.isValid())
    throw InvalidTimeException();

  return time_ < other.time_;
}

bool WTime::operator!= (const WTime& other) const
{
  return !(*this == other);
}

bool WTime::operator== (const WTime& other) const
{
  if (!isValid() && !other.isValid())
    return true;

  if (!isValid() || !other.isValid())
    throw InvalidTimeException();

  return time_ == other.time_;
}

bool WTime::operator<= (const WTime& other) const
{
  if (!isValid() && !other.isValid())
    return true;

  if (!isValid() || !other.isValid())
    throw InvalidTimeException();

  return time_ <= other.time_;
}

bool WTime::operator>= (const WTime& other) const
{
  return other <= *this;
}

WTime WTime::currentServerTime()
{
  return WTime((long)boost::posix_time::microsec_clock::local_time()
	       .time_of_day().total_milliseconds());
}

WString WTime::defaultFormat()
{
  return WString::fromUTF8("HH:mm:ss"); 
}

WTime WTime::fromString(const WString& s)
{
  return fromString(s, defaultFormat());
}

WTime::ParseState::ParseState()
{
  h = m = s = z = a = 0;
  hour = minute = sec = msec = 0;
  pm = parseAMPM = haveAMPM = false;
}

WTime WTime::fromString(const WString& s, const WString& format)
{
  WTime result;

  WDateTime::fromString(0, &result, s, format);

  return result;
}

WDateTime::CharState WTime::handleSpecial(char c, const std::string& v,
					  unsigned& vi, ParseState& parse,
					  const WString& format)
{
  switch (c) {
  case 'H':
  case 'h':
    parse.parseAMPM = c == 'h';

    if (parse.h == 0)
      if (!parseLast(v, vi, parse, format))
	return WDateTime::CharInvalid;

    ++parse.h;

    return WDateTime::CharHandled;

  case 'm':
    if (parse.m == 0)
      if (!parseLast(v, vi, parse, format))
	return WDateTime::CharInvalid;

    ++parse.m;

    return WDateTime::CharHandled;

  case 's':
    if (parse.s == 0)
      if (!parseLast(v, vi, parse, format))
	return WDateTime::CharInvalid;

    ++parse.s;

    return WDateTime::CharHandled;

  case 'z':
    if (parse.z == 0)
      if (!parseLast(v, vi, parse, format))
	return WDateTime::CharInvalid;

    ++parse.z;

    return WDateTime::CharHandled;

  case 'A':
  case 'a':
    if (!parseLast(v, vi, parse, format))
      return WDateTime::CharInvalid;

    parse.a = 1;

    return WDateTime::CharHandled;

  case 'P':
  case 'p':
    if (parse.a == 1) {
      if (!parseLast(v, vi, parse, format))
	return WDateTime::CharInvalid;

      return WDateTime::CharHandled;
    }

    /* fall through */

  default:
    if (!parseLast(v, vi, parse, format))
      return WDateTime::CharInvalid;

    return WDateTime::CharUnhandled;
  }
}

static void fatalFormatError(const WString& format, int c, const char* cs)
{
  std::stringstream s;
  s << "WTime format syntax error (for \"" << format.toUTF8()
    << "\"): Cannot handle " << c << " consecutive " << cs;

  throw WtException(s.str());
}

bool WTime::parseLast(const std::string& v, unsigned& vi,
		      ParseState& parse,
		      const WString& format)
{
  const char *letter[] = { "h's", "m's", "s'es", "z's" };

  for (int i = 0; i < 4; ++i) {
    int *count;
    int *value;
    int maxCount = 2;

    switch (i) {
    case 0:
      count = &parse.h;
      value = &parse.hour;
      break;
    case 1:
      count = &parse.m;
      value = &parse.minute;
      break;
    case 2:
      count = &parse.s;
      value = &parse.sec;
      break;
    case 3:
      count = &parse.z;
      value = &parse.msec;
      maxCount = 3;
    }

    if (*count != 0) {
      if (*count == 1) {
	std::string str;

	if (vi >= v.length())
	  return false;
	str += v[vi++];

	for (int j = 0; j < maxCount - 1; ++j)
	  if (vi < v.length())
	    if ('0' <= v[vi] && v[vi] <= '9')
	      str += v[vi++];
	try {
	  *value = boost::lexical_cast<int>(str);
	} catch (boost::bad_lexical_cast&) {
	  return false;
	}

      } else if (*count == maxCount) {
	if (vi + (maxCount - 1) >= v.length())
	  return false;

	std::string str = v.substr(vi, maxCount);
	vi += maxCount;

	try {
	  *value = boost::lexical_cast<int>(str);
	} catch (boost::bad_lexical_cast&) {
	  return false;
	}
      } else 
	fatalFormatError(format, *count, letter[i]);
    }

    *count = 0;
  }

  if (parse.a) {
    if (vi + 1 >= v.length())
      return false;

    std::string str = v.substr(vi, 2);
    vi += 2;

    parse.haveAMPM = true;

    if (str == "am" || str == "AM") {
      parse.pm = false;
    } else if (str == "pm" || str == "PM") {
      parse.pm = true;
    } else
      return false;

    parse.a = 0;
  }

  return true;
}

WString WTime::toString() const
{
  return WTime::toString(defaultFormat());
}

WString WTime::toString(const WString& format) const
{
  return WDateTime::toString(0, this, format);
}

int WTime::pmhour() const
{
  int result = hour() % 12;
  return result != 0 ? result : 12;
}

bool WTime::writeSpecial(const std::string& f, unsigned& i,
			 std::stringstream& result) const
{
  char buf[30];

  bool useAMPM = false; // FIXME

  switch (f[i]) {
  case 'h':
    if (f[i + 1] == 'h') {
      ++i;
      result << Utils::pad_itoa(useAMPM ? pmhour() : hour(), 2, buf);
    } else
      result << Utils::itoa(useAMPM ? pmhour() : hour(), buf);

    return true;
  case 'H':
    if (f[i + 1] == 'H') {
      ++i;
      result << Utils::pad_itoa(hour(), 2, buf);
    } else
      result << Utils::itoa(hour(), buf);

    return true;
  case 'm':
    if (f[i + 1] == 'm') {
      ++i;
      result << Utils::pad_itoa(minute(), 2, buf);
    } else
      result << Utils::itoa(minute(), buf);

    return true;
  case 's':
    if (f[i + 1] == 's') {
      ++i;
      result << Utils::pad_itoa(second(), 2, buf);
    } else
      result << Utils::itoa(second(), buf);

    return true;
  case 'z':
    if (f.substr(i + 1, 2) == "zz") {
      i += 2;
      result << Utils::pad_itoa(msec(), 3, buf);
    } else
      result << Utils::itoa(msec(), buf);

    return true;
  case 'a':
  case 'A':
    if (f[i + 1] == 'p' || f[i + 1] == 'P')
      ++i;

    if (hour() < 12)
      result << ((f[i] == 'a') ? "am" : "AM");
    else
      result << ((f[i] == 'p') ? "pm" : "PM");
 
    return true;
  default:
    return false;
  }
}

WTime::WTime(long t)
  : valid_(true),
    time_(t)
{ }

}
