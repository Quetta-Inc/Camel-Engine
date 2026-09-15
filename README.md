# Camel Engine

Small Vulkan renderer built on SDL3 and GLM. The repository is organized by
role so that Vulkan objects are created and destroyed in dependency order.

## Project layout

```text
include/camel/       public C++ headers
src/                 engine implementation and application entry point
tests/               unit, integration, and CTest helper tests
cmake/               CMake test and build helpers
assets/              source shaders and runtime texture
external/stb/        vendored stb_image single-header dependency
```

## Vulkan ownership order

```text
VulkanInstance
  -> VulkanSurface
  -> VulkanPhysicalDevice
  -> VulkanDevice
  -> VulkanSwapchain
      -> VulkanImageViews
      -> VulkanDepthBuffer
          -> VulkanRenderPass
              -> VulkanFramebuffers
              -> VulkanPipeline
```

`Engine` coordinates these components; each component owns only the Vulkan
handles that belong to its responsibility.

## Build and test

Configure with a vcpkg toolchain that provides SDL3, Vulkan, and GLM:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

The build compiles GLSL shaders into `build/generated_shaders`; generated
artifacts are intentionally not stored in the source tree.
