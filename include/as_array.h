#include "as_types.h"

#define AS_DECLARE_ARRAY(_name, _capacity, _type)           \
    typedef struct {                                        \
        _type data[_capacity];                              \
        sz size;                                            \
    } _name

#define AS_PUSH_BACK_ARRAY(_array, _element)                                    \
    if ((_array).size < AS_ARRAY_SIZE(_array.data)){                            \
        (_array).data[(_array).size++] = _element; }                            \
    else { AS_LOG(LV_ERROR,"Array overflow"); }

#define AS_INSERT_AT_ARRAY(_array, _index, _element)                                                    \
    if ((_index) >= 0 && (_index) <= (_array).size) {                                                   \
        if ((_array).size < AS_ARRAY_SIZE(_array.data)) {                                               \
            for (sz i = (_array).size; i > (_index); --i) { (_array).data[i] = (_array).data[i - 1];}   \
            (_array).data[(_index)] = _element;                                                         \
            ++(_array).size; }                                                                          \
        else { AS_LOG(LV_ERROR,"Array overflow"); }                                                     \
    } else { AS_LOG(LV_ERROR,"Array index out of bounds"); }

#define AS_REMOVE_AT_ARRAY(_array, _index)                                                              \
    if ((_index) >= 0 && (_index) < (_array).size) {                                                    \
         for (sz i = (_index); i < (_array).size - 1; ++i) { (_array).data[i] = (_array).data[i + 1]; } \
        --(_array).size; }                                                                              \
    else { AS_LOG(LV_ERROR,"Array index out of bounds"); }

#define AS_GET_ARRAY_ELEM(_array, _index)                                                               \
    (((_index) >= 0 && (_index) < (_array).size) ? &((_array).data[_index]) : NULL)
