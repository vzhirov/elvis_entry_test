#include "parser.h"

#include <iomanip>
#include <sstream>
#include <string.h>

Parser::Parser(Byte* bytes, int length)
    : mCursor    (bytes)
    , mBegin     (bytes)
    , mEnd       (bytes+length)
    , mIsError   (false)
    , mErrorMsg  ("")
{
}

int Parser::leftBytes() const
{
    return mEnd - mCursor;
}

bool Parser::parseSignature(Byte* data, int length)
{
    if(leftBytes()<length)
        setError("enought bytes for signature");
    else if(memcmp(mCursor,data,length)!=0)
        setError("invalid signature");
    if(isError())
        return false;
    mCursor += length;
    return true;
}

bool Parser::parseI8(int* result)
{
    if(leftBytes()<1)
        setError("enought bytes for I8");
    if(isError())
        return false;
    *result = mCursor[0];
    mCursor++;
    return true;
}

bool Parser::parseI16LE(int* result)
{
    if(leftBytes()<2)
        setError("enought bytes for I16");
    if(isError())
        return false;
    *result = (((int)mCursor[0])<<8)+mCursor[1];
    mCursor += 2;
    return true;
}

bool Parser::parseI32LE(int* result)
{
    int v1, v2;
    parseI16LE(&v1);
    parseI16LE(&v2);
    if(isError())
        return false;
    *result = (v1<<16)+v2;
    return true;
}

void Parser::skip(int length)
{
    if(leftBytes()<length)
        setError("enought bytes for skip");
    if(isError())
        return;
    mCursor += length;
}

int  Parser::byteSum(int size)
{
    if(leftBytes()<size)
        setError("enought bytes for byteSum");
    unsigned char result = 0;
    for(int i=0;i<size;i++)
        result += mCursor[i];
    return (int)result;
}

Parser::Byte Parser::operator[] (int index)
{
    if(leftBytes()<index)
        setError("enought bytes for subsript");
    return isError() ? 0 : mCursor[index];
}

bool Parser::isError() const
{
    return mIsError;
}

std::string Parser::errorMessage() const
{
    return mErrorMsg;
}

void Parser::setError(const std::string& msg)
{
    if(isError())
        return;
    mIsError = true;
    std::ostringstream oss;
    oss << msg << " at " << (mCursor-mBegin)
        << " :";
    for(int i=0;i<mEnd-mCursor && i<5 ;i++)
        oss << " " << std::hex << std::setw(2) << std::setfill('0')
            << (int)mCursor[i];
    oss << std::endl;
    mErrorMsg = oss.str();
}

// vim: ts=4 et

