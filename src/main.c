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

#define WIDTH 800
#define HEIGHT 640

// const char *vert_shader_path = TEXT("shaders/shader.vert.spv");
LPCSTR vert_shader_path = "shaders/shader.vert.spv";
// const char *frag_shader_path = TEXT("shaders/shader.frag.spv");
LPCSTR frag_shader_path = "shaders/shader.frag.spv";

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
// fileops
//--------------------------------
HANDLE file_handle;
LARGE_INTEGER file_size;
LPDWORD bytes_read;

void *fileops_readBinary(LPCSTR file_path) {
    file_handle = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    Assert(file_handle != INVALID_HANDLE_VALUE, "Failed to open file!");

    BOOL result = GetFileSizeEx(file_handle, &file_size);
    Assert(result != false, "Failed to get file size");

    void *file_memory = arenaPushArrayZero(&arena, byte, file_size.QuadPart);
    result = ReadFile(file_handle, file_memory, file_size.QuadPart, bytes_read, NULL);
    Assert(result != false, "Failed to read file");

    result = CloseHandle(file_handle);
    Assert(result != false, "Failed to close file");

    return file_memory;
}

//--------------------------------
// vulkan
//--------------------------------
PFN_vkCreateInstance vkCreateInstance;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
PFN_vkCreateDevice vkCreateDevice;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
PFN_vkCreateImageView vkCreateImageView;
PFN_vkCreateRenderPass vkCreateRenderPass;
PFN_vkCreateShaderModule vkCreateShaderModule;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
PFN_vkCreateFramebuffer vkCreateFramebuffer;
PFN_vkCreateCommandPool vkCreateCommandPool;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
PFN_vkCreateSemaphore vkCreateSemaphore;
PFN_vkCreateFence vkCreateFence;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
PFN_vkCmdBindPipeline vkCmdBindPipeline;
PFN_vkCmdSetViewport vkCmdSetViewport;
PFN_vkCmdSetScissor vkCmdSetScissor;
PFN_vkCmdDraw vkCmdDraw;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
PFN_vkEndCommandBuffer vkEndCommandBuffer;
PFN_vkWaitForFences vkWaitForFences;
PFN_vkResetFences vkResetFences;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
PFN_vkResetCommandBuffer vkResetCommandBuffer;
PFN_vkQueueSubmit vkQueueSubmit;
PFN_vkQueuePresentKHR vkQueuePresentKHR;
PFN_vkGetDeviceQueue vkGetDeviceQueue;
PFN_vkDestroyShaderModule vkDestroyShaderModule;

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

    vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)GetProcAddress(vulkan_module, "vkCreateWin32SurfaceKHR");
    Assert(vkCreateWin32SurfaceKHR, "Failed to load vkCreateWin32SurfaceKHR function pointer");

    vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)GetProcAddress(vulkan_module, "vkEnumeratePhysicalDevices");
    Assert(vkEnumeratePhysicalDevices, "Failed to load vkEnumeratePhysicalDevices function pointer");

    vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)GetProcAddress(vulkan_module, "vkGetPhysicalDeviceSurfaceSupportKHR");
    Assert(vkGetPhysicalDeviceSurfaceSupportKHR, "Failed to load vkGetPhysicalDeviceSurfaceSupportKHR function pointer");

    vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)GetProcAddress(vulkan_module, "vkGetPhysicalDeviceProperties");
    Assert(vkGetPhysicalDeviceProperties, "Failed to load vkGetPhysicalDeviceProperties function pointer");

    vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)GetProcAddress(vulkan_module, "vkGetPhysicalDeviceQueueFamilyProperties");
    Assert(vkGetPhysicalDeviceQueueFamilyProperties, "Failed to load vkGetPhysicalDeviceQueueFamilyProperties function pointer");

    vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)GetProcAddress(vulkan_module, "vkEnumerateDeviceExtensionProperties");
    Assert(vkEnumerateDeviceExtensionProperties, "Failed to load vkEnumerateDeviceExtensionProperties function pointer");

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)GetProcAddress(vulkan_module, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    Assert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, "Failed to load vkGetPhysicalDeviceSurfaceCapabilitiesKHR function pointer");

    vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)GetProcAddress(vulkan_module, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    Assert(vkGetPhysicalDeviceSurfaceFormatsKHR, "Failed to load vkGetPhysicalDeviceSurfaceFormatsKHR function pointer");

    vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)GetProcAddress(vulkan_module, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    Assert(vkGetPhysicalDeviceSurfacePresentModesKHR, "Failed to load vkGetPhysicalDeviceSurfacePresentModesKHR function pointer");

    vkCreateDevice = (PFN_vkCreateDevice)GetProcAddress(vulkan_module, "vkCreateDevice");
    Assert(vkCreateDevice, "Failed to load vkCreateDevice function pointer");

    vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)GetProcAddress(vulkan_module, "vkCreateSwapchainKHR");
    Assert(vkCreateSwapchainKHR, "Failed to load vkCreateSwapchainKHR function pointer");

    vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)GetProcAddress(vulkan_module, "vkGetSwapchainImagesKHR");
    Assert(vkGetSwapchainImagesKHR, "Failed to load vkGetSwapchainImagesKHR function pointer");

    vkCreateImageView = (PFN_vkCreateImageView)GetProcAddress(vulkan_module, "vkCreateImageView");
    Assert(vkCreateImageView, "Failed to load vkCreateImageView function pointer");

    vkCreateRenderPass = (PFN_vkCreateRenderPass)GetProcAddress(vulkan_module, "vkCreateRenderPass");
    Assert(vkCreateRenderPass, "Failed to load vkCreateRenderPass function pointer");

    vkCreateShaderModule = (PFN_vkCreateShaderModule)GetProcAddress(vulkan_module, "vkCreateShaderModule");
    Assert(vkCreateShaderModule, "Failed to load vkCreateShaderModule function pointer");

    vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout)GetProcAddress(vulkan_module, "vkCreatePipelineLayout");
    Assert(vkCreatePipelineLayout, "Failed to load vkCreatePipelineLayout function pointer");

    vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)GetProcAddress(vulkan_module, "vkCreateGraphicsPipelines");
    Assert(vkCreateGraphicsPipelines, "Failed to load vkCreateGraphicsPipelines function pointer");

    vkCreateFramebuffer = (PFN_vkCreateFramebuffer)GetProcAddress(vulkan_module, "vkCreateFramebuffer");
    Assert(vkCreateFramebuffer, "Failed to load vkCreateFramebuffer function pointer");

    vkCreateCommandPool = (PFN_vkCreateCommandPool)GetProcAddress(vulkan_module, "vkCreateCommandPool");
    Assert(vkCreateCommandPool, "Failed to load vkCreateCommandPool function pointer");

    vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)GetProcAddress(vulkan_module, "vkAllocateCommandBuffers");
    Assert(vkAllocateCommandBuffers, "Failed to load vkAllocateCommandBuffers function pointer");

    vkCreateSemaphore = (PFN_vkCreateSemaphore)GetProcAddress(vulkan_module, "vkCreateSemaphore");
    Assert(vkCreateSemaphore, "Failed to load vkCreateSemaphore function pointer");

    vkCreateFence = (PFN_vkCreateFence)GetProcAddress(vulkan_module, "vkCreateFence");
    Assert(vkCreateFence, "Failed to load vkCreateFence function pointer");

    vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)GetProcAddress(vulkan_module, "vkBeginCommandBuffer");
    Assert(vkBeginCommandBuffer, "Failed to load vkBeginCommandBuffer function pointer");

    vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)GetProcAddress(vulkan_module, "vkCmdBeginRenderPass");
    Assert(vkCmdBeginRenderPass, "Failed to load vkCmdBeginRenderPass function pointer");

    vkCmdBindPipeline = (PFN_vkCmdBindPipeline)GetProcAddress(vulkan_module, "vkCmdBindPipeline");
    Assert(vkCmdBindPipeline, "Failed to load vkCmdBindPipeline function pointer");

    vkCmdSetViewport = (PFN_vkCmdSetViewport)GetProcAddress(vulkan_module, "vkCmdSetViewport");
    Assert(vkCmdSetViewport, "Failed to load vkCmdSetViewport function pointer");

    vkCmdSetScissor = (PFN_vkCmdSetScissor)GetProcAddress(vulkan_module, "vkCmdSetScissor");
    Assert(vkCmdSetScissor, "Failed to load vkCmdSetScissor function pointer");

    vkCmdDraw = (PFN_vkCmdDraw)GetProcAddress(vulkan_module, "vkCmdDraw");
    Assert(vkCmdDraw, "Failed to load vkCmdDraw function pointer");

    vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass)GetProcAddress(vulkan_module, "vkCmdEndRenderPass");
    Assert(vkCmdEndRenderPass, "Failed to load vkCmdEndRenderPass function pointer");

    vkEndCommandBuffer = (PFN_vkEndCommandBuffer)GetProcAddress(vulkan_module, "vkEndCommandBuffer");
    Assert(vkEndCommandBuffer, "Failed to load vkEndCommandBuffer function pointer");

    vkWaitForFences = (PFN_vkWaitForFences)GetProcAddress(vulkan_module, "vkWaitForFences");
    Assert(vkWaitForFences, "Failed to load vkWaitForFences function pointer");

    vkResetFences = (PFN_vkResetFences)GetProcAddress(vulkan_module, "vkResetFences");
    Assert(vkResetFences, "Failed to load vkResetFences function pointer");

    vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)GetProcAddress(vulkan_module, "vkAcquireNextImageKHR");
    Assert(vkAcquireNextImageKHR, "Failed to load vkAcquireNextImageKHR function pointer");

    vkResetCommandBuffer = (PFN_vkResetCommandBuffer)GetProcAddress(vulkan_module, "vkResetCommandBuffer");
    Assert(vkResetCommandBuffer, "Failed to load vkResetCommandBuffer function pointer");

    vkQueueSubmit = (PFN_vkQueueSubmit)GetProcAddress(vulkan_module, "vkQueueSubmit");
    Assert(vkQueueSubmit, "Failed to load vkQueueSubmit function pointer");

    vkQueuePresentKHR = (PFN_vkQueuePresentKHR)GetProcAddress(vulkan_module, "vkQueuePresentKHR");
    Assert(vkQueuePresentKHR, "Failed to load vkQueuePresentKHR function pointer");

    vkGetDeviceQueue = (PFN_vkGetDeviceQueue)GetProcAddress(vulkan_module, "vkGetDeviceQueue");
    Assert(vkGetDeviceQueue, "Failed to load vkGetDeviceQueue function pointer");

    vkDestroyShaderModule = (PFN_vkDestroyShaderModule)GetProcAddress(vulkan_module, "vkDestroyShaderModule");
    Assert(vkDestroyShaderModule, "Failed to load vkDestroyShaderModule function pointer");
}

PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = NULL;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = NULL;
PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT = NULL;
PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;

void VkLoadExtProcs(VkInstance instance) {
    // *(void **)&vkCreateDebugReportCallbackEXT = vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT" );
    // Assert(vkCreateDebugReportCallbackEXT != 0, "Failed to load vkCreateDebugReportCallbackEXT");
    // *(void **)&vkDestroyDebugReportCallbackEXT = vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT" );
    // *(void **)&vkDebugReportMessageEXT = vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT" );
    *(void **)&vkCreateDebugUtilsMessengerEXT = vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
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

const char *extensions[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
const char *layers[] = { "VK_LAYER_KHRONOS_validation" };
const char *device_extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

VkShaderModule vert_shader_module;
VkShaderModule frag_shader_module;

typedef struct VkContext VkContext;
struct VkContext {
    u32 width;
    u32 height;
    VkInstance instance;
    VkDebugUtilsMessengerEXT callback;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkSwapchainKHR swapchain;
    VkFormat swapchain_image_format;
    VkExtent2D swapchain_extent;
    VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    VkSemaphore image_available_semaphore;
    VkSemaphore render_finished_semaphore;
    VkFence in_flight_fence;

    VkImage *swapchain_images;
    u32 swapchain_images_length;

    VkImageView *swapchain_image_views;
    u32 swapchain_image_views_length;

    VkFramebuffer *swapchain_framebuffers;
    u32 swapchain_framebuffers_count;
};
VkContext context;

typedef struct SwapChainSupportDetails SwapChainSupportDetails;
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    u32 formats_count;
    VkPresentModeKHR *present_modes;
    u32 present_modes_count;
};

typedef struct QueueFamilyIndices QueueFamilyIndices;
struct QueueFamilyIndices {
    u32 graphics_family;
    u32 present_family;
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, bool *out_has_value) {
    QueueFamilyIndices indices = {};

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    void *pos = arenaGetPos(&arena);
    VkQueueFamilyProperties *queue_families = arenaPushArrayZero(&arena, VkQueueFamilyProperties, queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    bool graphics_family_has_value = false;
    bool present_family_has_value = false;
    for(i32 i = 0; i < queue_family_count; i++) {
        if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
            graphics_family_has_value = true;
        }
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, context.surface, &present_support);
        if(present_support) {
            indices.present_family = i;
            present_family_has_value = true;
        }
        if(graphics_family_has_value && present_family_has_value) {
            if(out_has_value) {
                *out_has_value = true;
            }
            break;
        }
    }
    arenaSetPos(&arena, pos);
    return indices;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, context.surface, &details.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, context.surface, &details.formats_count, NULL);
    if (details.formats_count != 0) {
        details.formats = arenaPushArrayZero(&arena, VkSurfaceFormatKHR, details.formats_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, context.surface, &details.formats_count, details.formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, context.surface, &details.present_modes_count, NULL);
    if (details.present_modes_count != 0) {
        details.present_modes = arenaPushArrayZero(&arena, VkPresentModeKHR, details.present_modes_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, context.surface, &details.present_modes_count, details.present_modes);
    }

    return details;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    bool layer_found = false;

    u32 extension_count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);

    void *pos = arenaGetPos(&arena);
    VkExtensionProperties *available_extensions = arenaPushArrayZero(&arena, VkExtensionProperties, extension_count);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, available_extensions);

    for (i32 device_extensions_count = 0; device_extensions_count < sizeofarray(device_extensions); ++device_extensions_count) {
        for (i32 available_layer_count = 0; available_layer_count < extension_count; ++available_layer_count) {
            if (strcmp(device_extensions[device_extensions_count], available_extensions[available_layer_count].extensionName) == 0) {
                layer_found = true;
                break;
            }
        }
        if(!layer_found) {
            layer_found = false;
        }
    }
    arenaSetPos(&arena, pos);
    return layer_found;
}

