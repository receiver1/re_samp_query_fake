#pragma once

#include <sstream>

class c_bytestream
{
    std::stringstream *_ss;
public:
    void set_stream(std::stringstream &ss)
    {
        _ss = &ss;
    }

    template<typename T>
    T read_num(std::streampos pos = -1)
    {
        T result;
        if (pos != -1) {
            _ss->seekg(pos);
        }
        _ss->read(reinterpret_cast<char *>(&result), sizeof(T));
        return result;
    }

    template<typename T = uint32_t>
    std::string read_str(std::streampos pos = -1)
    {
        std::string result;
        T result_length = read_num<T>();
        result.resize(result_length);
        _ss->read(&result[0], result_length);
        return result;
    }

    template<typename T>
    void write_num(T num, std::streampos pos = -1)
    {
        if (pos != -1) {
            _ss->seekp(pos);
        }
        _ss->write(reinterpret_cast<char *>(&num), sizeof(T));
    }

    template<typename T = uint32_t>
    void write_str(std::string str, std::streampos pos = -1)
    {
        T str_length = str.length();
        write_num<T>(str_length);
        _ss->write(str.c_str(), str_length);
    }

    std::string get() { return _ss->str(); }
};