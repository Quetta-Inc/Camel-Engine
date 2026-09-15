#include "camel/VulkanPhysicalDevice.hpp"
#include "camel/VulkanSwapchain.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

void expect(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

} // namespace

int main() {
    try {
        const VkSurfaceFormatKHR preferred{
            VK_FORMAT_B8G8R8A8_SRGB,
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        };
        const VkSurfaceFormatKHR fallback{
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        };
        expect(
            VulkanSwapchain::chooseSurfaceFormat({fallback, preferred}).format ==
                VK_FORMAT_B8G8R8A8_SRGB,
            "Swapchain should prefer the standard SRGB surface format"
        );
        expect(
            VulkanSwapchain::chooseSurfaceFormat({fallback}).format ==
                VK_FORMAT_R8G8B8A8_UNORM,
            "Swapchain should fall back to the first available format"
        );
        expect(
            VulkanSwapchain::chooseSurfaceFormat(
                {{VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}}
            ).format == VK_FORMAT_B8G8R8A8_SRGB,
            "Undefined surface format should select the engine default"
        );
        expect(
            VulkanSwapchain::choosePresentMode({VK_PRESENT_MODE_FIFO_KHR}) ==
                VK_PRESENT_MODE_FIFO_KHR,
            "FIFO must be the safe present-mode fallback"
        );
        expect(
            VulkanSwapchain::choosePresentMode(
                {VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR}
            ) == VK_PRESENT_MODE_MAILBOX_KHR,
            "Swapchain should prefer mailbox when available"
        );

        QueueFamilyIndices incomplete;
        expect(!incomplete.isComplete(), "Empty queue-family selection is complete");
        incomplete.graphics = 0;
        incomplete.present = 1;
        expect(incomplete.isComplete(), "Complete queue-family selection was rejected");
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }

    return 0;
}
