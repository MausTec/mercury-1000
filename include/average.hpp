#ifndef __average_hpp
#define __average_hpp

#include <stddef.h>

template<class T, size_t len>
class MovingAverage {
public:
    T avg(void);
    void insert(T value, long tag);
    void clear(T value = 0);
    T max(void);
    T min(void);
    long max_tag(void);
    long min_tag(void);
    
private:
    T values[len] = {0};
    long tags[len] = {0};
    size_t index = 0;
    size_t valid_len = 0;
};

template <class T, size_t len>
T MovingAverage<T, len>::avg(void) {
    T sum = 0;

    for (size_t i = 0; i < len; i++) {
        sum += values[(index + i) % len];
    }

    return sum / len;
}

template<class T, size_t len>
void MovingAverage<T, len>::insert(T value, long tag) 
{
    index = (index + 1) % len;
    values[index] = value;
    tags[index] = tag;
}

template<class T, size_t len>
void MovingAverage<T, len>::clear(T value) 
{
    for (size_t i = 0; i < len; i++) {
        values[i] = value;
    }
}

template<class T, size_t len>
T MovingAverage<T, len>::max(void) 
{
    T val = values[0];
    long tag = tags[0];
    for (size_t i = 1; i < len; i++) {
        if (values[i] > val) {
            val = values[i];
            tag = tags[i];
        }
    }
    return val;
}

template<class T, size_t len>
T MovingAverage<T, len>::min(void) 
{
    T val = values[0];
    long tag = tags[0];
    for (size_t i = 1; i < len; i++) {
        if (values[i] < val) {
            val = values[i];
            tag = tags[i];
        }
    }
    return val;
}

template<class T, size_t len>
long MovingAverage<T, len>::max_tag(void) 
{
    T val = values[0];
    long tag = tags[0];
    for (size_t i = 1; i < len; i++) {
        if (values[i] > val) {
            val = values[i];
            tag = tags[i];
        }
    }
    return tag;
}

template<class T, size_t len>
long MovingAverage<T, len>::min_tag(void) 
{
    T val = values[0];
    long tag = tags[0];
    for (size_t i = 1; i < len; i++) {
        if (values[i] < val) {
            val = values[i];
            tag = tags[i];
        }
    }
    return tag;
}

#endif