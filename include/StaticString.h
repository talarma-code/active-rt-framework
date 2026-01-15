#pragma once

#include <cstddef>
#include <cstring>
#include <cstdio>


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

    // -------- Formatting --------

    /**
     * @brief Format string into internal buffer using snprintf-style API.
     *
     * Writes at most N characters plus null terminator into the buffer.
     * On truncation, the buffer still contains a valid, null-terminated string.
     *
     * @param format printf-style format string
     * @param args   printf-style arguments
     * @return number of characters that would have been written (excluding null terminator),
     *         or negative value on error (same semantics as std::snprintf)
     */
    template<typename... Args>
    int snprintf(const char* format, Args... args) {
        if (!format) {
            clear();
            return -1;
        }

        int result = std::snprintf(buffer_, sizeof(buffer_), format, args...);

        if (result < 0) {
            clear();
            return result;
        }

        // std::snprintf returns the number of characters that would have been written
        // (excluding the null terminator). Clamp length_ to capacity N.
        if (static_cast<size_t>(result) > N) {
            length_ = N;
        } else {
            length_ = static_cast<size_t>(result);
        }

        // Ensure null-termination at the end of the used buffer.
        buffer_[length_] = '\0';

        return result;
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

