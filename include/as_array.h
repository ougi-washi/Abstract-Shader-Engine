// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_types.h"

#define AS_ARRAY_DECLARE(_name, _capacity, _type)           \
    typedef struct {                                        \
        _type data[_capacity];                              \
        sz size;                                            \
    } _name;

#define AS_ARRAY_INCREMENT(_array) \
    ((_array).size < AS_ARRAY_SIZE((_array).data) ? &((_array).data[(_array).size++]) : NULL)
    
#define AS_ARRAY_PUSH_BACK(_array, _element)                                    \
    if ((_array).size < AS_ARRAY_SIZE((_array).data)){                            \
        (_array).data[(_array).size++] = _element; }                          \
    else { AS_LOG(LV_ERROR,"Array overflow"); }

#define AS_ARRAY_INSERT_AT(_array, _index, _element)                                                    \
    if ((_index) >= 0 && (_index) <= (_array).size) {                                                   \
        if ((_array).size < AS_ARRAY_SIZE(_array.data)) {                                               \
            for (sz i = (_array).size; i > (_index); --i) { (_array).data[i] = (_array).data[i - 1];}   \
            (_array).data[(_index)] = _element;                                                         \
            ++(_array).size; }                                                                          \
        else { AS_LOG(LV_ERROR,"Array overflow"); }                                                     \
    } else { AS_LOG(LV_ERROR,"Array index out of bounds"); }

#define AS_ARRAY_GET(_array, _index)                                                               \
    (((_index) >= 0 && (_index) < (_array).size) ? &((_array).data[_index]) : NULL)

#define AS_ARRAY_SWAP(_type, _array, _index1, _index2) \
    if ((_index1) >= 0 && (_index1) < (_array).size && \
        (_index2) >= 0 && (_index2) < (_array).size) { \
        _type temp = (_array).data[(_index1)]; \
        (_array).data[(_index1)] = (_array).data[(_index2)]; \
        (_array).data[(_index2)] = temp; \
    } else { AS_LOG(LV_ERROR, "Invalid indices for array swap"); }

#define AS_ARRAY_FIND(_array, _element, _found_index) \
for (sz _i = 0; _i < (_array).size; ++_i) { \
    if ((_array).data[_i] == (_element)) { \
        (_found_index) = _i; \
        break; \
    } \
}

#define AS_ARRAY_FIND_PTR(_array, _element_ptr, _found_index) \
for (sz _i = 0; _i < (_array).size; ++_i) { \
    if (&(_array).data[_i] == (_element_ptr)) { \
        (_found_index) = _i; \
        break; \
    } \
}

#define AS_ARRAY_REMOVE_AT(_array, _index)                                                              \
    if ((_index) >= 0 && (_index) < (_array).size) {                                                    \
         for (sz i = (_index); i < (_array).size - 1; ++i) { (_array).data[i] = (_array).data[i + 1]; } \
        --(_array).size; }                                                                              \
    else { AS_LOG(LV_ERROR,"Array index out of bounds"); }

#define AS_ARRAY_REMOVE_PTR(_array, _element_ptr)                                                               \
    do{sz _found_index = -1;                                                                                       \
    AS_ARRAY_FIND_PTR(_array, _element_ptr, _found_index);                                                      \
    if(_found_index > -1) { AS_ARRAY_REMOVE_AT(_array, _found_index); }} while(0)

#define AS_ARRAY_FOR_EACH(_array, _type, _it, _exec)    \
for (sz _i = 0; _i < (_array).size; ++_i) {             \
    _type* _it = &((_array).data[_i]);                  \
    if (_it){ ##_exec };                                \
}

#define AS_ARRAY_GET_SIZE(_array) (_array).size
#define AS_ARRAY_GET_LAST_INDEX(_array) _array.size - 1
#define AS_ARRAY_CLEAR(_array) { (_array).size = 0; }

AS_ARRAY_DECLARE(voids32, 32, void*);
AS_ARRAY_DECLARE(voids64, 64, void*);
AS_ARRAY_DECLARE(voids128, 128, void*);
AS_ARRAY_DECLARE(voids256, 256, void*);
AS_ARRAY_DECLARE(voids512, 512, void*);
AS_ARRAY_DECLARE(voids1024, 1024, void*);

AS_ARRAY_DECLARE(char512, 512, char);
AS_ARRAY_DECLARE(char1024, 1024, char);
