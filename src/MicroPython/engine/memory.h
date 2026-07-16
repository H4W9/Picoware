#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "py/runtime.h"

#ifdef __cplusplus
}

#include <new>
#include <cstddef>

// Overriding these installs them for the WHOLE link, not just Picoware: any C++
// in ESP-IDF gets them too. On the Pancake that is fatal. ESP-IDF's NVS is C++,
// and app_main() calls boardctrl_startup() -> nvs_flash_init() BEFORE it starts
// mp_task, which is what calls gc_init()/mp_init() (ports/esp32/main.c:278 vs
// 281 -> 139). So NVS's delete[] lands in gc_free(), whose first act is to read
// MP_STATE_THREAD(gc_lock_depth) from a thread state that does not exist yet -
// a null dereference at boot, before any of Picoware runs.
//
// So this board keeps the compiler's new/delete (malloc/free). That is
// self-consistent because ENGINE_MEM_NEW/ENGINE_MEM_DELETE are always paired,
// and malloc here is PSRAM-backed anyway. Other boards are untouched.
#ifndef PANCAKE

// Route global new/delete through MicroPython's allocator
inline void *operator new(std::size_t size) { return m_new(uint8_t, size); }
inline void *operator new[](std::size_t size) { return m_new(uint8_t, size); }
inline void operator delete(void *p) noexcept
{
    if (p)
        m_del(uint8_t, (uint8_t *)p, 1);
}
inline void operator delete[](void *p) noexcept
{
    if (p)
        m_del(uint8_t, (uint8_t *)p, 1);
}
inline void operator delete(void *p, std::size_t size) noexcept
{
    if (p)
        m_del(uint8_t, (uint8_t *)p, size);
}
inline void operator delete[](void *p, std::size_t size) noexcept
{
    if (p)
        m_del(uint8_t, (uint8_t *)p, size);
}

#endif // !PANCAKE

#endif