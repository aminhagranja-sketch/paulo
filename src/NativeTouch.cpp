#include "granja/NativeTouch.hpp"
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0602
#endif
#include <windows.h>
#include <set>
#include <stdexcept>
namespace granja {
struct NativeTouch::Impl {
    HWND window{};WNDPROC previous{};Handler handler;std::set<int> fingers;
    static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wparam,LPARAM lparam) {
        auto* self=static_cast<Impl*>(GetPropW(window,L"MeuGalinheiroNativeTouch"));
        if(!self)return DefWindowProcW(window,message,wparam,lparam);
        if(message==WM_POINTERDOWN || message==WM_POINTERUPDATE || message==WM_POINTERUP) {
            UINT32 id=GET_POINTERID_WPARAM(wparam);POINTER_INFO info{};
            if(GetPointerInfo(id,&info) && info.pointerType==PT_TOUCH) {
                POINT pixel=info.ptPixelLocation;ScreenToClient(window,&pixel);
                int phase=message==WM_POINTERDOWN?0:message==WM_POINTERUPDATE?1:2;
                if(phase==0)self->fingers.insert(int(id));
                self->handler(phase,int(id),{float(pixel.x),float(pixel.y)});
                if(phase==2)self->fingers.erase(int(id));
                return 0; // Do not promote these touches into duplicate mouse clicks.
            }
        }
        if(message==WM_POINTERCAPTURECHANGED) {
            int id=int(GET_POINTERID_WPARAM(wparam));
            if(self->fingers.erase(id))self->handler(2,id,{});
        }
        if(message==WM_CANCELMODE || message==WM_KILLFOCUS) {
            for(int id:self->fingers)self->handler(2,id,{});
            self->fingers.clear();
        }
        return CallWindowProcW(self->previous,window,message,wparam,lparam);
    }
};
NativeTouch::NativeTouch(void* handle,Handler handler):impl(std::make_unique<Impl>()) {
    impl->window=static_cast<HWND>(handle);impl->handler=std::move(handler);
    if(!SetPropW(impl->window,L"MeuGalinheiroNativeTouch",impl.get()))throw std::runtime_error("Falha no registro do controle touch");
    SetLastError(0);
    impl->previous=reinterpret_cast<WNDPROC>(SetWindowLongPtrW(impl->window,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(&Impl::procedure)));
    if(!impl->previous){RemovePropW(impl->window,L"MeuGalinheiroNativeTouch");throw std::runtime_error("Falha ao instalar controle touch Windows");}
}
NativeTouch::~NativeTouch() {
    if(impl && IsWindow(impl->window)) {
        SetWindowLongPtrW(impl->window,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(impl->previous));
        RemovePropW(impl->window,L"MeuGalinheiroNativeTouch");
    }
}
}
#else
namespace granja {
struct NativeTouch::Impl {};
NativeTouch::NativeTouch(void*,Handler){}
NativeTouch::~NativeTouch()=default;
}
#endif
