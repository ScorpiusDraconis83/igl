/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <igl/Common.h>
#include <igl/ComputeCommandEncoder.h>
#include <igl/vulkan/CommandBuffer.h>
#include <igl/vulkan/ResourcesBinder.h>
#include <igl/vulkan/util/SpvReflection.h>

namespace igl {

class IComputePipelineState;
class ISamplerState;

namespace vulkan {

class ComputePipelineState;
class VulkanImage;

/// @brief Implements the igl::IComputeCommandEncoder interface for Vulkan
class ComputeCommandEncoder : public IComputeCommandEncoder {
 public:
  ComputeCommandEncoder(const std::shared_ptr<CommandBuffer>& commandBuffer, VulkanContext& ctx);
  ~ComputeCommandEncoder() override {
    IGL_DEBUG_ASSERT(!isEncoding_); // did you forget to call endEncoding()?
    endEncoding();
  }

  void bindComputePipelineState(
      const std::shared_ptr<IComputePipelineState>& pipelineState) override;
  void dispatchThreadGroups(const Dimensions& threadgroupCount,
                            const Dimensions& threadgroupSize,
                            const Dependencies& dependencies) override;

  /// @brief Ends encoding for compute commands and transitions all images bound to this encoder
  /// back to `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL`
  void endEncoding() override;

  void pushDebugGroupLabel(const char* label, const igl::Color& color) const override;
  void insertDebugEventLabel(const char* label, const igl::Color& color) const override;
  void popDebugGroupLabel() const override;

  /// @brief This is only for backends that MUST use single uniforms in some situations. Do not
  /// implement!
  void bindUniform(const UniformDesc& uniformDesc, const void* data) override;

  /// @brief Binds a texture and transitions it to `VK_IMAGE_LAYOUT_GENERAL`. If the texture is not
  /// a storage texture, this function is a no-op
  void bindTexture(uint32_t index, ITexture* texture) override;
  /// @brief Binds an image texture and transitions it to `VK_IMAGE_LAYOUT_GENERAL`. If the texture
  /// is not a storage texture, this function will assert.
  void bindImageTexture(uint32_t index, ITexture* texture, TextureFormat format) override;

  void bindSamplerState(uint32_t index, ISamplerState* samplerState) override;

  /// @brief Binds a buffer. If the buffer is not a storage buffer, this function is a no-op
  void bindBuffer(uint32_t index, IBuffer* buffer, size_t offset, size_t bufferSize) override;

  /// @brief Not implemented
  void bindBytes(uint32_t index, const void* data, size_t length) override;

  /// @brief Binds push constants pointed by `data` with `length` bytes starting at `offset`.
  /// `length` must be a multiple of 4.
  void bindPushConstants(const void* data, size_t length, size_t offset) override;

  /// @brief Returns the underlying Vulkan command buffer handle
  [[nodiscard]] VkCommandBuffer getVkCommandBuffer() const {
    return cmdBuffer_;
  }

 private:
  void processDependencies(const Dependencies& dependencies);

 private:
  VulkanContext& ctx_;
  VkCommandBuffer cmdBuffer_ = VK_NULL_HANDLE;
  bool isEncoding_ = false;

  ResourcesBinder binder_;

  std::vector<const igl::vulkan::VulkanImage*> restoreLayout_;
  std::vector<VkImageAspectFlags> restoreLayoutAspectFlags_;

  const igl::vulkan::ComputePipelineState* cps_ = nullptr;
};

} // namespace vulkan
} // namespace igl
