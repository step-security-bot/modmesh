#pragma once

/*
 * Copyright (c) 2024, Yung-Yu Chen <yyc@solvcon.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the copyright holder nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <modmesh/base.hpp>
#include <modmesh/buffer/ConcreteBuffer.hpp>

#include <stdexcept>
#include <memory>
#include <algorithm>
#include <sstream>

namespace modmesh
{

/**
 * Untyped and growing memory buffer for contiguous data storage.  The internal
 * expandable memory buffer cannot be used externally.
 */
class BufferExpander
    : public std::enable_shared_from_this<BufferExpander>
{

private:

    struct ctor_passkey
    {
    };

public:

    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template <typename... Args>
    static std::shared_ptr<BufferExpander> construct(Args &&... args)
    {
        return std::make_shared<BufferExpander>(std::forward<Args>(args)..., ctor_passkey());
    }

    std::shared_ptr<BufferExpander> clone()
    {
        return BufferExpander::construct(copy_concrete(), /*clone*/ false);
    }

    BufferExpander(std::shared_ptr<ConcreteBuffer> const & buf, bool clone, ctor_passkey const &)
        : m_concrete_buffer(clone ? buf->clone() : buf)
        , m_begin(m_concrete_buffer->data())
        , m_end(m_begin + size())
        , m_end_cap(m_begin + m_concrete_buffer->size())
    {
    }

    BufferExpander(size_type nbyte, ctor_passkey const &)
    {
        expand(nbyte);
    }

    BufferExpander(ctor_passkey const &) {}

    BufferExpander() = delete;
    BufferExpander(BufferExpander const &) = delete;
    BufferExpander(BufferExpander &&) = delete;
    BufferExpander & operator=(BufferExpander const &) = delete;
    BufferExpander & operator=(BufferExpander &&) = delete;
    ~BufferExpander() = default;

    size_type size() const noexcept
    {
        return static_cast<size_type>(this->m_end - this->m_begin);
    }

    size_type capacity() const noexcept
    {
        return static_cast<size_type>(this->m_end_cap - this->m_begin);
    }

    void reserve(size_type cap);

    void expand(size_type length)
    {
        reserve(length);
        m_end = m_begin + length;
    }

    std::shared_ptr<ConcreteBuffer> copy_concrete(size_type cap = 0) const;
    std::shared_ptr<ConcreteBuffer> const & as_concrete(size_type cap = 0);
    bool is_concrete() const { return bool(m_concrete_buffer); }

    int8_t operator[](size_t it) const { return data(it); }
    int8_t & operator[](size_t it) { return data(it); }

    int8_t at(size_t it) const
    {
        validate_range(it);
        return data(it);
    }
    int8_t & at(size_t it)
    {
        validate_range(it);
        return data(it);
    }

    /* Backdoor */
    int8_t data(size_type it) const { return data()[it]; }
    int8_t & data(size_type it) { return data()[it]; }
    int8_t const * data() const noexcept { return data<int8_t>(); }
    int8_t * data() noexcept { return data<int8_t>(); }
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    template <typename T> T const * data() const noexcept { return reinterpret_cast<T *>(m_begin); }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    template <typename T> T * data() noexcept { return reinterpret_cast<T *>(m_begin); }
    // clang-format on

private:

    void validate_range(size_type it) const
    {
        if (it >= size())
        {
            throw std::out_of_range(Formatter() << "BufferExpander: index " << it << " is out of bounds with size " << size());
        }
    }

    static std::unique_ptr<int8_t> allocate(size_type nbytes)
    {
        std::unique_ptr<int8_t> ret(nullptr);
        if (0 != nbytes)
        {
            ret = std::unique_ptr<int8_t>(new int8_t[nbytes]);
        }
        return ret;
    }

    std::unique_ptr<int8_t> m_data_holder = nullptr;
    std::shared_ptr<ConcreteBuffer> m_concrete_buffer = nullptr;

    int8_t * m_begin = nullptr;
    int8_t * m_end = nullptr;
    int8_t * m_end_cap = nullptr;

}; /* end class BufferExpander */

} /* end namespace modmesh */

/* vim: set et ts=4 sw=4: */