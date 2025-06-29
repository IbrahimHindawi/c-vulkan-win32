#define UNICODE
#include <windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include "vulkan/vulkan.h"

#include <stdio.h>
#include <saha.h>

Arena arena;

HMODULE window_instance;
WNDCLASSEX window_class;
HINSTANCE window_instance;
HWND window_handle;

//--------------------------------
// asserts
//--------------------------------
void Assert(bool flag, char *msg) {
    if(!flag) {
        OutputDebugStringA("ASSERT: ");
        OutputDebugStringA(msg);
        OutputDebugStringA("\n");
        int *base = 0;
        *base = 1;
    }
}

void VkAssert(VkResult result, char *msg) {
    Assert(result == VK_SUCCESS, msg);
}

//--------------------------------
// vulkan
//--------------------------------
PFN_vkCreateInstance vkCreateInstance;

void VkLoad() {
    HMODULE vulkan_module = LoadLibrary(TEXT("vulkan-1.dll"));
    Assert(vulkan_module, "Failed to load vulkan module!");


    vkCreateInstance = (PFN_vkCreateInstance)GetProcAddress(vulkan_module, "vkCreateInstance");
    Assert(vkCreateInstance, "Failed to load vkCreateInstance function pointer.");
}

typedef struct VkContext VkContext;
struct VkContext {
    u32 width;
    u32 height;
    VkInstance instance;
};
VkContext context;

//--------------------------------
// win32
//--------------------------------
LRESULT CALLBACK WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    switch( uMsg ) {
        case WM_CLOSE: { 
            PostQuitMessage( 0 );
            break;
        }
        default: {
            break;
        }
    }
    
    // a pass-through for now. We will return to this callback
    return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

int main(int argc, char *argv[]) {
    printf("Hell, World!\n");
    window_instance = GetModuleHandle(0);
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = WindowProc;
    window_class.hInstance = window_instance;
    window_class.lpszClassName = TEXT("CVulkanWin32");
    RegisterClassEx(&window_class);

    window_handle = CreateWindowEx(NULL, TEXT("CVulkanWin32"), TEXT("Core"), WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 800, 640, NULL, NULL, window_instance, NULL);

    VkLoad();

    VkApplicationInfo app_info = { };
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "CVulkanWin32";
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo instance_info = { };
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;
    instance_info.enabledLayerCount = 0;
    instance_info.ppEnabledLayerNames = NULL;
    instance_info.enabledExtensionCount = 0;
    instance_info.ppEnabledExtensionNames = NULL;

    VkResult result = {};
    result = vkCreateInstance(&instance_info, NULL, &context.instance);
    VkAssert(result, "Failed to create Vulkan Instance");

    MSG msg;
    bool done = false;
    while( !done ) {
        PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE );
        if( msg.message == WM_QUIT ) {
            done = true;
        } else {
            TranslateMessage( &msg ); 
            DispatchMessage( &msg );
        }

         RedrawWindow( window_handle, NULL, NULL, RDW_INTERNALPAINT );
    }

    return msg.wParam;
}
