#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <map>
#include <optional>

class HelloTriangleApplication 
{
public: //public functions  
    void run() 
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private: //private variables
    GLFWwindow* window;

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const std::vector<const char*> validationLayers = {
       "VK_LAYER_KHRONOS_validation"
    };

    #ifdef DEBUG
        const bool enableValidationLayers = true;
    #else
        const bool enableValidationLayers = false;
    #endif
    
    vk::Instance inst;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

private:
    void initWindow() 
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);
    }
    void initVulkan() 
    {
        createInstance();
        setupDebugMessenger();
        pickPhysicalDevice();
        createLogicalDevice();
    }
    void mainLoop() 
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
        }
    }
    void cleanup() 
    {
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void createInstance()
    {
        if (enableValidationLayers && !checkValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};

        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

        if (enableValidationLayers) 
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        }
        else 
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

        if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create instance!");
        }
    }
    bool checkValidationLayerSupport()
    {
        uint32_t layerCount;

        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        std::cout << "Validation layers: " << std::endl;
        for (const auto layer : validationLayers)
        {
            std::cout << "\t" << layer;
            bool layerFound = false;
            for (const auto availableLayer : availableLayers)
            {
                if (strcmp(availableLayer.layerName, layer) == 0)
                {
                    layerFound = true;
                    std::cout << " <available>" << std::endl;
                    break;
                }
            }

            if (!layerFound)
            {
                std::cout << " <not available>" << std::endl;
                return false;
            }
        }
        return true;
    }
    std::vector<const char*> getRequiredExtensions() 
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<VkExtensionProperties> availableExtensions(glfwExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &glfwExtensionCount, availableExtensions.data());

        std::cout << "Required extensions: " << std::endl;

        for (int i = 0; i < glfwExtensionCount; i++)
        {
            std::cout << "\t" << glfwExtensions[i];
            bool extensionAvailable = false;
            for (const auto extension : availableExtensions)
            {
                if (strcmp(extension.extensionName, glfwExtensions[i]) == 0)
                {
                    extensionAvailable = true;
                    std::cout << " <available>" << std::endl;
                    break;
                }
            }

            if (!extensionAvailable)
            {
                std::cout << " <not available>" << std::endl;
            }
        }

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData) 
    {

        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }
    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }
    
    void setupDebugMessenger()
    {
        if (!enableValidationLayers) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) 
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }
    
    void pickPhysicalDevice() 
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDeviceGroups(instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        std::multimap<int, VkPhysicalDevice> deviceCandidate;
        for (const auto& device : devices)
        {
            int score = deviceSuitability(device);
            deviceCandidate.insert(std::make_pair(score, device));
        }

        if (deviceCandidate.rbegin()->first > 0)
        {
            physicalDevice = deviceCandidate.rbegin()->second;
            
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);

            std::cout << "Selected GPU: " << properties.deviceName << std::endl;
        }
        else
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }
    int deviceSuitability(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(device, &properties);
        vkGetPhysicalDeviceFeatures(device, &features);

        int score = 0;

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            score += 1000;
        }
        score += properties.limits.maxImageDimension2D;
        if (!features.geometryShader) return 0;

        QueueFamilyIndices indices = findQueueFamilies(device);
        if (!indices.isComplete()) return 0;
        
        return score;
    }

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        bool isComplete() 
        {
            return graphicsFamily.has_value();
        }
    };
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            i++;
            if (indices.isComplete()) break;
        }

        return indices;
    }

    void createLogicalDevice()
    {

    }
};


int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}