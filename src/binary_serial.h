
#include <iostream>
#include <boost/endian/arithmetic.hpp>

namespace Binary {
    class NotEnoughData: public std::runtime_error {
        private:
            std::string typ;
        public:

        NotEnoughData(std::string x)
            : std::runtime_error("Not enough data (" + x + ")"  )
            , typ(x) {};

        virtual const char* what() const throw() {
            return typ.c_str();
        }
    };

    struct Data {
        std::queue<boost::endian::little_uint8_t> q;
        Data() { };
        Data(std::initializer_list<boost::endian::little_uint8_t> x) : q(x) {};
        Data(size_t s, boost::endian::little_uint8_t* arr) {
            for (size_t i=0; i<s; i++) {
                push(arr[i]);
            };
        };
        boost::endian::little_uint8_t getByte() {
            boost::endian::little_uint8_t x = q.front();
            q.pop();
            return x;
        };
        void push(boost::endian::little_uint8_t x) {
            q.push(x);
        };
        template <typename FILE>
        void writeTo(FILE& f) {
            while (!q.empty()) {
                auto x = getByte();
                f.write((char*)&x, sizeof(decltype(x)));
            };
        };
        template <typename FILE>
        void loadFrom(FILE& f) {
            while (!f.eof()) {
                boost::endian::little_uint8_t x;
                f.read((char*)&x, sizeof(decltype(x)));
                push(x);
            };
        };
    };

    Data operator+(Data x, Data y) {
        if (y.q.empty()) return x;
        Data w;
        w.q = x.q;
        for (auto e = y.q.front() ; !y.q.empty() ; y.q.pop()) {
            w.q.push(e);
        };
        return w;
    }
    Data& operator+=(Data& x, Data y) {
        while (!y.q.empty()) {
            x.q.push(y.getByte());
        };
        return x;
    };

    template <typename T = uint8_t>
    struct data_int {};
    template <> struct data_int <int8_t> {
        using endian = boost::endian::little_int8_t;
    };
    template <> struct data_int <uint8_t> {
        using endian = boost::endian::little_uint8_t;
    };
    template <> struct data_int <int16_t> {
        using endian = boost::endian::little_int16_t;
    };
    template <> struct data_int <uint16_t> {
        using endian = boost::endian::little_uint16_t;
    };
    template <> struct data_int <int32_t> {
        using endian = boost::endian::little_int32_t;
    };
    template <> struct data_int <uint32_t> {
        using endian = boost::endian::little_uint32_t;
    };
    template <> struct data_int <int64_t> {
        using endian = boost::endian::little_int64_t;
    };
    template <> struct data_int <uint64_t> {
        using endian = boost::endian::little_uint64_t;
    };

    template <typename T>
    Data serialize(T x) {
        typedef typename data_int<T>::endian ENDIAN_T;
        ENDIAN_T w = x;
        return Data(sizeof(ENDIAN_T)/sizeof(data_int<uint8_t>::endian) , (data_int<uint8_t>::endian *)&w);
    };
    template<> Data serialize(std::string str) {
        Data w = serialize<uint8_t>(str.size());
        for (auto& c : str) {
            w += serialize<uint8_t>(c);
        };
        return w;
    };

    template <typename T>
    T deserialize(Data& d) {
        typedef typename data_int<T>::endian ENDIAN_T;
        size_t s = sizeof(ENDIAN_T)/sizeof(data_int<>::endian);
        if (d.q.size() < s) throw NotEnoughData(typeid(T).name());
        data_int<>::endian arr[16];
        for (size_t i=0; i<s; i++) {
            arr[i] = d.getByte();
        };
        return (T)*(ENDIAN_T*)&arr;
    };
    template<> std::string deserialize(Data& d) {
        std::string str = "";
        {
            auto size = deserialize<uint8_t>(d);
            for (auto j=0; j<size; j++) {
                str.push_back(Binary::deserialize<uint8_t>(d));
            };
        };
        return str;
    };
};