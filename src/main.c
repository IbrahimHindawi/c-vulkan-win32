#define UNICODE
#include <windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include "vulkan/vulkan.h"

#include <stdio.h>
#define SAHA_IMPLEMENTATION
#include <saha.h>

Arena arena;
void *pos;

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
typedef struct VkContext VkContext;
struct VkContext {
    u32 width;
    u32 height;
    VkInstance instance;
    VkDebugUtilsMessengerEXT callback;
};
VkContext context;

PFN_vkCreateInstance vkCreateInstance;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

void VkLoadProcs() {
    HMODULE vulkan_module = LoadLibrary(TEXT("vulkan-1.dll"));
    Assert(vulkan_module, "Failed to load vulkan module!");


    vkCreateInstance = (PFN_vkCreateInstance)GetProcAddress(vulkan_module, "vkCreateInstance");
    Assert(vkCreateInstance, "Failed to load vkCreateInstance function pointer.");

    vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)GetProcAddress(vulkan_module, "vkEnumerateInstanceLayerProperties");
    Assert(vkEnumerateInstanceLayerProperties, "Failed to load vkEnumerateInstanceLayerProperties function pointer.");

    vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)GetProcAddress(vulkan_module, "vkEnumerateInstanceExtensionProperties");
    Assert(vkEnumerateInstanceExtensionProperties, "Failed to load vkEnumerateInstanceExtensionProperties function pointer.");

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(vulkan_module, "vkGetInstanceProcAddr");
    Assert(vkGetInstanceProcAddr, "Failed to load vkGetInstanceProcAddr function pointer");
}

PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = NULL;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = NULL;
PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT = NULL;
PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;

void VkLoadExtProcs(VkContext *context ) {
    // *(void **)&vkCreateDebugReportCallbackEXT = vkGetInstanceProcAddr(context->instance, "vkCreateDebugReportCallbackEXT" );
    // Assert(vkCreateDebugReportCallbackEXT != 0, "Failed to load vkCreateDebugReportCallbackEXT");
    // *(void **)&vkDestroyDebugReportCallbackEXT = vkGetInstanceProcAddr(context->instance, "vkDestroyDebugReportCallbackEXT" );
    // *(void **)&vkDebugReportMessageEXT = vkGetInstanceProcAddr(context->instance, "vkDebugReportMessageEXT" );
    *(void **)&vkCreateDebugUtilsMessengerEXT = vkGetInstanceProcAddr(context->instance, "vkCreateDebugUtilsMessengerEXT");
    Assert(vkCreateDebugUtilsMessengerEXT != 0, "Failed to load vkCreateDebugUtilsMessengerEXT");
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugReportCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
    OutputDebugStringA("Validation Layer: ");
    // OutputDebugStringA(messageSeverity);
    // OutputDebugStringA(" ");
    // offsetof(VkDebugUtilsMessengerCallbackDataEXT, pMessage);
    OutputDebugStringA(pCallbackData->pMessage);
    OutputDebugStringA("\n");
    return VK_FALSE;
}

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
    arenaInit(&arena);
    pos = arenaGetPos(&arena);

    window_instance = GetModuleHandle(0);
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = WindowProc;
    window_class.hInstance = window_instance;
    window_class.lpszClassName = TEXT("CVulkanWin32");
    RegisterClassEx(&window_class);

    window_handle = CreateWindowEx(NULL, TEXT("CVulkanWin32"), TEXT("Core"), WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 800, 640, NULL, NULL, window_instance, NULL);

    VkLoadProcs();

    VkResult result = {};

    // layers
    u32 layer_count = 0;
    result = vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    VkAssert(result, "Failed to create Enumerate Instance Layer Properties.");

    Assert(layer_count != 0, "Failed to find any layer in your system.");

    VkLayerProperties *layer_available = arenaPushArrayZero(&arena, VkLayerProperties, layer_count);
    result = vkEnumerateInstanceLayerProperties(&layer_count, layer_available);
    VkAssert(result, "Failed to create Enumerate Instance Layer Properties.");

    bool found_validation = false;
    for(int i = 0; i < layer_count; ++i) {
       if(strcmp(layer_available[i].layerName, "VK_LAYER_KHRONOS_validation") == 0) {
            found_validation = true;
       }
    }
    Assert(found_validation, "Could not find validation layer.");
    const char *layers[] = { "VK_LAYER_KHRONOS_validation" };

    arenaSetPos(&arena, pos);

    // extensions
    u32 extension_count = 0;
    result = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
    VkAssert(result, "Error: vkEnumerateInstanceExtensionProperties failed!");

    VkExtensionProperties *extensions_available = arenaPushArrayZero(&arena, VkExtensionProperties, extension_count);

    result = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions_available);
    VkAssert(result, "Error: vkEnumerateInstanceExtensionProperties failed!");

    const char *extensions[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
    u32 number_required_extensions = sizeofarray(extensions);
    u32 found_extensions = 0;
    for(u32 i = 0; i < extension_count; ++i) {
        for(int j = 0; j < number_required_extensions; ++j) {
            if(strcmp( extensions_available[i].extensionName, extensions[j] ) == 0) {
                found_extensions++;
            }
        }
    }
    Assert(found_extensions == number_required_extensions, "Could not find debug extension");
    arenaSetPos(&arena, pos);

    // instance
    VkApplicationInfo app_info = { };
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "CVulkanWin32";
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo instance_info = { };
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;
    // validation layers & debug extensions
    instance_info.ppEnabledExtensionNames = extensions;
    instance_info.enabledExtensionCount = sizeofarray(extensions);
    instance_info.ppEnabledLayerNames = layers;
    instance_info.enabledLayerCount = sizeofarray(layers);

    // debug
    VkDebugUtilsMessengerCreateInfoEXT debug_callback_create_info = {};
    debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_callback_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_callback_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_callback_create_info.pfnUserCallback = &vkDebugReportCallback;

    instance_info.pNext  = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_callback_create_info;

    result = vkCreateInstance(&instance_info, NULL, &context.instance);
    VkAssert(result, "Failed to create Vulkan Instance.");

    VkLoadExtProcs(&context);

    result = vkCreateDebugUtilsMessengerEXT(context.instance, &debug_callback_create_info, NULL, &context.callback);
    VkAssert(result, "Failed to create debug report callback.");

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
