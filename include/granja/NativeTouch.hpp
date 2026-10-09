#pragma once
#include "Types.hpp"
#include <functional>
#include <memory>
namespace granja {
// SFML's Win32 backend does not emit Touch events. Adapt native WM_POINTER.
class NativeTouch {
public:
    using Handler=std::function<void(int phase,int id,Vec pixel)>;
    NativeTouch(void* window,Handler handler);
    ~NativeTouch();
    NativeTouch(const NativeTouch&)=delete;
    NativeTouch& operator=(const NativeTouch&)=delete;
private:
    struct Impl;std::unique_ptr<Impl> impl;
};
}