bool isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties device_properties = {};
    vkGetPhysicalDeviceProperties(device, &device_properties);
    bool is_discrete = false;
    if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		  is_discrete = true; 
    }

    // VkPhysicalDeviceFeatures device_features;
    // vkGetPhysicalDeviceFeatures(device, &device_features);

    bool queue_families_has_value = false;
    QueueFamilyIndices indices = findQueueFamilies(device, &queue_families_has_value);
    bool extensions_supported = checkDeviceExtensionSupport(device);
    printf("DeviceCheck::QueueFamilyIndices={.graphics_family:%d, .presernt_family:%d}.\n", indices.graphics_family, indices.present_family);

    bool swapchain_adequate = false;
    if (extensions_supported) {
        void *pos = arenaGetPos(&arena);
        SwapChainSupportDetails swapchain_support = querySwapChainSupport(device);
        swapchain_adequate = swapchain_support.present_modes_count > 0 && swapchain_support.formats_count > 0;
        arenaSetPos(&arena, pos);
    }

    return queue_families_has_value && extensions_supported && swapchain_adequate && is_discrete;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR *avaiable_formats, u32 formats_count) {
    for(u32 avaiable_formats_count = 0; avaiable_formats_count < formats_count; ++avaiable_formats_count) {
        if(avaiable_formats[avaiable_formats_count].format == VK_FORMAT_B8G8R8A8_SRGB && avaiable_formats[avaiable_formats_count].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return avaiable_formats[avaiable_formats_count];
        }
    }
    // If that also fails then we could start ranking the available formats based on how "good" they are, 
    // but in most cases it’s okay to just settle with the first format that is specified.
    return avaiable_formats[0];
}

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR *available_present_modes, u32 present_modes_count) {
    for (u32 available_present_modes_count = 0; available_present_modes_count < present_modes_count; ++available_present_modes_count) {
        if (available_present_modes[available_present_modes_count] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_modes[available_present_modes_count];
        }
    }
    // I personally think that VK_PRESENT_MODE_MAILBOX_KHR is a very nice trade-off if energy usage is not a concern. 
    // It allows us to avoid tearing while still maintaining a fairly low latency by rendering new images that are 
    // as up-to-date as possible right until the vertical blank. On mobile devices, where energy usage is more important, 
    // you will probably want to use VK_PRESENT_MODE_FIFO_KHR instead.
    return VK_PRESENT_MODE_FIFO_KHR;
}

