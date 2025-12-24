#pragma once

#include <cstddef>
#include <cstring>


/**
 * @brief Fixed-capacity, stack/RTOS-safe string.
 *
 * StaticString stores text in an internal fixed-size buffer.
 * No dynamic allocation is ever performed.
 *
 * @tparam N Maximum string length (excluding null terminator)
 */
template<size_t N>
class StaticString {
public:
    StaticString() {
        clear();
    }

    StaticString(const char* str) {
        assign(str);
    }

    StaticString(const StaticString& other) {
        assign(other.c_str());
    }

    StaticString& operator=(const StaticString& other) {
        if (this != &other) {
            assign(other.c_str());
        }
        return *this;
    }

    // -------- Core API --------

    /**
     * @brief Assign C-string (truncated if too long)
     */
    void assign(const char* str) {
        if (!str) {
            clear();
            return;
        }

        size_t len = strnlen(str, N);
        memcpy(buffer_, str, len);
        buffer_[len] = '\0';
        length_ = len;
    }

    /**
     * @brief Clear string
     */
    void clear() {
        buffer_[0] = '\0';
        length_ = 0;
    }

    // -------- Access --------

    const char* c_str() const {
        return buffer_;
    }

    size_t length() const {
        return length_;
    }

    constexpr size_t capacity() const {
        return N;
    }

    bool empty() const {
        return length_ == 0;
    }

    bool full() const {
        return length_ == N;
    }

    // -------- Modification --------

    /**
     * @brief Append C-string (truncated if needed)
     */
    bool append(const char* str) {
        if (!str || full()) {
            return false;
        }

        size_t avail = N - length_;
        size_t add = strnlen(str, avail);

        memcpy(buffer_ + length_, str, add);
        length_ += add;
        buffer_[length_] = '\0';

        return add > 0;
    }

    /**
     * @brief Append single character
     */
    bool push_back(char c) {
        if (full()) {
            return false;
        }

        buffer_[length_++] = c;
        buffer_[length_] = '\0';
        return true;
    }

    // -------- Comparison --------

    bool operator==(const StaticString& other) const {
        return strcmp(buffer_, other.buffer_) == 0;
    }

    bool operator!=(const StaticString& other) const {
        return !(*this == other);
    }

private:
    char buffer_[N + 1];   // +1 for null terminator
    size_t length_;
};

