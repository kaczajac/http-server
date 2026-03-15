#include <core_string.h>

#include <stdio.h>

string str_substr(string str, u32 start, u32 end) {

    end = end < str.size ? end : str.size;
    start = start < end ? start : end;

    return (string) { .str = str.str + start, .size = end - start };

}

boolean str_equals(string str, string other) {

    u8 str_p = 0; 
    u8 other_p = 0;

    if (str.size != other.size) return FALSE;
    if (str.str == other.str) return TRUE;

    for (; str_p < str.size; str_p++, other_p++) {
        if (str.str[str_p] != other.str[other_p])
            return FALSE;
    }

    return TRUE;

}

boolean str_contains(string str, u8 target) {
    
    for (u8 position = 0; position < str.size; position++) {
        if (str.str[position] == target) {
            return TRUE;
        }
    }

    return FALSE;

}

i32 str_findchar(string str, u8 target) {
    
    for (u8 position = 0; position < str.size; position++) {
        if (str.str[position] == target) {
            return position;
        }
    }

    return -1;

}

string str_fromfile(const char *filename, struct MemoryBlock *block) {

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return (string) {};
    }

    u64 file_size = 524;
    u8 *file_content = memblock_alloc(block, file_size);
    if (file_content == NULL) {
        return (string) {};
    }

    i32 bytes_read = fread(file_content, 1, file_size - 1, file);

    fclose(file);
    return (string) {
        .str = file_content,
        .size = bytes_read
    };

}

struct StringIterator striter_new(string str, string sep) {

    return (struct StringIterator) {
        .state  = STRITER_SEARCH,
        .str    = str,
        .sep    = sep,
        .start  = 0,
        .end    = 0
    };

}

boolean striter_hasnext(struct StringIterator *iterator) {

    switch (iterator->state) {

        case STRITER_FOUND: {
            return TRUE;
        };

        case STRITER_EMPTY: {
            return FALSE;
        };

        case STRITER_SEARCH: {

            boolean new_start_found = FALSE;
            u64 position = iterator->start;
            while (position < iterator->str.size) {

                if (!str_contains(iterator->sep, iterator->str.str[position])) {
                    new_start_found = TRUE;
                    break;
                }
                position++;
                
            }

            if (new_start_found) {

                iterator->start = position;
                while (position < iterator->str.size) {

                    if (str_contains(iterator->sep, iterator->str.str[position])) {
                        break;
                    };
                    position++;

                }

                iterator->end = position;
                iterator->state = STRITER_FOUND;

            } else {

                iterator->state = STRITER_EMPTY;

            }

            return new_start_found;

        };

    }

    return FALSE;

}

string striter_next(struct StringIterator *iterator) {

    string result = (string) {
        .str = iterator->str.str + iterator->start,
        .size = iterator->end - iterator->start
    };

    iterator->start = iterator->end;
    iterator->state = STRITER_SEARCH;
    return result;

}
