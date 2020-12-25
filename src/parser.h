#ifndef PARSER_H
#define PARSER_H

#include <string>

class ValueInfo
{
};

class EnumInfo
{
};

/// General purpose binary data parser
/// Implements data cursor and conversion routines

class Parser
{
    typedef const unsigned char Byte;

    Byte*  mCursor;
    Byte*  mBegin;
    Byte*  mEnd;
    bool   mIsError;
    std::string mErrorMsg;

public:
    Parser(Byte* bytes, int length);

    // read stream
    int  leftBytes() const;
    bool parseSignature(Byte* data, int length);
    bool parseI16LE(int* result);
    bool parseI32LE(int* result);
    bool parseI8(int* result);
    void skip(int length);
    int  byteSum(int size);

    Byte operator[] (int index);

    // error
    bool        isError      () const;
    std::string errorMessage () const;
    void        setError     (const std::string& msg);


};


#endif // PARSER_H

// vim: ts=4 et

