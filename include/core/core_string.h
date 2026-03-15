#ifndef __STRING_H__
#define __STRING_H__

#include <core_types.h>
#include <core_allocator.h>

typedef struct {
    u8  *str;
    u64 size;
} string;

enum StringIteratorState {
    STRITER_FOUND,          // New string has been found and is ready to be returned via striter_next() call
    STRITER_SEARCH,         // New string is yet to be found
    STRITER_EMPTY           // The iterator is empty and strings are no more
};

struct StringIterator {
    enum StringIteratorState    state;
    string                      str;
    string                      sep;
    u64                         start;
    u64                         end;
};

#define str_fromlit(s)  (string){ (u8*)(s), sizeof((s)) - 1 }
#define str_fmt(s)      (int)(s).size, (s).str

string str_substr(string str, u32 start, u32 end);
boolean str_equals(string str, string other);
boolean str_contains(string str, u8 target);
i32 str_findchar(string str, u8 target);
string str_fromfile(const char *filename, struct MemoryBlock *block);

struct StringIterator striter_new(string str, string sep);
boolean striter_hasnext(struct StringIterator *iterator);
string striter_next(struct StringIterator *iterator);

#endif /* __STRING_H__ */
