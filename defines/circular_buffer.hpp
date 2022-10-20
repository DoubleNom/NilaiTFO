#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP

#include <cstdint>
#include <cstdlib>
#include <vector>

template<typename T>
class CircularBuffer {
  public:
    CircularBuffer(size_t size = 0) : m_external(false), m_capacity(size) { init(size); }

    CircularBuffer(T* c, size_t size) : m_external(true), m_capacity(size) { init(c, size); }

    ~CircularBuffer() {
        if (!m_external) delete[] m_c;
    }

    void init(size_t size) {
        m_capacity = size;
        m_write    = 0;
        m_read     = 0;
        m_size     = 0;
        if (!m_external) {
            delete[] m_c;
            if (size > 0) {
                m_c = new T[size];
            }
        }
    }

    void init(T* c, size_t size) {
        m_capacity = size;
        m_write    = 0;
        m_read     = 0;
        m_size     = 0;
        if (!m_external) {
            delete[] m_c;
        }
        m_c        = c;
        m_external = true;
    }

    /**
     * Return the first element of the buffer
     */
    T& front() { return m_c[m_read]; }

    /**
     * Increment the read pointer by one, effectively removing the first element
     * of the buffer If the buffer size is 0, does nothing
     */
    void pop() {
        if (size() >= 0) {
            m_read = next(m_read);
            --m_size;
        }
    }

    /**
     * Increment the read pointer by a number n
     * effectively removing the n first elements of the buffer
     */
    void pop(size_t n) {
        if (n >= m_capacity) n = m_capacity;
        m_read += n;
        m_size -= n;
        while (m_read >= m_capacity) m_read -= m_capacity;
    }

    /**
     * Insert an element at the end of the buffer
     * If the buffer is full, does nothing
     * @param t the element to insert
     */
    size_t push(const T t) {
        if (size() < m_capacity) {
            m_c[m_write] = t;
            m_write      = next(m_write);
            ++m_size;
            return 1;
        }
        return 0;
    }

    size_t push(const T* t, size_t len) {
        size_t pushed = 0;
        for (size_t i = 0; i < len; ++i) {
            pushed += push(t[i]);
        }
        return pushed;
    }

    size_t push(std::vector<T> buff) {
        size_t pushed = 0;
        for (const auto& b : buff) {
            pushed += push(b);
        }
        return pushed;
    }

    T& read() {
        T& obj = m_c[m_read];
        m_read = next(m_read);
        --m_size;
        return obj;
    }

    size_t read(T* buff, size_t len = 0) {
        if (len <= 0 || m_size < len) len = m_size;
        for (size_t i = 0; i < len; ++i) {
            buff[i] = (*this)[i];
        }
        m_read = next(m_read + len - 1);
        m_size -= len;
        return len;
    }

    T& peek() {
        T& obj = m_c[m_read];
        return obj;
    }

    size_t peek(T* buff, size_t len = 0) {
        if (len <= 0 || m_size < len) len = m_size;
        for (size_t i = 0; i < len; ++i) {
            buff[i] = (*this)[i];
        }
        return len;
    }

    size_t dmaCounter(size_t counter) {
        // DMA counter is number of bytes remaining
        // We want the number of bytes written
        counter = m_capacity - counter;

        if (counter < m_lastDmaCounter) m_lastDmaCounter -= m_capacity;
        size_t diff = counter - m_lastDmaCounter;

        m_write += diff;
        m_size += diff;

        // Wrap if needed
        while (m_write >= m_capacity) {
            m_write -= m_capacity;
        }

        // Check overload
        if (m_size > m_capacity) {
            m_read = next(m_read + m_size - m_capacity - 1);
            m_size = m_capacity;
        }
        m_lastDmaCounter = counter;

        return diff;
    }

    size_t getWritePos() const { return m_write; }

    size_t getReadPos() const { return m_read; }

    void setReadPos(size_t readPos) {
        while (readPos > m_capacity) readPos -= m_capacity;
        m_read = readPos;
        m_size = m_write - m_read;
        if (m_size < 0) m_size += m_capacity;
    }

    /**
     * Return the number of elements inserted in the buffer
     * Does not return the max size of the buffer
     * Elements inserted using operator [] are ignored
     */
    size_t size() const { return m_size; }

    size_t capacity() const { return m_capacity; }

    /**
     * Reset the cursor and size of the buffer to 0
     * Effectively clearing the buffer
     */
    void clear(void) {
        m_size  = 0;
        m_read  = 0;
        m_write = 0;
    }

    /**
     * Access to the underlying array
     */
    T* data() { return m_c; }

    /**
     * array access operator for writing
     * Note: does not update cursors position nor array size counter;
     */
    T& operator[](int i) { return m_c[idx(i)]; }

    /**
     * array access operation for reading
     */
    T operator[](int i) const { return m_c[idx(i)]; }

  private:
    T*     m_c        = nullptr;
    bool   m_external = false;
    size_t m_capacity;    // Cannot be const, because we want default operator=
    // This is used so we have different sized buffer
    int32_t m_read  = 0;
    int32_t m_write = 0;
    size_t  m_size  = 0;

    int32_t m_lastDmaCounter = 0;

    size_t next(size_t origin) const {
        size_t r = origin + 1;
        while (r >= m_capacity) r -= m_capacity;
        return r;
    }

    size_t idx(size_t idx) const {
        size_t r = m_read + idx;
        while (r >= m_capacity) r -= m_capacity;
        return r;
    }
};

#endif
