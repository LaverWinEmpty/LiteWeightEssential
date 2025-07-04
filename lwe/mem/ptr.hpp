#ifndef LWE_MEM_PTR
#define LWE_MEM_PTR

#include "../base/base.h"
#include "../mem/block.hpp"
#include "../util/id.hpp"
#include "../diag/diag.h"
#include "pool.hpp"

/**************************************************************************************************
 * Ptr<T>: unique_ptr + weak_ptr hybrid smart pointer
 *
 * - Acts like std::unique_ptr when created directly.
 * - Acts like std::weak_ptr when copied (non-owning).
 * - Transfer ownership with `own()`.
 * - Check ownership with `owned()`.
 * - Validate pointer state (null/dangling) with `valid()`.
 * - Perform copy-on-write with `clone()` method.
 *
 * Use case: lightweight smart pointer for memory safety with minimal overhead.
 **************************************************************************************************/

LWE_BEGIN
namespace mem {
//! @brief a non-releasable memory pool dedicated to `Ptr`
template<typename T> class Slotmap {
    using Adapter = Slotmap<mem::Block<sizeof(T)>>;
public:
    static T*   acquire();
    static void release(T* in);
};

template<typename T> class Ptr {
    using Deleter = void (*)(T*); //!< custom destructor

    //! constrcutor SFINAE for incomplete type
    template<typename U, typename... Args> using Enable = std::enable_if_t<std::is_constructible_v<U, Args...>>;

    //! block base
    struct Block {
        uint64_t id    = 0;
        Ptr*     owner = nullptr;
    };

    //! internal data control block
    struct Internal: Block {
        ~Internal() { /* not work */ }
        union {
            T       data;
            uint8_t ptr[sizeof(T)] = { 0 }; // memset
        };
    };

    //! external data control block
    struct External: Block {
        T* ptr = nullptr;
    };

private:
    bool initialize(bool); // malloc
    bool release();        // free

public:
    //! default set nullptr
    Ptr();

public:
    //! @param [in] T* exnernal pointer
    //! @param [in] Deleter destructor lambda: default delete in, ignore nullptr
    Ptr(T*, Deleter = [](T* in) { delete in; });

public:
    //! @brief copy const
    //! @param [in] U is T, for incomplete type
    template<typename U, typename = Enable<T, const U&>> Ptr(const U&);

public:
    //! @brief move
    //! @param [in] U is T, for incomplete type
    template<typename U, typename = Enable<T, U&&>> Ptr(U&&);

public:
    ~Ptr();
    Ptr(const Ptr&);                //!< shallow copy
    Ptr(Ptr&&) noexcept;            //!< move
    Ptr& operator=(const Ptr&);     //!< shallow copy
    Ptr& operator=(Ptr&&) noexcept; //!< move

public:
    T*       operator->();       //!< get ptr
    const T* operator->() const; //!< get ptr const
    T&       operator*();        //!< get ref
    const T& operator*() const;  //!< get ref const

public:
    bool operator==(void*) const;      //!< compare to pointer
    bool operator!=(void*) const;      //!< compare to pointer
    bool operator==(const Ptr&) const; //!< compare if the block are the same
    bool operator!=(const Ptr&) const; //!< compare if the block are the same

public:
    explicit operator bool() const;     //! check nullptr
    explicit operator T*();             //! raw pointer
    explicit operator const T*() const; //! raw pointer const

public:
    //! @brief to unique, NEED: copy constructor
    //! @return false: bad alloc
    bool clone();

public:
    T*       get();       //!< get
    const T* get() const; //!< get

public:
    template<typename U> U*       as();       //!< cast
    template<typename U> const U* as() const; //!< cast

public:
    bool owned() const; //!< get ownership
    bool own();         //!< set owner
    bool valid() const; //!< check null and dangling

private:
    Block*   block;   //!< block
    Deleter  deleter; //!< custom deleter
    uint64_t id = 0;  //!< dangling checker
    bool     pointer; //!< is pointer flag
};

} // namespace mem
LWE_END
#include "ptr.ipp"
#endif
