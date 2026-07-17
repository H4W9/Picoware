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

// These operators install for the WHOLE link, ESP-IDF's C++ included. On the
// ESP32-C5 boards that is fatal: app_main() runs nvs_flash_init() (which is C++)
// BEFORE mp_task starts gc_init()/mp_init() (ports/esp32/main.c:282 vs the task
// spawn), so NVS's operator delete[] lands in gc_free(), which reads
// MP_STATE_THREAD(gc_lock_depth) (py/gc.c) from a thread state that does not
// exist yet - a null dereference at boot, decoded from the crash on hardware.
// The C5 boards keep the compiler's new/delete (malloc/free); the engine pairs
// ENGINE_MEM_NEW/ENGINE_MEM_DELETE so that is consistent. Other boards unchanged.
#if !defined(PANCAKE) && !defined(V8)

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

#endif // !PANCAKE && !V8

#endif