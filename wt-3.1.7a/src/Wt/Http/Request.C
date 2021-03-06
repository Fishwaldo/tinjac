/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdexcept>
#include <sstream>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "Wt/Http/Request"
#include "Utils.h"
#include "WebRequest.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace {
  std::stringstream emptyStream;
}

namespace Wt {
  namespace Http {

UploadedFile::UploadedFile(const std::string& spoolName,
			   const std::string& clientFileName,
			   const std::string& contentType)
{
  fileInfo_.reset(new Impl());

  fileInfo_->spoolFileName = spoolName;
  fileInfo_->clientFileName = clientFileName;
  fileInfo_->contentType = contentType;
  fileInfo_->isStolen = false;
}

void UploadedFile::Impl::cleanup()
{
  if (!isStolen)
    unlink(spoolFileName.c_str());
}

UploadedFile::Impl::~Impl()
{
  cleanup();
}

const std::string& UploadedFile::spoolFileName() const
{
  return fileInfo_->spoolFileName;
}

const std::string& UploadedFile::clientFileName() const
{
  return fileInfo_->clientFileName;
}

const std::string& UploadedFile::contentType() const
{
  return fileInfo_->contentType;
}

void UploadedFile::stealSpoolFile() const
{
  fileInfo_->isStolen = true;
}

Request::ByteRange::ByteRange()
  : firstByte_(0),
    lastByte_(0)
{
}

Request::ByteRange::ByteRange(::uint64_t first, ::uint64_t last)
  : firstByte_(first),
    lastByte_(last)
{ }

Request::ByteRangeSpecifier::ByteRangeSpecifier()
  : satisfiable_(true)
{ }

const ParameterValues& Request::getParameterValues(const std::string& name)
  const
{
  ParameterMap::const_iterator i = parameters_.find(name);
  if (i != parameters_.end())
    return i->second;
  else
    return WebRequest::emptyValues_;
}

const std::string *Request::getParameter(const std::string& name) const
{
  const ParameterValues& v = getParameterValues(name);
  if (!Utils::isEmpty(v))
    return &v[0];
  else
    return 0;
}

const UploadedFile *Request::getUploadedFile(const std::string& name) const
{
  UploadedFileMap::const_iterator i = files_.find(name);
  if (i != files_.end())
    return &i->second;
  else
    return 0;
}

std::string Request::method() const
{
  return request_ ? request_->requestMethod() : "GET";
}

std::string Request::serverName() const
{
  return request_ ? request_->serverName() : std::string();
}

std::string Request::serverPort() const
{
  return request_ ? request_->serverPort() : std::string();
}

std::string Request::path() const
{
  return request_ ? request_->scriptName() : std::string();
}

std::string Request::pathInfo() const
{
  return request_ ? request_->pathInfo() : std::string();
}

std::string Request::queryString() const
{
  return request_ ? request_->queryString() : std::string();
}

std::string Request::urlScheme() const
{
  return request_ ? request_->urlScheme() : std::string();
}

std::string Request::headerValue(const std::string& field) const
{
  return request_ ? request_->headerValue(field) : std::string();
}

int Request::tooLarge() const
{
  return request_ ? request_->postDataExceeded() : 0;
}

std::istream& Request::in() const
{
  if (request_) {
    WebRequest *web = const_cast<WebRequest *>(request_);
    return web->in();
  } else {
    return emptyStream;
  }
}

std::string Request::contentType() const
{
  return request_ ? request_->contentType() : std::string();
}

int Request::contentLength() const
{
  return request_ ? request_->contentLength() : 0;
}

std::string Request::userAgent() const
{
  return request_ ? request_->userAgent() : std::string();
}

std::string Request::clientAddress() const
{
  return request_ ? request_->remoteAddr() : std::string();
}

Request::ByteRangeSpecifier Request::getRanges(::int64_t filesize) const
{
  return getRanges(headerValue("Range"), filesize);
}

Request::ByteRangeSpecifier Request::getRanges(const std::string &rangeHdr,
                                               ::int64_t filesize)
{
  Request::ByteRangeSpecifier retval;

  if (filesize == 0) {
    // Don't waste CPU time and simplify code below.
    retval.setSatisfiable(false);
    return retval;
  }

  bool syntaxError = false;
  bool satisfiable = filesize == -1;
  std::vector<std::string> rangeSpecifier;
  boost::split(rangeSpecifier, rangeHdr, boost::is_any_of("="));

  if (rangeSpecifier.size() == 2) {
    boost::trim(rangeSpecifier[0]);
    if (boost::iequals(rangeSpecifier[0], "bytes")) {
      std::vector<std::string> ranges;
      boost::split(ranges, rangeSpecifier[1], boost::is_any_of(","));
      for (std::size_t i = 0; i < ranges.size(); ++i) {
        std::vector<std::string> range;
        boost::split(range, ranges[i], boost::is_any_of("-"));
        if (range.size() == 2) {
          std::string start = range[0];
          std::string end = range[1];

	  boost::trim(start);
	  boost::trim(end);

          uint64_t startInt=0, endInt=0;
          try {
            if (start != "")
              startInt = boost::lexical_cast<uint64_t>(start);
            if (end != "")
              endInt = boost::lexical_cast<uint64_t>(end);
          } catch (boost::bad_lexical_cast &) {
            // syntactically invalid
            syntaxError = true;
          }
          if (start == "") {
            // notation -599: return last 599 bytes
            if (filesize != -1 && end != "") {
              if (endInt >= (uint64_t)filesize) {
                endInt = (std::size_t)filesize;
              }
              if (endInt > 0) {
                satisfiable = true;
                retval.push_back
		  (ByteRange
		   (uint64_t(filesize - endInt), std::size_t(filesize - 1)));
              }
              else {
                // Not really specified as syntax error. The paragraph about
                // 'satisfiability' seems to imply that we should simply
                // ignore it.
              }
            } else {
              // syntactically invalid
              syntaxError = true;
            }
          } else {
            if (filesize == -1 || startInt < (uint64_t)filesize) {
              if (end == "") {
                satisfiable = true;
                // notation 599-: returns from byte 599 to eof
                if (filesize == -1)
                  retval.push_back
		    (ByteRange(startInt, std::numeric_limits<uint64_t>::max()));
                else
                  retval.push_back
		    (ByteRange
		     (startInt, uint64_t(filesize - 1)));
              } else {
                if (startInt <= endInt) {
                  satisfiable = true;
                  if (filesize >= 0 && endInt > (uint64_t)filesize)
                    endInt = uint64_t(filesize - 1);
                  retval.push_back(ByteRange(startInt, endInt));
                } else {
                  // syntactically invalid
                  syntaxError = true;
                }
              }
            } else {
              // Not-satisfiable: just skip this range
            }
          }
        } else {
          syntaxError = true;
        }
      }
    } else {
      // only understand 'bytes'
      syntaxError = true;
    }
  } else {
    // Too many equals
    syntaxError = true;
  }
  if (syntaxError) {
    return ByteRangeSpecifier();
  } else {
    retval.setSatisfiable(satisfiable);
    return retval;
  }
}

Request::Request(const WebRequest& request, ResponseContinuation *continuation)
  : request_(&request),
    parameters_(request.getParameterMap()),
    files_(request.uploadedFiles()),
    continuation_(continuation)
{ }

Request::Request(const ParameterMap& parameters, const UploadedFileMap& files)
  : request_(0),
    parameters_(parameters),
    files_(files),
    continuation_(0)
{ }

  }
}