#define hkMath_clamp_procgen(type) \
    type concat(hkMath_clamp_, type)(type value, type min, type max) { if (value < min) { return min; } else if(value > max) { return max; } else { return value; } }
hkMath_clamp_procgen(f32)
hkMath_clamp_procgen(f64)
hkMath_clamp_procgen(u8)
hkMath_clamp_procgen(u16)
hkMath_clamp_procgen(u32)
hkMath_clamp_procgen(u64)
hkMath_clamp_procgen(i8)
hkMath_clamp_procgen(i16)
hkMath_clamp_procgen(i32)
hkMath_clamp_procgen(i64)
// hkMath_clamp_gen(u32);

u32 clamp_u32(u32 value, u32 min, u32 max) { if (value < min) { return min; } else if(value > max) { return max; } else { return value; } }

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR capabilities) {
    hkMath_clamp_i32(1, 1, 1);
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        i32 width = WIDTH;
        i32 height = HEIGHT;
        // glfwGetFramebufferSize(g_window, &width, &height);
        VkExtent2D actual_extent = { (u32)width, (u32)height };
        actual_extent.width = hkMath_clamp_u32(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.height = hkMath_clamp_u32(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actual_extent;
    }
}

VkShaderModule createShaderModule(HANDLE file_handle, LARGE_INTEGER file_size) {
    VkShaderModuleCreateInfo shader_module_create_info = {};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = file_size.QuadPart;
    shader_module_create_info.pCode = file_handle;
    VkShaderModule shader_module = {};
    VkResult result = vkCreateShaderModule(context.logical_device, &shader_module_create_info, NULL, &shader_module);
    VkAssert(result, "Failed to create shader module!");
    return shader_module;
}

void recordCommandBuffer(VkCommandBuffer command_buffer, u32 image_index) {
    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0; // Optional
    begin_info.pInheritanceInfo = NULL; // Optional

    VkResult result = vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS;
    assert(result == VK_SUCCESS);

    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = context.render_pass;
    render_pass_info.framebuffer = context.swapchain_framebuffers[image_index];
    // check this:
    render_pass_info.renderArea.offset = (VkOffset2D){0};
    render_pass_info.renderArea.extent = context.swapchain_extent;
    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(context.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(context.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context.graphics_pipeline);

    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (f32)(context.swapchain_extent.width);
    viewport.height = (f32)(context.swapchain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(context.command_buffer, 0, 1, &viewport);

    VkRect2D scissor = {0};
    // check this:
    scissor.offset = (VkOffset2D){0};
    scissor.extent = context.swapchain_extent;
    vkCmdSetScissor(context.command_buffer, 0, 1, &scissor);
    vkCmdDraw(context.command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(context.command_buffer);

    result = vkEndCommandBuffer(context.command_buffer);
    assert(result == VK_SUCCESS);
}

void drawFrame() {
    vkWaitForFences(context.logical_device, 1, &context.in_flight_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(context.logical_device, 1, &context.in_flight_fence);

    u32 image_index = 0;
    vkAcquireNextImageKHR(context.logical_device, context.swapchain, UINT64_MAX, context.image_available_semaphore, VK_NULL_HANDLE, &image_index);
    vkResetCommandBuffer(context.command_buffer, 0);
    recordCommandBuffer(context.command_buffer, image_index);

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {context.image_available_semaphore};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &context.command_buffer;

    VkSemaphore signal_semaphores[] = {context.render_finished_semaphore};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    VkResult result = vkQueueSubmit(context.graphics_queue, 1, &submit_info, context.in_flight_fence);
    assert(result == VK_SUCCESS);

    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapchains[] = {context.swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;

    present_info.pImageIndices = &image_index;

    present_info.pResults = NULL;
    vkQueuePresentKHR(context.present_queue, &present_info);
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

    window_handle = CreateWindowEx(NULL, TEXT("CVulkanWin32"), TEXT("Core"), WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, WIDTH, HEIGHT, NULL, NULL, window_instance, NULL);
    context.width = WIDTH;
    context.height = HEIGHT;

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

    arenaSetPos(&arena, pos);

    // extensions
    u32 extension_count = 0;
    result = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
    VkAssert(result, "Error: vkEnumerateInstanceExtensionProperties failed!");

    VkExtensionProperties *extensions_available = arenaPushArrayZero(&arena, VkExtensionProperties, extension_count);

    result = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions_available);
    VkAssert(result, "Error: vkEnumerateInstanceExtensionProperties failed!");

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

    VkLoadExtProcs(context.instance);

    result = vkCreateDebugUtilsMessengerEXT(context.instance, &debug_callback_create_info, NULL, &context.callback);
    VkAssert(result, "Failed to create debug report callback.");

    // surface
    VkWin32SurfaceCreateInfoKHR surface_create_info = {};
    surface_create_info.hinstance = window_instance;
    surface_create_info.hwnd = window_handle;

    result = vkCreateWin32SurfaceKHR(context.instance, &surface_create_info, NULL, &context.surface);
    VkAssert(result, "Failed to create Win32Surface.");

    // physical device
    u32 physical_device_count = 0;
    result = vkEnumeratePhysicalDevices(context.instance, &physical_device_count, NULL);
    VkAssert(result, "Failed to enumerate physical devices.");

    VkPhysicalDevice *physical_devices = arenaPushArrayZero(&arena, VkPhysicalDevice, physical_device_count);
    result = vkEnumeratePhysicalDevices(context.instance, &physical_device_count, physical_devices);
    VkAssert(result, "Failed to enumerate physical devices.");

    for(i32 i = 0; i < physical_device_count; i++) {
        if(isDeviceSuitable(physical_devices[i])) {
            context.physical_device = physical_devices[i];
            break;
        }
    }
    Assert(context.physical_device != VK_NULL_HANDLE, "physical device not found!");

    bool has_value = false;
    QueueFamilyIndices indices = findQueueFamilies(context.physical_device, &has_value);

    f32 queue_priority = 1.f;

    VkDeviceQueueCreateInfo queue_create_info = {};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = indices.graphics_family;
	queue_create_info.queueCount = 1;
	queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures device_features = {0};

    VkDeviceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = &queue_create_info;  // .data;
    create_info.queueCreateInfoCount = 1;

    create_info.pEnabledFeatures = &device_features;

    create_info.enabledExtensionCount = sizeofarray(device_extensions);
    create_info.ppEnabledExtensionNames = device_extensions;
    create_info.enabledLayerCount = 0;

    result = vkCreateDevice(context.physical_device, &create_info, NULL, &context.logical_device);
    assert(result == VK_SUCCESS);

    vkGetDeviceQueue(context.logical_device, indices.graphics_family, 0, &context.graphics_queue);
    vkGetDeviceQueue(context.logical_device, indices.present_family, 0, &context.present_queue);
    
    // swapchain
    void *pos = arenaGetPos(&arena);
    SwapChainSupportDetails swapchain_support = querySwapChainSupport(context.physical_device);
    VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(swapchain_support.formats, swapchain_support.formats_count);
    VkPresentModeKHR present_mode = chooseSwapPresentMode(swapchain_support.present_modes, swapchain_support.present_modes_count);
    VkExtent2D extent = chooseSwapExtent(swapchain_support.capabilities);
    u32 image_count = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount) {
        image_count = swapchain_support.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR swapchain_create_info = {0};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = context.surface;

    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = surface_format.format;
    swapchain_create_info.imageColorSpace = surface_format.colorSpace;
    swapchain_create_info.imageExtent = extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    has_value = false;
    // QueueFamilyIndices indices = findQueueFamilies(context.physical_device, &has_value);
    uint32_t queue_family_indices[] = { indices.graphics_family, indices.present_family };

    if (indices.graphics_family != indices.present_family) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0; // Optional
        swapchain_create_info.pQueueFamilyIndices = NULL; // Optional
    }
    swapchain_create_info.preTransform = swapchain_support.capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    result = vkCreateSwapchainKHR(context.logical_device, &swapchain_create_info, NULL, &context.swapchain);
    assert(result == VK_SUCCESS);

    arenaSetPos(&arena, pos);

    // pos = arenaGetPos(&arena);
    context.swapchain_images = arenaPushArrayZero(&arena, VkImage, image_count);
    result = vkGetSwapchainImagesKHR(context.logical_device, context.swapchain, &image_count, NULL);
    assert(result == VK_SUCCESS);
    // Array_VkImage_resize(&context.swapchain_images, image_count);
    // Array_VkImage_resize(&context.swapchain_images);
    vkGetSwapchainImagesKHR(context.logical_device, context.swapchain, &image_count, context.swapchain_images);
    context.swapchain_image_format = surface_format.format;
    context.swapchain_extent = extent;
    // Array_VkImage_destroy(&context.swapchain_images); // dont destroy
    // arenaSetPos(&arena, pos); // dont destroy
    context.swapchain_images_length = image_count;

    // image views
    result = false;
    context.swapchain_image_views = arenaPushArrayZero(&arena, VkImageView, context.swapchain_images_length);
    context.swapchain_image_views_length = context.swapchain_images_length;
    for (u32 swapchain_images_count = 0; swapchain_images_count < context.swapchain_images_length; ++swapchain_images_count) {
        VkImageViewCreateInfo create_info = {0};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = context.swapchain_images[swapchain_images_count];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = context.swapchain_image_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;
        vkCreateImageView(context.logical_device, &create_info, NULL, &context.swapchain_image_views[swapchain_images_count]);
        assert(result == VK_SUCCESS);
    }

    // render pass
    VkAttachmentDescription color_attachment = {0};
    color_attachment.format = context.swapchain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {0};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    result = vkCreateRenderPass(context.logical_device, &render_pass_info, NULL, &context.render_pass);
    assert(result == VK_SUCCESS);

    // graphics pipeline
    pos = arenaGetPos(&arena);
    void *file_memory = NULL;
    file_memory = fileops_readBinary(vert_shader_path);
    vert_shader_module = createShaderModule(file_memory, file_size);
    file_memory = fileops_readBinary(frag_shader_path);
    frag_shader_module = createShaderModule(file_memory, file_size);
    arenaSetPos(&arena, pos);

    VkPipelineShaderStageCreateInfo vert_shader_stage_info = {0};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vert_shader_module;
    vert_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = {0};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = frag_shader_module;
    frag_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewport_state = {0};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blending = {0};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic_state = {0};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = (u32)sizeofarray(dynamic_states);
    dynamic_state.pDynamicStates = dynamic_states;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pushConstantRangeCount = 0;

    result = vkCreatePipelineLayout(context.logical_device, &pipeline_layout_info, NULL, &context.pipeline_layout);
    assert(result == VK_SUCCESS);

    VkGraphicsPipelineCreateInfo pipeline_info = {0};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.layout = context.pipeline_layout;
    pipeline_info.renderPass = context.render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    result = vkCreateGraphicsPipelines(context.logical_device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &context.graphics_pipeline);
    assert(result == VK_SUCCESS);

    vkDestroyShaderModule(context.logical_device, frag_shader_module, NULL);
    vkDestroyShaderModule(context.logical_device, vert_shader_module, NULL);

    // framebuffers
    context.swapchain_framebuffers = arenaPushArrayZero(&arena, VkFramebuffer, context.swapchain_image_views_length);
    for (usize swapchain_image_views_count = 0; swapchain_image_views_count < context.swapchain_image_views_length; swapchain_image_views_count++) {
        VkResult result = false;
        // VkImageView attachments[] = { context.swapchain_image_views.data[swapchain_image_views_count] };
        VkImageView *attachments = &context.swapchain_image_views[swapchain_image_views_count];

        VkFramebufferCreateInfo framebuffer_info = {0};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = context.render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = context.swapchain_extent.width;
        framebuffer_info.height = context.swapchain_extent.height;
        framebuffer_info.layers = 1;

        vkCreateFramebuffer(context.logical_device, &framebuffer_info, NULL, &context.swapchain_framebuffers[swapchain_image_views_count]);
        assert(result == VK_SUCCESS);
    }

    // command pool
    has_value = false;
    indices = findQueueFamilies(context.physical_device, &has_value);

    VkCommandPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = indices.graphics_family;

    result = vkCreateCommandPool(context.logical_device, &pool_info, NULL, &context.command_pool);
    assert(result == VK_SUCCESS);

    // command buffer
    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = context.command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    result = vkAllocateCommandBuffers(context.logical_device, &alloc_info, &context.command_buffer);
    assert(result == VK_SUCCESS);

    // sync objects
    VkSemaphoreCreateInfo semaphore_create_info = {0};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info = {0};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult semaphore_result = vkCreateSemaphore(context.logical_device, &semaphore_create_info, NULL, &context.image_available_semaphore);
    VkResult semaphore_result2 = vkCreateSemaphore(context.logical_device, &semaphore_create_info, NULL, &context.render_finished_semaphore);
    VkResult fence_result = vkCreateFence(context.logical_device, &fence_create_info, NULL, &context.in_flight_fence);
    assert(semaphore_result == VK_SUCCESS || semaphore_result2 == VK_SUCCESS || fence_result == VK_SUCCESS);

    // loop
    MSG msg;
    bool done = false;
    while(!done) {
        PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE);
        if(msg.message == WM_QUIT) {
            done = true;
        } else {
            TranslateMessage(&msg); 
            DispatchMessage(&msg);
            drawFrame();
        }
        RedrawWindow(window_handle, NULL, NULL, RDW_INTERNALPAINT);
    }

    return msg.wParam;
}
