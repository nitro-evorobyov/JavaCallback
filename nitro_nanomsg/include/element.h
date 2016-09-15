

#pragma once

#include <iostream>
#include <string>
#include <memory>

#if 0

namespace nitro
{
namespace element
{
enum Type
{
    None,
    String,
    Int,
    Float,
};

class BaseRef
{
public:

    template<class TBase>
    static std::shared_ptr<TBase>  Deserialize(std::istream& readStream)
    {
        int type = 0;

        auto saveSeek = readStream.cur;

        readStream >> type;

        char rlf;

        std::shared_ptr<BaseRef> readValue(new TBase());

        if (readValue->m_type != type)
        {
            readStream.seekg(saveSeek, std::ios_base::beg);
            return nullptr;
        }

        readValue->Read(readStream);

        return std::dynamic_pointer_cast<TBase>(readValue);
    }

    static bool Serialize(std::ostream& writeStream, const BaseRef& base)
    {
        writeStream << base.m_type << '\r';

        base.Write(writeStream);

        writeStream << "\r";

        return true;
    }

protected:
    BaseRef(Type type) : m_type(type){}

    virtual void Read(std::istream& readStream) = 0;
    virtual void Write(std::ostream& writeStream) const = 0;

private:
    int  m_type;
};


template<class T>
class TempRef
    : public BaseRef
{
public:


    const T&  Get() const{ return m_value; }
    void  Set(const T& value) const{ m_value = value; }

protected:
    TempRef(Type type, const T& defaultValue)
        : BaseRef(type)
        , m_value(defaultValue)
    {
    }

    TempRef(Type type)
        : BaseRef(type)
    {
    }

    void Read(std::istream& readStream) override
    {
        readStream >> m_value;
    }

    void Write(std::ostream& writeStream)  const override
    {
        writeStream << m_value;
    }

private:
    T   m_value;
};


class IntRef
    : public TempRef<int>
{
public:
    IntRef()
        : TempRef<int>(Int, 0)
    {}

    IntRef(int value)
        : TempRef<int>(Int, value)
    {}
};


class FloatRef
    : public TempRef<float>
{
public:
    FloatRef()
        : TempRef<float>(Float, 0)
    {}

    FloatRef(float value)
        : TempRef<float>(Float, value)
    {}
};

class StringRef
    : public TempRef<std::string>
{
public:
    StringRef()
        : TempRef<std::string>(String, "")
    {}

    StringRef(const std::string& value)
        : TempRef<std::string>(String, value)
    {}
};

}
}

#endif