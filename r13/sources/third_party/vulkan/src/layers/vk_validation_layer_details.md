[TOC]

# Validation Layer Details

## VK_LAYER_LUNARG_standard_validation

### VK_LAYER_LUNARG_standard_validation Overview

This is a meta-layer managed by the loader. Specifying this layer name will cause the loader to load the all of the standard validation layers in the following optimal order:

 - VK_LAYER_GOOGLE_threading
 - VK_LAYER_LUNARG_parameter_validation
 - VK_LAYER_LUNARG_device_limits
 - VK_LAYER_LUNARG_object_tracker
 - VK_LAYER_LUNARG_image
 - VK_LAYER_LUNARG_core_validation
 - VK_LAYER_LUNARG_swapchain
 - VK_LAYER_GOOGLE_unique_objects

Other layers can be specified and the loader will remove duplicates. See the following individual layer descriptions for layer details.

## VK_LAYER_LUNARG_core_validation

### VK_LAYER_LUNARG_core_validation Overview

The VK_LAYER_LUNARG_core_validation layer is the main layer performing state tracking, object and state lifetime validation, and consistency and coherency between these states and the requirements, limits, and capabilities. Currently, it is divided into three main areas of validation:  Draw State, Memory Tracking, and Shader Checking.

### VK_LAYER_LUNARG_core_validation Draw State Details Table
The Draw State portion of the core validation layer tracks state leading into Draw commands. This includes the Pipeline state, dynamic state, shaders, and descriptor set state. This functionality validates the consistency and correctness between and within these states.

| Check | Overview | ENUM DRAWSTATE_* | Relevant API | Testname | Notes/TODO |
| ----- | -------- | ---------------- | ------------ | -------- | ---------- |
| Valid Pipeline Layouts | Verify that sets being bound are compatible with their PipelineLayout and that the last-bound PSO PipelineLayout at Draw time is compatible with all bound sets used by that PSO | PIPELINE_LAYOUTS_INCOMPATIBLE | vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect | DescriptorSetCompatibility | None |
| Valid BeginCommandBuffer state | Must not call Begin on command buffers that are being recorded, and primary command buffers must specify VK_NULL_HANDLE for RenderPass or Framebuffer parameters, while secondary command buffers must provide non-null parameters,  | BEGIN_CB_INVALID_STATE | vkBeginCommandBuffer | CallBeginCommandBufferBeforeCompletion SecondaryCommandBufferNullRenderpass | None |
| Command Buffer Simultaneous Use | Violation of VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT rules. Most likely attempting to simultaneously use a CmdBuffer w/o having that bit set. This also warns if you add secondary command buffer w/o that bit set to a primary command buffer that does have that bit set. | INVALID_CB_SIMULTANEOUS_USE | vkQueueSubmit vkCmdExecuteCommands | TODO | Write test |
| Valid Command Buffer Reset | Can only reset individual command buffer that was allocated from a pool with VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT set | INVALID_COMMAND_BUFFER_RESET | vkBeginCommandBuffer vkResetCommandBuffer | CommandBufferResetErrors | None |
| PSO Bound | Verify that a properly created and valid pipeline object is bound to the CommandBuffer specified in these calls | NO_PIPELINE_BOUND | vkCmdBindDescriptorSets vkCmdBindVertexBuffers | PipelineNotBound | This check is currently more related to VK_LAYER_LUNARG_core_validation internal data structures and less about verifying that PSO is bound at all appropriate points in API. For API purposes, need to make sure this is checked at Draw time and any other relevant calls. |
| Valid DescriptorPool | Verifies that the descriptor set pool object was properly created and is valid | INVALID_POOL | vkResetDescriptorPool vkAllocateDescriptorSets | TODO | This is just an internal layer data structure check. VK_LAYER_LUNARG_parameter_validation or VK_LAYER_LUNARG_object_tracker should really catch bad DSPool |
| Valid DescriptorSet | Validate that descriptor set was properly created and is currently valid | INVALID_SET | vkCmdBindDescriptorSets | TODO | Is this needed other places (like Update/Clear descriptors) |
| Valid DescriptorSetLayout | Flag DescriptorSetLayout object that was not properly created | INVALID_LAYOUT | vkAllocateDescriptorSets | TODO | Anywhere else to check this? |
| Valid RenderArea | Flag renderArea field that is outside of the framebuffer | INVALID_RENDER_AREA | vkCmdBeginRenderPass | TODO | Anywhere else to check this? |
| Valid Pipeline | Flag VkPipeline object that was not properly created, or case when Draw/Dispatch is bound to cmd buffer without a pipeline being bound | INVALID_PIPELINE | vkCmdBindPipeline | InvalidPipeline | NA |
| Valid PipelineLayout | Flag VkPipelineLayout object that was not properly created | INVALID_PIPELINE_LAYOUT | vkCmdBindPipeline | TODO | Write test for this case |
| Valid Pipeline Create Info | Tests for the following: That compute shaders are not specified for the graphics pipeline, tess evaluation and tess control shaders are included or excluded as a pair, that VK_PRIMITIVE_TOPOLOGY_PATCH_LIST is set as IA topology for tessellation pipelines, that VK_PRIMITIVE_TOPOLOGY_PATCH_LIST primitive topology is only set for tessellation pipelines, and that Vtx Shader specified | INVALID_PIPELINE_CREATE_STATE | vkCreateGraphicsPipelines | InvalidPipelineCreateState | NA |
| Valid CommandBuffer | Validates that the command buffer object was properly created and is currently valid | INVALID_COMMAND_BUFFER | vkQueueSubmit vkBeginCommandBuffer vkEndCommandBuffer vkCmdBindPipeline vkCmdBindDescriptorSets vkCmdBindIndexBuffer vkCmdBindVertexBuffers vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect vkCmdDispatch vkCmdDispatchIndirect vkCmdCopyBuffer vkCmdCopyImage vkCmdBlitImage vkCmdCopyBufferToImage vkCmdCopyImageToBuffer vkCmdUpdateBuffer vkCmdFillBuffer vkCmdClearAttachments vkCmdClearColorImage vkCmdClearDepthStencilImage vkCmdResolveImage vkCmdSetEvent vkCmdResetEvent vkCmdWaitEvents vkCmdPipelineBarrier vkCmdBeginQuery vkCmdEndQuery vkCmdResetQueryPool vkCmdWriteTimestamp vkCmdBeginRenderPass vkCmdNextSubpass vkCmdEndRenderPass vkCmdExecuteCommands vkAllocateCommandBuffers | TODO | NA |
| Vtx Buffer Bounds | Check if VBO index too large for PSO Vtx binding count, and that at least one vertex buffer is attached to pipeline object | VTX_INDEX_OUT_OF_BOUNDS | vkCmdBindDescriptorSets vkCmdBindVertexBuffers | VtxBufferBadIndex | NA |
| Idx Buffer Alignment | Verify that offset of Index buffer falls on an alignment boundary as defined by IdxBufferAlignmentError param | VTX_INDEX_ALIGNMENT_ERROR | vkCmdBindIndexBuffer | IdxBufferAlignmentError | NA |
| Cmd Buffer End | Verifies that EndCommandBuffer was called for this commandBuffer at QueueSubmit time | NO_END_COMMAND_BUFFER | vkQueueSubmit | TODO | NA |
| Cmd Buffer Begin | Check that BeginCommandBuffer was called for this command buffer when binding commands or calling end | NO_BEGIN_COMMAND_BUFFER | vkEndCommandBuffer vkCmdBindPipeline vkCmdSetViewport vkCmdSetLineWidth vkCmdSetDepthBias vkCmdSetBlendConstants vkCmdSetDepthBounds vkCmdSetStencilCompareMask vkCmdSetStencilWriteMask vkCmdSetStencilReference vkCmdBindDescriptorSets vkCmdBindIndexBuffer vkCmdBindVertexBuffers vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect vkCmdDispatch vkCmdDispatchIndirect vkCmdCopyBuffer vkCmdCopyImage vkCmdBlitImage vkCmdCopyBufferToImage vkCmdCopyImageToBuffer vkCmdUpdateBuffer vkCmdFillBuffer vkCmdClearAttachments vkCmdClearColorImage vkCmdClearDepthStencilImage vkCmdResolveImage vkCmdSetEvent vkCmdResetEvent vkCmdWaitEvents vkCmdPipelineBarrier vkCmdBeginQuery vkCmdEndQuery vkCmdResetQueryPool vkCmdWriteTimestamp | NoBeginCommandBuffer | NA |
| Cmd Buffer Submit Count | Verify that ONE_TIME submit cmdbuffer is not submitted multiple times | COMMAND_BUFFER_SINGLE_SUBMIT_VIOLATION | vkBeginCommandBuffer, vkQueueSubmit | CommandBufferTwoSubmits | NA |
| Valid Secondary CommandBuffer | Validates that no primary command buffers are sent to vkCmdExecuteCommands() are | INVALID_SECONDARY_COMMAND_BUFFER | vkCmdExecuteCommands | ExecuteCommandsPrimaryCB | NA |
| Invalid Descriptor Set | Invalid Descriptor Set used. Either never created or already destroyed. | INVALID_DESCRIPTOR_SET | vkQueueSubmit | TODO | Create Test |
| Descriptor Type | Verify Descriptor type in bound descriptor set layout matches descriptor type specified in update. This also includes mismatches in the TYPES of copied descriptors. | DESCRIPTOR_TYPE_MISMATCH | vkUpdateDescriptorSets | DSTypeMismatch CopyDescriptorUpdateErrors | NA |
| Descriptor StageFlags | Verify all descriptors within a single write update have the same stageFlags | DESCRIPTOR_STAGEFLAGS_MISMATCH | vkUpdateDescriptorSets | TODO | Test this case |
| DS Update Size | DS update out of bounds for given layout section. | DESCRIPTOR_UPDATE_OUT_OF_BOUNDS | vkUpdateDescriptorSets | DSUpdateOutOfBounds CopyDescriptorUpdateErrors | NA |
| Descriptor Pool empty | Attempt to allocate descriptor type from descriptor pool when no more of that type are available to be allocated. | DESCRIPTOR_POOL_EMPTY | vkAllocateDescriptorSets | AllocDescriptorFromEmptyPool | NA |
| Free from NON_FREE Pool | It's invalid to call vkFreeDescriptorSets() on Sets that were allocated from a Pool created with NON_FREE usage. | CANT_FREE_FROM_NON_FREE_POOL | vkFreeDescriptorSets | TODO | NA |
| DS Update Index | DS update binding too large for layout binding count. | INVALID_UPDATE_INDEX | vkUpdateDescriptorSets | InvalidDSUpdateIndex CopyDescriptorUpdateErrors | NA |
| DS Update Type | Verifies that structs in DS Update tree are properly created, currently valid, and of the right type | INVALID_UPDATE_STRUCT | vkUpdateDescriptorSets | InvalidDSUpdateStruct | NA |
| MSAA Sample Count | Verifies that Pipeline, RenderPass, and Subpass sample counts are consistent | NUM_SAMPLES_MISMATCH | vkCmdBindPipeline vkCmdBeginRenderPass vkCmdNextSubpass | NumSamplesMismatch | NA |
| Dynamic Viewport State Binding | Verify that viewport dynamic state bound to Cmd Buffer at Draw time | VIEWPORT_NOT_BOUND |vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect | DynamicStatesNotBound | NA |
| Dynamic Scissor State Binding | Verify that scissor dynamic state bound to Cmd Buffer at Draw time | SCISSOR_NOT_BOUND |vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect | DynamicStatesNotBound | NA |
| Dynamic Line Width State Binding | Verify that line width dynamic state bound to Cmd Buffer at draw time when required | LINE_WIDTH_NOT_BOUND |vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect | DynamicStatesNotBound | NA |
| Dynamic Depth Bias State Binding | Verify that depth bias dynamic state bound when depth enabled | DEPTH_BIAS_NOT_BOUND |vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect | DynamicStatesNotBound | NA |
| Dynamic Blend State Binding | Verify that blend dynamic state bound when color blend enabled | BLEND_NOT_BOUND |vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect | DynamicStatesNotBound | NA |
| Dynamic Depth Bounds State Binding | Verify that depth bounds dynamic state bound when depth enabled | DEPTH_BOUNDS_NOT_BOUND |vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect | DynamicStatesNotBound | NA |
| Dynamic Stencil State Binding | Verify that stencil dynamic state bound when depth enabled | STENCIL_NOT_BOUND | vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect | DynamicStatesNotBound | NA |
| RenderPass misuse | Tests for the following: that vkCmdDispatch, vkCmdDispatchIndirect, vkCmdCopyBuffer, vkCmdCopyImage, vkCmdBlitImage, vkCmdCopyBufferToImage, vkCmdCopyImageToBuffer, vkCmdUpdateBuffer, vkCmdFillBuffer, vkCmdClearColorImage, vkCmdClearDepthStencilImage, vkCmdResolveImage, vkCmdSetEvent, vkCmdResetEvent, vkCmdResetQueryPool, vkCmdCopyQueryPoolResults, vkCmdBeginRenderPass, vkEndCommandBuffer are not called during an active Renderpass, and that binding compute descriptor sets or pipelines does not take place during an active Renderpass  | INVALID_RENDERPASS_CMD | vkCmdBindPipeline vkCmdBindDescriptorSets vkCmdDispatch vkCmdDispatchIndirect vkCmdCopyBuffer vkCmdCopyImage vkCmdBlitImage vkCmdCopyBufferToImage vkCmdCopyImageToBuffer vkCmdUpdateBuffer vkCmdFillBuffer vkCmdClearColorImage vkCmdClearDepthStencilImage vkCmdResolveImage vkCmdSetEvent vkCmdResetEvent vkCmdResetQueryPool vkCmdCopyQueryPoolResults vkCmdBeginRenderPass vkEndCommandBuffer | RenderPassWithinRenderPass UpdateBufferWithinRenderPass ClearColorImageWithinRenderPass ClearDepthStencilImageWithinRenderPass FillBufferWithinRenderPass EndCommandBufferWithinRenderPass | NA |
| Correct use of RenderPass | Validates that the following rendering commands are issued inside an active RenderPass: vkCmdDraw, vkCmdDrawIndexed, vkCmdDrawIndirect, vkCmdDrawIndexedIndirect, vkCmdClearAttachments, vkCmdNextSubpass, vkCmdEndRenderPass | NO_ACTIVE_RENDERPASS | vkCmdBindPipeline vkCmdBindDescriptorSets  vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect vkCmdClearAttachments vkCmdNextSubpass vkCmdEndRenderPass | ClearColorAttachmentsOutsideRenderPass | NA |
| Valid RenderPass | Flag error if attempt made to Begin/End/Continue a NULL or otherwise invalid RenderPass object | INVALID_RENDERPASS | vkCmdBeginRenderPass vkCmdEndRenderPass vkBeginCommandBuffer | NullRenderPass | NA |
| RenderPass Compatibility | Verify that active renderpass is compatible with renderpass specified in secondary command buffer, and that renderpass specified for a framebuffer is compatible with renderpass specified in secondary command buffer | RENDERPASS_INCOMPATIBLE | vkCmdExecuteCommands vkBeginCommandBuffer | TODO | None |
| Framebuffer Compatibility | If a framebuffer is passed to secondary command buffer in vkBeginCommandBuffer, then it must match active renderpass (if any) at time of vkCmdExecuteCommands | FRAMEBUFFER_INCOMPATIBLE | vkCmdExecuteCommands | TODO | None |
| DescriptorSet Updated | Warn user if DescriptorSet bound that was never updated and is not empty. Trigger error at draw time if a set being used was never updated. | DESCRIPTOR_SET_NOT_UPDATED | vkCmdBindDescriptorSets vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect | DescriptorSetCompatibility | NA |
| DescriptorSet Bound | Error if DescriptorSet not bound that is used by currently bound VkPipeline at draw time | DESCRIPTOR_SET_NOT_BOUND | vkCmdBindDescriptorSets | DescriptorSetNotUpdated | NA |
| Dynamic Offset Count | Error if dynamicOffsetCount at CmdBindDescriptorSets time is not equal to the actual number of dynamic descriptors in all sets being bound. | INVALID_DYNAMIC_OFFSET_COUNT | vkCmdBindDescriptorSets | InvalidDynamicOffsetCases | None |
| Dynamic Offsets | At draw time, for a *_DYNAMIC type descriptor, the combination of dynamicOffset along with offset and range from its descriptor update must be less than the size of its buffer. | DYNAMIC_OFFSET_OVERFLOW | vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect | InvalidDynamicOffsetCases | None |
| Correct Clear Use | Warn user if CmdClear for Color or DepthStencil issued to Cmd Buffer prior to a Draw Cmd. RenderPass LOAD_OP_CLEAR is preferred in this case. | CLEAR_CMD_BEFORE_DRAW | vkCmdClearColorImage vkCmdClearDepthStencilImage | ClearCmdNoDraw | NA |
| Index Buffer Binding | Verify that an index buffer is bound at the point when an indexed draw is attempted. | INDEX_BUFFER_NOT_BOUND | vkCmdDrawIndexed vkCmdDrawIndexedIndirect | TODO | Implement validation test |
| Viewport and Scissors match | In PSO viewportCount and scissorCount must match. Also for each count that is non-zero, there corresponding data array ptr should be non-NULL. | VIEWPORT_SCISSOR_MISMATCH | vkCreateGraphicsPipelines vkCmdSetViewport vkCmdSetScissor | TODO | Implement validation test |
| Valid Image Aspects for descriptor Updates | When updating ImageView for Descriptor Sets with layout of DEPTH_STENCIL type, the Image Aspect must not have both the DEPTH and STENCIL aspects set, but must have one of the two set. For COLOR_ATTACHMENT, aspect must have COLOR_BIT set. | INVALID_IMAGE_ASPECT | vkUpdateDescriptorSets | DepthStencilImageViewWithColorAspectBitError | This test hits Image layer error, but tough to create case that that skips that error and gets to VK_LAYER_LUNARG_core_validaton draw state error. |
| Valid sampler descriptor Updates | An invalid sampler is used when updating SAMPLER descriptor. | SAMPLER_DESCRIPTOR_ERROR | vkUpdateDescriptorSets | SampleDescriptorUpdateError | Currently only making sure sampler handle is known, can add further validation for sampler parameters |
| Immutable sampler update consistency | Within a single write update, all sampler updates must use either immutable samplers or non-immutable samplers, but not a combination of both. | INCONSISTENT_IMMUTABLE_SAMPLER_UPDATE | vkUpdateDescriptorSets | TODO | Write a test for this case |
| Valid imageView descriptor Updates | An invalid imageView is used when updating *_IMAGE or *_ATTACHMENT descriptor. | IMAGEVIEW_DESCRIPTOR_ERROR | vkUpdateDescriptorSets | ImageViewDescriptorUpdateError | Currently only making sure imageView handle is known, can add further validation for imageView and underlying image parameters |
| Valid bufferView descriptor Updates | An invalid bufferView is used when updating *_TEXEL_BUFFER descriptor. | BUFFERVIEW_DESCRIPTOR_ERROR | vkUpdateDescriptorSets | InvalidBufferViewObject | Currently only making sure bufferView handle is known, can add further validation for bufferView parameters |
| Valid bufferInfo descriptor Updates | An invalid bufferInfo is used when updating *_UNIFORM_BUFFER* or *_STORAGE_BUFFER* descriptor. | BUFFERINFO_DESCRIPTOR_ERROR | vkUpdateDescriptorSets | TODO | Implement validation test |
| Attachment References in Subpass | Attachment reference must be present in active subpass | MISSING_ATTACHMENT_REFERENCE | vkCmdClearAttachments | TODO | Currently only making sure bufferInfo has buffer whose handle is known, can add further validation for bufferInfo parameters |
| Verify Image Layouts | Validate correct image layouts for presents, image transitions, command buffers and renderpasses | INVALID_IMAGE_LAYOUT | vkCreateRenderPass vkMapMemory vkQueuePresentKHR vkQueueSubmit vkCmdCopyImage vkCmdCopyImageToBuffer vkCmdWaitEvents VkCmdPipelineBarrier | InvalidImageLayout MapMemWithoutHostVisibleBit | None |
| Verify Memory Access Flags/Memory Barriers | Validate correct access flags for memory barriers | INVALID_BARRIER | vkCmdWaitEvents vkCmdPipelineBarrier | InvalidBarriers | None |
| Verify Memory Buffer Not Deleted | Validate Command Buffer not submitted with deleted memory buffer | INVALID_BUFFER | vkQueueSubmit | TODO | None |
| Verify Memory Buffer Destroy | Validate memory buffers are not destroyed more than once | DOUBLE_DESTROY | vkDestroyBuffer | TODO | None |
| Verify Object Not In Use | Validate that object being freed or modified is not in use | OBJECT_INUSE | vkDestroyBuffer vkFreeDescriptorSets vkUpdateDescriptorSets | TODO | None |
| Verify Get Queries| Validate that that queries are properly setup, initialized and synchronized | INVALID_QUERY | vkGetFenceStatus vkQueueWaitIdle vkWaitForFences vkDeviceWaitIdle vkCmdBeginQuery vkCmdEndQuery | TODO | None |
| Verify Fences Not In Use | Validate that that fences are not used in multiple submit calls at the same time | INVALID_FENCE | vkQueueSubmit | TODO | None |
| Verify Semaphores Not In Use | Validate that the semaphores are not used in multiple submit calls at the same time | INVALID_SEMAPHORE | vkQueueSubmit | TODO | None |
| Verify Events Not In Use | Validate that that events are not used at the time they are destroyed | INVALID_EVENT | vkDestroyEvent | TODO | None |
| Live Semaphore  | When waiting on a semaphore, need to make sure that the semaphore is live and therefore can be signalled, otherwise queue is stalled and cannot make forward progress. | QUEUE_FORWARD_PROGRESS | vkQueueSubmit vkQueueBindSparse vkQueuePresentKHR vkAcquireNextImageKHR | TODO | Create test |
| Buffer Alignment  | Buffer memory offset in BindBufferMemory must agree with VkMemoryRequirements::alignment returned from a call to vkGetBufferMemoryRequirements with buffer | INVALID_BUFFER_MEMORY_OFFSET | vkBindBufferMemory | TODO | Create test |
| Texel Buffer Alignment  | Storage/Uniform Texel Buffer memory offset in BindBufferMemory must agree with offset alignment device limit | INVALID_TEXEL_BUFFER_OFFSET | vkBindBufferMemory | TODO | Create test |
| Storage Buffer Alignment  | Storage Buffer offsets in BindBufferMemory, BindDescriptorSets must agree with offset alignment device limit | INVALID_STORAGE_BUFFER_OFFSET | vkBindBufferMemory vkCmdBindDescriptorSets | TODO | Create test |
| Uniform Buffer Alignment  | Uniform Buffer offsets in BindBufferMemory, BindDescriptorSets must agree with offset alignment device limit | INVALID_UNIFORM_BUFFER_OFFSET | vkBindBufferMemory vkCmdBindDescriptorSets | TODO | Create test |
| Independent Blending  | If independent blending is not enabled, all elements of pAttachments must be identical | INDEPENDENT_BLEND | vkCreateGraphicsPipelines | TODO | Create test |
| Enabled Logic Operations  | If logic operations is not enabled, logicOpEnable must be VK_FALSE | DISABLED_LOGIC_OP | vkCreateGraphicsPipelines | TODO | Create test |
| Valid Logic Operations  | If logicOpEnable is VK_TRUE, logicOp must be a valid VkLogicOp value | INVALID_LOGIC_OP | vkCreateGraphicsPipelines | TODO | Create test |
| QueueFamilyIndex is Valid | Validates that QueueFamilyIndices are less an the number of QueueFamilies | INVALID_QUEUE_INDEX | vkCmdWaitEvents vkCmdPipelineBarrier vkCreateBuffer vkCreateImage | TODO | Create test |
| Push Constants | Validate that the size of push constant ranges and updates does not exceed maxPushConstantSize | PUSH_CONSTANTS_ERROR | vkCreatePipelineLayout vkCmdPushConstants | TODO | Create test |
| NA | Enum used for informational messages | NONE | | TODO | None |
| NA | Enum used for errors in the layer itself. This does not indicate an app issue, but instead a bug in the layer. | INTERNAL_ERROR | | TODO | None |
| NA | Enum used when VK_LAYER_LUNARG_core_validation attempts to allocate memory for its own internal use and is unable to. | OUT_OF_MEMORY | | TODO | None |
| NA | Enum used when VK_LAYER_LUNARG_core_validation attempts to allocate memory for its own internal use and is unable to. | OUT_OF_MEMORY | | TODO | None |

### VK_LAYER_LUNARG_core_validation Draw State Pending Work

See the Khronos github repository for Vulkan-LoaderAndValidationLayers for additional pending issues, or to submit new validation requests


### VK_LAYER_LUNARG_core_validation Shader Checker Details Table
The Shader Checker portion of the VK_LAYER_LUNARG_core_validation layer inspects the SPIR-V shader images and fixed function pipeline stages at PSO creation time.
It flags errors when inconsistencies are found across interfaces between shader stages. The exact behavior of the checksdepends on the pair of pipeline stages involved.

| Check | Overview | ENUM SHADER_CHECKER_* | Relevant API | Testname | Notes/TODO |
| ----- | -------- | ---------------- | ------------ | -------- | ---------- |
| Not consumed | Flag warning if a location is not consumed (useless work) | OUTPUT_NOT_CONSUMED | vkCreateGraphicsPipelines | CreatePipeline*NotConsumed | NA |
| Not produced | Flag error if a location is not produced (consumer reads garbage) | INPUT_NOT_PRODUCED | vkCreateGraphicsPipelines | CreatePipeline*NotProvided | NA |
| Type mismatch | Flag error if a location has inconsistent types | INTERFACE_TYPE_MISMATCH | vkCreateGraphicsPipelines | CreatePipeline*TypeMismatch | Between shader stages, an exact structural type match is required. Between VI and VS, or between FS and CB, only the basic component type must match (float for UNORM/SNORM/FLOAT, int for SINT, uint for UINT) as the VI and CB stages perform conversions to the exact format. |
| Inconsistent shader | Flag error if an inconsistent SPIR-V image is detected. Possible cases include broken type definitions which the layer fails to walk. | INCONSISTENT_SPIRV | vkCreateGraphicsPipelines | TODO | All current tests use the reference compiler to produce valid SPIRV images from GLSL. |
| Non-SPIRV shader | Flag warning if a non-SPIR-V shader image is detected. This can occur if early drivers are ingesting GLSL. VK_LAYER_LUNARG_ShaderChecker cannot analyze non-SPIRV shaders, so this suppresses most other checks. | NON_SPIRV_SHADER | vkCreateGraphicsPipelines | TODO | NA |
| VI Binding Descriptions | Validate that there is a single vertex input binding description for each binding | INCONSISTENT_VI | vkCreateGraphicsPipelines | CreatePipelineAttribBindingConflict | NA |
| Shader Stage Check | Warns if shader stage is unsupported | UNKNOWN_STAGE | vkCreateGraphicsPipelines | TODO | NA |
| Shader Specialization | Error if specialization entry data is not fully contained within the specialization data block. | BAD_SPECIALIZATION | vkCreateGraphicsPipelines vkCreateComputePipelines | TODO | NA |
| Missing Descriptor | Flags error if shader attempts to use a descriptor binding not declared in the layout | MISSING_DESCRIPTOR | vkCreateGraphicsPipelines | CreatePipelineUniformBlockNotProvided | NA |
| Missing Entrypoint | Flags error if specified entrypoint is not present in the shader module | MISSING_ENTRYPOINT | vkCreateGraphicsPipelines | TODO | NA |
| Push constant out of range | Flags error if a member of a push constant block is not contained within a push constant range specified in the pipeline layout | PUSH_CONSTANT_OUT_OF_RANGE | vkCreateGraphicsPipelines | CreatePipelinePushConstantsNotInLayout | NA |
| Push constant not accessible from stage | Flags error if the push constant range containing a push constant block member is not accessible from the current shader stage. | PUSH_CONSTANT_NOT_ACCESSIBLE_FROM_STAGE | vkCreateGraphicsPipelines | TODO | NA |
| Descriptor not accessible from stage | Flags error if a descriptor used by a shader stage does not include that stage in its stageFlags | DESCRIPTOR_NOT_ACCESSIBLE_FROM_STAGE | vkCreateGraphicsPipelines | TODO | NA |
| Descriptor type mismatch | Flags error if a descriptor type does not match the shader resource type. | DESCRIPTOR_TYPE_MISMATCH | vkCreateGraphicsPipelines | TODO | NA |
| Feature not enabled | Flags error if a capability declared by the shader requires a feature not enabled on the device | FEATURE_NOT_ENABLED | vkCreateGraphicsPipelines | TODO | NA |
| Bad capability | Flags error if a capability declared by the shader is not supported by Vulkan shaders | BAD_CAPABILITY | vkCreateGraphicsPipelines | TODO | NA |
| NA | Enum used for informational messages | NONE | | TODO | None |

### VK_LAYER_LUNARG_core_validation Shader Checker Pending Work
 
See the Khronos github repository for Vulkan-LoaderAndValidationLayers for additional pending issues, or to submit new validation requests

### VK_LAYER_LUNARG_core_validation Memory Tracker Details Table
The Mem Tracker portion of the VK_LAYER_LUNARG_core_validation layer tracks memory objects and references and validates that they are managed correctly by the application.  This includes tracking object bindings, memory hazards, and memory object lifetimes. Several other hazard-related issues related to command buffers, fences, and memory mapping are also validated in this layer segment.

| Check | Overview | ENUM MEMTRACK_* | Relevant API | Testname | Notes/TODO |
| ----- | -------- | ---------------- | ------------ | -------- | ---------- |
| Valid Command Buffer | Verifies that the command buffer was properly created and is currently valid | INVALID_CB | vkCmdBindPipeline vkCmdSetViewport vkCmdSetLineWidth vkCmdSetDepthBias vkCmdSetBlendConstants vkCmdSetDepthBounds vkCmdSetStencilCompareMask vkCmdSetStencilWriteMask vkCmdSetStencilReference vkBeginCommandBuffer vkResetCommandBuffer vkDestroyDevice vkFreeMemory | TODO | NA |
| Valid Memory Object | Verifies that the memory object was properly created and is currently valid | INVALID_MEM_OBJ | vkCmdDrawIndirect vkCmdDrawIndexedIndirect vkCmdDispatchIndirect vkCmdCopyBuffer vkCmdCopyImage vkCmdBlitImage vkCmdCopyBufferToImage vkCmdCopyImageToBuffer vkCmdUpdateBuffer vkCmdFillBuffer vkCmdClearColorImage vkCmdClearDepthStencilImage vkCmdResolveImage vkFreeMemory vkBindBufferMemory vkBindImageMemory vkQueueBindSparse | TODO | NA |
| Memory Aliasing | Flag error if image and/or buffer memory binding ranges overlap | INVALID_ALIASING | vkBindBufferMemory vkBindImageMemory | InvalidMemoryAliasing | Implement test |
| Free Referenced Memory | Checks to see if memory being freed still has current references | FREED_MEM_REF | vmFreeMemory | TODO | NA |
| Valid Object | Verifies that the specified Vulkan object was created properly and is currently valid | INVALID_OBJECT | vkCmdBindPipeline vkCmdDrawIndirect vkCmdDrawIndexedIndirect vkCmdDispatchIndirect vkCmdCopyBuffer vkCmdCopyImage vkCmdBlitImage vkCmdCopyBufferToImage vkCmdCopyImageToBuffer vkCmdUpdateBuffer vkCmdFillBuffer vkCmdClearColorImage vkCmdClearDepthStencilImage vkCmdResolveImage | TODO | NA |
| Objects Not Destroyed | Verify all objects destroyed at DestroyDevice time | MEMORY_LEAK | vkDestroyDevice | TODO | NA |
| Memory Mapping State | Verifies that mapped memory is CPU-visible | INVALID_STATE | vkMapMemory | MapMemWithoutHostVisibleBit | NA |
| Command Buffer Synchronization | Command Buffer must be complete before BeginCommandBuffer or ResetCommandBuffer can be called | RESET_CB_WHILE_IN_FLIGHT | vkBeginCommandBuffer vkResetCommandBuffer | CallBeginCommandBufferBeforeCompletion CallBeginCommandBufferBeforeCompletion | NA |
| Submitted Fence Status | Verifies that: The fence is not submitted in an already signaled state, that ResetFences is not called with a fence in an unsignaled state, and that fences being checked have been submitted | INVALID_FENCE_STATE | vkResetFences vkWaitForFences vkQueueSubmit vkGetFenceStatus | SubmitSignaledFence ResetUnsignaledFence | Create test(s) for case where an unsubmitted fence is having its status checked |
| Immutable Memory Binding | Validates that non-sparse memory bindings are immutable, so objects are not re-boundt | REBIND_OBJECT | vkBindBufferMemory, vkBindImageMemory | RebindMemory | NA |
| Image/Buffer Usage bits | Verify correct USAGE bits set based on how Images and Buffers are used | INVALID_USAGE_FLAG | vkCreateImage, vkCreateBuffer, vkCreateBufferView, vkCmdCopyBuffer, vkCmdCopyQueryPoolResults, vkCmdCopyImage, vkCmdBlitImage, vkCmdCopyBufferToImage, vkCmdCopyImageToBuffer, vkCmdUpdateBuffer, vkCmdFillBuffer  | InvalidUsageBits | NA |
| Memory Map Range Checks | Validates that Memory Mapping Requests are valid for the Memory Object (in-range, not currently mapped on Map, currently mapped on UnMap, size is non-zero) | INVALID_MAP | vkMapMemory | InvalidMemoryMapping | NA |
| NA | Enum used for informational messages | NONE | | TODO | None |
| NA | Enum used for errors in the layer itself. This does not indicate an app issue, but instead a bug in the layer. | INTERNAL_ERROR | | TODO | None |

### VK_LAYER_LUNARG_core_validation Memory Tracker Pending Work and Enhancements

See the Khronos github repository for Vulkan-LoaderAndValidationLayers for additional pending issues, or to submit new validation requests

## VK_LAYER_LUNARG_parameter_validation

### VK_LAYER_LUNARG_parameter_validation Overview

The VK_LAYER_LUNARG_parameter_validation layer validates parameter values and flags errors for any values that are not consistent with the valid usage criteria defined for that parameter.

### VK_LAYER_LUNARG_parameter_validation Details Table

| Check | Overview | ENUM * | Relevant API | Testname | Notes/TODO |
| ----- | -------- | ---------------- | ------------ | -------- | ---------- |
| Valid Usage | Verifies that the value of a parameter is consistent with the valid usage criteria defined in the Vulkan specification | INVALID_USAGE | | TODO | NA |
| Valid VkStructureType Value | Verifies that the sType field of a Vulkan structure contains the value expected for a structure of that type | INVALID_STRUCT_STYPE | | InvalidStructSType | NA |
| Valid Structure pNext Value | Verifies that the pNext field of a Vulkan structure references a value that is compatible with a structure of that type or is NULL when a structure of that type has no compatible pNext values | INVALID_STRUCT_PNEXT | | InvalidStructPNext | NA |
| Required Parameter | Verifies that a required parameter was not specified as 0 or NULL | REQUIRED_PARAMETER | | RequiredParameter | NA |
| Reserved Parameter | Verifies that a parameter reserved for future use was specified as 0 or NULL | RESERVED_PARAMETER | | ReservedParameter | NA |
| Unrecognized Value | Verifies that a Vulkan enumeration, VkFlags, or VkBool32 parameter contains a value that is recognized as valid for that type | UNRECOGNIZED_VALUE | | UnrecognizedValue | NA |
| Failed Call Return Code | Provides a description of a failure code returned by a Vulkan API call | FAILURE_RETURN_CODE | | FailedReturnValue | NA |
| NA | Enum used for informational messages | NONE | | TODO | None |

### VK_LAYER_LUNARG_parameter_validation Pending Work

See the Khronos github repository for Vulkan-LoaderAndValidationLayers for additional pending issues, or to submit new validation requests

## VK_LAYER_LUNARG_image

### VK_LAYER_LUNARG_image Layer Overview

The VK_LAYER_LUNARG_image layer is responsible for validating format-related information and enforcing format restrictions.

### VK_LAYER_LUNARG_image Layer Details Table

DETAILS TABLE PENDING

| Check | Overview | ENUM IMAGE_* | Relevant API | Testname | Notes/TODO |
| ----- | -------- | ---------------- | ------------ | -------- | ---------- |
| Image Format | Verifies that requested format is a supported Vulkan format on this device | FORMAT_UNSUPPORTED | vkCreateImage vkCreateRenderPass | ImageLayerUnsupportedFormat | NA |
| RenderPass Attachments | Validates that attachment image layouts, loadOps, and storeOps are valid Vulkan values | RENDERPASS_INVALID_ATTACHMENT | vkCreateRenderPass | TODO | NA |
| Subpass DS Settings | Verifies that if there is no depth attachment then the subpass attachment is set to VK_ATTACHMENT_UNUSED | RENDERPASS_INVALID_DS_ATTACHMENT | vkCreateRenderPass | TODO | NA |
| View Creation | Verify that requested Image View Creation parameters are reasonable for the image that the view is being created for | VIEW_CREATE_ERROR | vkCreateImageView | ImageLayerViewTests | NA |
| Image Aspects | Verify that Image commands are using valid Image Aspect flags | INVALID_IMAGE_ASPECT | vkCreateImageView vkCmdClearColorImage vkCmdClearDepthStencilImage vkCmdClearAttachments vkCmdCopyImage vkCmdCopyImageToBuffer vkCmdCopyBufferToImage vkCmdResolveImage vkCmdBlitImage | InvalidImageViewAspect | NA |
| Image Aspect Mismatch | Verify that Image commands with source and dest images use matching aspect flags | MISMATCHED_IMAGE_ASPECT | vkCmdCopyImage | MiscImageLayerTests | NA |
| Image Type Mismatch | Verify that Image commands with source and dest images use matching types | MISMATCHED_IMAGE_TYPE | vkCmdCopyImage vkCmdResolveImage | ResolveImageTypeMismatch | NA |
| Image Format Mismatch | Verify that Image commands with source and dest images use matching formats | MISMATCHED_IMAGE_FORMAT | vkCmdCopyImage vkCmdResolveImage | CopyImageDepthStencilFormatMismatch ResolveImageFormatMismatch | NA |
| Resolve Sample Count | Verifies that source and dest images sample counts are valid | INVALID_RESOLVE_SAMPLES | vkCmdResolveImage | ResolveImageHighSampleCount ResolveImageLowSampleCount | NA |
| Verify Format | Verifies the formats are valid for this image operation | INVALID_FORMAT | vkCreateImageView vkCmdBlitImage | ImageLayerViewTests ClearImageErrors | NA |
| Verify Correct Image Filter| Verifies that specified filter is valid | INVALID_FILTER | vkCmdBlitImage | MiscImageLayerTests | NA |
| Verify Correct Image Settings | Verifies that values are valid for a given resource or subresource | INVALID_IMAGE_RESOURCE | vkCmdPipelineBarrier | MiscImageLayerTests | NA |
| Verify Image Format Limits | Verifies that image creation parameters are with the device format limits | INVALID_FORMAT_LIMITS_VIOLATION | vkCreateImage | ImageFormatLimits | NA |
| Verify Layout | Verifies the layouts are valid for this image operation | INVALID_LAYOUT | vkCreateImage vkCmdClearColorImage | TODO | ImageFormatLimits |
| Verify Image Extents | Validates that image extent limits are not invalid | INVALID_EXTENTS | vkCmdCopyImage | CopyImageLayerCountMismatch | NA |
| NA | Enum used for informational messages | NONE | | TODO | None |

### VK_LAYER_LUNARG_image Pending Work

See the Khronos github repository for Vulkan-LoaderAndValidationLayers for additional pending issues, or to submit new validation requests

## VK_LAYER_LUNARG_object_tracker

### VK_LAYER_LUNARG_object_tracker Overview

The VK_LAYER_LUNARG_object_tracker layer maintains a record of all Vulkan objects. It flags errors when invalid objects are used and at DestroyInstance time it flags any objects that were not properly destroyed.

### VK_LAYER_LUNARG_object_tracker Details Table

| Check | Overview | ENUM OBJTRACK_* | Relevant API | Testname | Notes/TODO |
| ----- | -------- | ---------------- | ------------ | -------- | ---------- |
| Valid Object | Validates that referenced object was properly created and is currently valid. | INVALID_OBJECT | vkAcquireNextImageKHR vkAllocateDescriptorSets vkAllocateMemory vkBeginCommandBuffer vkBindBufferMemory vkBindImageMemory vkCmdBeginQuery vkCmdBeginRenderPass vkCmdBindDescriptorSets vkCmdBindIndexBuffer vkCmdBindPipeline vkCmdBindVertexBuffers vkCmdBlitImage vkCmdClearAttachments vkCmdClearColorImage vkCmdClearDepthStencilImage vkCmdCopyBuffer vkCmdCopyBufferToImage vkCmdCopyImage vkCmdCopyImageToBuffer vkCmdCopyQueryPoolResults vkCmdDispatch vkCmdDispatchIndirect vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndexedIndirect vkCmdDrawIndirect vkCmdEndQuery vkCmdEndRenderPass vkCmdExecuteCommands vkCmdFillBuffer vkCmdNextSubpass vkCmdPipelineBarrier vkCmdPushConstants vkCmdResetEvent vkCmdResetQueryPool vkCmdResolveImage vkCmdSetEvent vkCmdUpdateBuffer vkCmdWaitEvents vkCmdWriteTimestamp vkCreateBuffer vkCreateBufferView vkAllocateCommandBuffers vkCreateCommandPool vkCreateComputePipelines vkCreateDescriptorPool vkCreateDescriptorSetLayout vkCreateEvent vkCreateFence vkCreateFramebuffer vkCreateGraphicsPipelines vkCreateImage vkCreateImageView vkCreatePipelineCache vkCreatePipelineLayout vkCreateQueryPool vkCreateRenderPass vkCreateSampler vkCreateSemaphore vkCreateShaderModule vkCreateSwapchainKHR vkDestroyBuffer vkDestroyBufferView vkFreeCommandBuffers vkDestroyCommandPool vkDestroyDescriptorPool vkDestroyDescriptorSetLayout vkDestroyEvent vkDestroyFence vkDestroyFramebuffer vkDestroyImage vkDestroyImageView vkDestroyPipeline vkDestroyPipelineCache vkDestroyPipelineLayout vkDestroyQueryPool vkDestroyRenderPass vkDestroySampler vkDestroySemaphore vkDestroyShaderModule vkDestroySwapchainKHR vkDeviceWaitIdle vkEndCommandBuffer vkEnumeratePhysicalDevices vkFreeDescriptorSets vkFreeMemory vkFreeMemory vkGetBufferMemoryRequirements vkGetDeviceMemoryCommitment vkGetDeviceQueue vkGetEventStatus vkGetFenceStatus vkGetImageMemoryRequirements vkGetImageSparseMemoryRequirements vkGetImageSubresourceLayout vkGetPhysicalDeviceSurfaceSupportKHR vkGetPipelineCacheData vkGetQueryPoolResults vkGetRenderAreaGranularity vkInvalidateMappedMemoryRanges vkMapMemory vkMergePipelineCaches vkQueueBindSparse vkResetCommandBuffer vkResetCommandPool vkResetDescriptorPool vkResetEvent vkResetFences vkSetEvent vkUnmapMemory vkUpdateDescriptorSets vkWaitForFences | BindInvalidMemory BindMemoryToDestroyedObject PipelineNotBound | Every VkObject class of parameter will be run through this check. This check may ultimately supersede UNKNOWN_OBJECT |
| Objects Leak | When an Instance or Device object is destroyed, validates that all objects belonging to that device/instance have previously been destroyed | OBJECT_LEAK | vkDestroyDevice vkDestroyInstance | LeakAnObject | NA |
| Unknown object  | Internal layer errors when it attempts to update use count for an object that's not in its internal tracking datastructures. | UNKNOWN_OBJECT |  | CreateUnknownObject | NA |
| Correct Command Pool | Validates that command buffers in a FreeCommandBuffers call were all created in the specified commandPool | COMMAND_POOL_MISMATCH | vkFreeCommandBuffers | InvalidCommandPoolConsistency | NA |
| Correct Descriptor Pool | Validates that descriptor sets in a FreeDescriptorSets call were all created in the specified descriptorPool | DESCRIPTOR_POOL_MISMATCH | vkFreeDescriptorSets | InvalidDescriptorPoolConsistency | NA |
| NA | Enum used for informational messages | NONE | | TODO | None |
| NA | Enum used for errors in the layer itself. This does not indicate an app issue, but instead a bug in the layer. | INTERNAL_ERROR | | TODO | None |

### VK_LAYER_LUNARG_object_tracker Pending Work

See the Khronos github repository for Vulkan-LoaderAndValidationLayers for additional pending issues, or to submit new validation requests

## VK_LAYER_GOOGLE_threading

### VK_LAYER_GOOGLE_threading Overview

The VK_LAYER_GOOGLE_threading layer checks for simultaneous use of objects by calls from multiple threads.
Application code is responsible for preventing simultaneous use of the same objects by certain calls that modify objects.
See [bug 13433](https://cvs.khronos.org/bugzilla/show_bug.cgi?id=13433) and
<https://cvs.khronos.org/svn/repos/oglc/trunk/nextgen/vulkan/function_properties.csv>
for threading rules.
Objects that may need a mutex include VkQueue, VkDeviceMemory, VkObject, VkBuffer, VkImage, VkDescriptorSet, VkDescriptorPool, VkCommandBuffer, and VkSemaphore.
The most common case is that a VkCommandBuffer passed to VkCmd* calls must be used by only one thread at a time.

In addition to reporting threading rule violations, the layer will enforce a mutex for those calls.
That can allow an application to continue running without actually crashing due to the reported threading problem.

The layer can only observe when a mutual exclusion rule is actually violated.
It cannot insure that there is no latent race condition needing mutual exclusion.

The layer can also catch reentrant use of the same object by calls from a single thread.
That might happen if Vulkan calls are made from a callback function or a signal handler.
But the layer cannot prevent such a reentrant use of an object.

The layer can only observe when a mutual exclusion rule is actually violated.
It cannot insure that there is no latent race condition.

### VK_LAYER_GOOGLE_threading Details Table

| Check | Overview | ENUM THREADING_CHECKER_* | Relevant API | Testname | Notes/TODO |
| ----- | -------- | ---------------- | ---------------- | -------- | ---------- |
| Thread Collision | Detects and notifies user if multiple threads are modifying thes same object | MULTIPLE_THREADS | vkQueueSubmit vkFreeMemory vkMapMemory vkUnmapMemory vkFlushMappedMemoryRanges vkInvalidateMappedMemoryRanges vkBindBufferMemory vkBindImageMemory vkQueueBindSparse vkDestroySemaphore vkDestroyBuffer vkDestroyImage vkDestroyDescriptorPool vkResetDescriptorPool vkAllocateDescriptorSets vkFreeDescriptorSets vkFreeCommandBuffers vkBeginCommandBuffer vkEndCommandBuffer vkResetCommandBuffer vkCmdBindPipeline vkCmdBindDescriptorSets vkCmdBindIndexBuffer vkCmdBindVertexBuffers vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect vkCmdDispatch vkCmdDispatchIndirect vkCmdCopyBuffer vkCmdCopyImage vkCmdBlitImage vkCmdCopyBufferToImage vkCmdCopyImageToBuffer vkCmdUpdateBuffer vkCmdFillBuffer vkCmdClearColorImage vkCmdClearDepthStencilImage vkCmdClearAttachments vkCmdResolveImage vkCmdSetEvent vkCmdResetEvent vkCmdWaitEvents vkCmdPipelineBarrier vkCmdBeginQuery vkCmdEndQuery vkCmdResetQueryPool vkCmdWriteTimestamp vkCmdCopyQueryPoolResults vkCmdBeginRenderPass vkCmdNextSubpass vkCmdPushConstants vkCmdEndRenderPass vkCmdExecuteCommands | TODO | NA |
| Thread Reentrancy | Detects cases of a single thread calling Vulkan reentrantly | SINGLE_THREAD_REUSE | vkQueueSubmit vkFreeMemory vkMapMemory vkUnmapMemory vkFlushMappedMemoryRanges vkInvalidateMappedMemoryRanges vkBindBufferMemory vkBindImageMemory vkQueueBindSparse vkDestroySemaphore vkDestroyBuffer vkDestroyImage vkDestroyDescriptorPool vkResetDescriptorPool vkAllocateDescriptorSets vkFreeDescriptorSets vkFreeCommandBuffers vkBeginCommandBuffer vkEndCommandBuffer vkResetCommandBuffer vkCmdBindPipeline vkCmdSetViewport vkCmdSetBlendConstants vkCmdSetLineWidth vkCmdSetDepthBias vkCmdSetDepthBounds vkCmdSetStencilCompareMask vkCmdSetStencilWriteMask vkCmdSetStencilReference vkCmdBindDescriptorSets vkCmdBindIndexBuffer vkCmdBindVertexBuffers vkCmdDraw vkCmdDrawIndexed vkCmdDrawIndirect vkCmdDrawIndexedIndirect vkCmdDispatch vkCmdDispatchIndirect vkCmdCopyBuffer vkCmdCopyImage vkCmdBlitImage vkCmdCopyBufferToImage vkCmdCopyImageToBuffer vkCmdUpdateBuffer vkCmdFillBuffer vkCmdClearColorImage vkCmdClearDepthStencilImage vkCmdClearAttachments vkCmdResolveImage vkCmdSetEvent vkCmdResetEvent vkCmdWaitEvents vkCmdPipelineBarrier vkCmdBeginQuery vkCmdEndQuery vkCmdResetQueryPool vkCmdWriteTimestamp vkCmdCopyQueryPoolResults vkCmdBeginRenderPass vkCmdNextSubpass vkCmdPushConstants vkCmdEndRenderPass vkCmdExecuteCommands | TODO | NA |
| NA | Enum used for informational messages | NONE | | TODO | None |

### VK_LAYER_GOOGLE_threading Pending Work

See the Khronos github repository for Vulkan-LoaderAndValidationLayers for additional pending issues, or to submit new validation requests

## VK_LAYER_LUNARG_device_limits

### VK_LAYER_LUNARG_device_limits Overview

This layer is a work in progress. VK_LAYER_LUNARG_device_limits layer is intended to capture two broad categories of errors:
 1. Incorrect use of APIs to query device capabilities
 2. Attempt to use API functionality beyond the capability of the underlying device

For the first category, the layer tracks which calls are made and flags errors if calls are excluded that should not be, or if call sequencing is incorrect. An example is an app that assumes attempts to Query and use queues without ever having called vkGetPhysicalDeviceQueueFamilyProperties(). Also, if an app is calling vkGetPhysicalDeviceQueueFamilyProperties() to retrieve properties with some assumed count for array size instead of first calling vkGetPhysicalDeviceQueueFamilyProperties() w/ a NULL pQueueFamilyProperties parameter in order to query the actual count.
For the second category of errors, VK_LAYER_LUNARG_device_limits stores its own internal record of underlying device capabilities and flags errors if requests are made beyond those limits. Most (all?) of the limits are queried via vkGetPhysicalDevice* calls.

### VK_LAYER_LUNARG_device_limits Details Table

| Check | Overview | ENUM DEVLIMITS_* | Relevant API | Testname | Notes/TODO |
| ----- | -------- | ---------------- | ---------------- | -------- | ---------- |
| Valid instance | If an invalid instance is used, this error will be flagged | INVALID_INSTANCE | vkEnumeratePhysicalDevices | TODO | VK_LAYER_LUNARG_object_tracker should also catch this so if we made sure VK_LAYER_LUNARG_object_tracker was always on top, we could avoid this check |
| Valid physical device | Enum used for informational messages | INVALID_PHYSICAL_DEVICE | vkEnumeratePhysicalDevices | TODO | VK_LAYER_LUNARG_object_tracker should also catch this so if we made sure VK_LAYER_LUNARG_object_tracker was always on top, we could avoid this check |
| Valid inherited query | If an invalid inherited query is used, this error will be flagged | INVALID_INHERITED_QUERY | vkBeginCommandBuffer | TODO | None |
| Valid attachment count | If the number of attachments exceeds the device max, this error will be flagged | INVALID_ATTACHMENT_COUNT | vkCreateRenderPass | TODO | None |
| Querying array counts | For API calls where an array count should be queried with an initial call and a NULL array pointer, verify that such a call was made before making a call with non-null array pointer. | MUST_QUERY_COUNT | vkEnumeratePhysicalDevices vkGetPhysicalDeviceQueueFamilyProperties | TODO | Create focused test |
| Array count value | For API calls where an array of details is queried, verify that the size of the requested array matches the size of the array supported by the device. | COUNT_MISMATCH | vkEnumeratePhysicalDevices vkGetPhysicalDeviceQueueFamilyProperties | TODO | Create focused test |
| Queue Creation | When creating/requesting queues, make sure that QueueFamilyPropertiesIndex and index/count within that queue family are valid. | INVALID_QUEUE_CREATE_REQUEST | vkGetDeviceQueue vkCreateDevice | TODO | Create focused test |
| API Call Sequencing | This is a general error indicating that an app did not use vkGetPhysicalDevice* and other such query calls, but rather made an assumption about device capabilities. | INVALID_CALL_SEQUENCE | vkCreateDevice | TODO | Add validation test |
| Feature Request | Attempting to vkCreateDevice with a feature that is not supported by the underlying physical device. | INVALID_FEATURE_REQUESTED | vkCreateDevice | TODO | Add validation test |
| Alignment | When updating a buffer, data should be aligned on 4 byte boundaries  | INVALID_BUFFER_UPDATE_ALIGNMENT | vkCmdUpdateBuffer | UpdateBufferAlignment | NA |
| Alignment | When filling a buffer, data should be aligned on 4 byte boundaries  | INVALID_BUFFER_UPDATE_ALIGNMENT | vkCmdFillBuffer | UpdateBufferAlignment | NA |
| Storage Buffer Alignment  | Storage Buffer offsets must agree with offset alignment device limit | INVALID_STORAGE_BUFFER_OFFSET | vkBindBufferMemory vkUpdateDescriptorSets | TODO | Create test |
| Uniform Buffer Alignment  | Uniform Buffer offsets must agree with offset alignment device limit | INVALID_UNIFORM_BUFFER_OFFSET | vkBindBufferMemory vkUpdateDescriptorSets | TODO | Create test |
| NA | Enum used for informational messages | NONE | | TODO | None |

### VK_LAYER_LUNARG_device_limits Pending Work

See the Khronos github repository for Vulkan-LoaderAndValidationLayers for additional pending issues, or to submit new validation requests

## VK_LAYER_LUNARG_swapchain

### Swapchain Overview

This layer is a work in progress. VK_LAYER_LUNARG_swapchain layer is intended to ...

### VK_LAYER_LUNARG_swapchain Details Table

| Check | Overview | ENUM SWAPCHAIN_* | Relevant API | Testname | Notes/TODO |
| ----- | -------- | ---------------- | ------------ | -------- | ---------- |
| Valid handle | If an invalid handle is used, this error will be flagged | INVALID_HANDLE | vkCreateDevice vkCreateSwapchainKHR | TODO | None |
| Valid pointer | If a NULL pointer is used, this error will be flagged | NULL_POINTER | vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfacePresentModesKHR vkCreateSwapchainKHR vkGetSwapchainImagesKHR vkAcquireNextImageKHR vkQueuePresentKHR | VkWsiEnabledLayerTest.TestEnabledWsi | None |
| Extension enabled before use | Validates that a WSI extension is enabled before its functions are used | EXT_NOT_ENABLED_BUT_USED | vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfacePresentModesKHR vkCreateSwapchainKHR vkDestroySwapchainKHR vkGetSwapchainImagesKHR vkAcquireNextImageKHR vkQueuePresentKHR | VkLayerTest.EnableWsiBeforeUse | None |
| Swapchains destroyed before devices | Validates that  vkDestroySwapchainKHR() is called for all swapchains associated with a device before vkDestroyDevice() is called | DEL_OBJECT_BEFORE_CHILDREN | vkDestroyDevice vkDestroySurfaceKHR | TODO | None |
| Surface seen to support presentation | Validates that pCreateInfo->surface was seen by vkGetPhysicalDeviceSurfaceSupportKHR() to support presentation | CREATE_UNSUPPORTED_SURFACE | vkCreateSwapchainKHR | TODO | None |
| Queries occur before swapchain creation | Validates that vkGetPhysicalDeviceSurfaceCapabilitiesKHR(), vkGetPhysicalDeviceSurfaceFormatsKHR() and vkGetPhysicalDeviceSurfacePresentModesKHR() are called before vkCreateSwapchainKHR() | CREATE_SWAP_WITHOUT_QUERY | vkCreateSwapchainKHR | VkWsiEnabledLayerTest.TestEnabledWsi | None |
| vkCreateSwapchainKHR(pCreateInfo->minImageCount) | Validates vkCreateSwapchainKHR(pCreateInfo->minImageCount) | CREATE_SWAP_BAD_MIN_IMG_COUNT | vkCreateSwapchainKHR | TODO | None |
| vkCreateSwapchainKHR(pCreateInfo->imageExtent) | Validates vkCreateSwapchainKHR(pCreateInfo->imageExtent) when window has no fixed size | CREATE_SWAP_OUT_OF_BOUNDS_EXTENTS | vkCreateSwapchainKHR | TODO | None |
| vkCreateSwapchainKHR(pCreateInfo->imageExtent) | Validates vkCreateSwapchainKHR(pCreateInfo->imageExtent) when window has a fixed size | CREATE_SWAP_EXTENTS_NO_MATCH_WIN | vkCreateSwapchainKHR | TODO | None |
| vkCreateSwapchainKHR(pCreateInfo->preTransform) | Validates vkCreateSwapchainKHR(pCreateInfo->preTransform) | CREATE_SWAP_BAD_PRE_TRANSFORM | vkCreateSwapchainKHR | TODO | None |
| vkCreateSwapchainKHR(pCreateInfo->compositeAlpha) | Validates vkCreateSwapchainKHR(pCreateInfo->compositeAlpha) | CREATE_SWAP_BAD_COMPOSITE_ALPHA | vkCreateSwapchainKHR | TODO | None |
| vkCreateSwapchainKHR(pCreateInfo->imageArraySize) | Validates vkCreateSwapchainKHR(pCreateInfo->imageArraySize) | CREATE_SWAP_BAD_IMG_ARRAY_SIZE | vkCreateSwapchainKHR | TODO | None |
| vkCreateSwapchainKHR(pCreateInfo->imageUsageFlags) | Validates vkCreateSwapchainKHR(pCreateInfo->imageUsageFlags) | CREATE_SWAP_BAD_IMG_USAGE_FLAGS | vkCreateSwapchainKHR | TODO | None |
| vkCreateSwapchainKHR(pCreateInfo->imageColorSpace) | Validates vkCreateSwapchainKHR(pCreateInfo->imageColorSpace) | CREATE_SWAP_BAD_IMG_COLOR_SPACE | vkCreateSwapchainKHR | TODO | None |
| vkCreateSwapchainKHR(pCreateInfo->imageFormat) | Validates vkCreateSwapchainKHR(pCreateInfo->imageFormat) | CREATE_SWAP_BAD_IMG_FORMAT | vkCreateSwapchainKHR | TODO | None |
| vkCreateSwapchainKHR(pCreateInfo->imageFormat and pCreateInfo->imageColorSpace) | Validates vkCreateSwapchainKHR(pCreateInfo->imageFormat and pCreateInfo->imageColorSpace) | CREATE_SWAP_BAD_IMG_FMT_CLR_SP | vkCreateSwapchainKHR | TODO | None |
| vkCreateSwapchainKHR(pCreateInfo->presentMode) | Validates vkCreateSwapchainKHR(pCreateInfo->presentMode) | CREATE_SWAP_BAD_PRESENT_MODE | vkCreateSwapchainKHR | TODO | None |
| vkCreateSwapchainKHR(pCreateInfo->imageSharingMode) | Validates vkCreateSwapchainKHR(pCreateInfo->imageSharingMode) | CREATE_SWAP_BAD_SHARING_MODE | vkCreateSwapchainKHR | VkWsiEnabledLayerTest.TestEnabledWsi | None |
| vkCreateSwapchainKHR(pCreateInfo->imageSharingMode) | Validates vkCreateSwapchainKHR(pCreateInfo->imageSharingMode) | CREATE_SWAP_BAD_SHARING_VALUES | vkCreateSwapchainKHR | VkWsiEnabledLayerTest.TestEnabledWsi | None |
| vkCreateSwapchainKHR(pCreateInfo->oldSwapchain and pCreateInfo->surface) | pCreateInfo->surface must match pCreateInfo->oldSwapchain's surface | CREATE_SWAP_DIFF_SURFACE | vkCreateSwapchainKHR | TODO | None |
| Use same device for swapchain | Validates that vkDestroySwapchainKHR() called with the same VkDevice as vkCreateSwapchainKHR() | DESTROY_SWAP_DIFF_DEVICE | vkCreateSwapchainKHR vkDestroySwapchainKHR | TODO | None |
| Don't acquire too many images | Validates that app never tries to acquire too many swapchain images at a time | APP_ACQUIRES_TOO_MANY_IMAGES | vkAcquireNextImageKHR | TODO | None |
| Index too large | Validates that an image index is within the number of images in a swapchain | INDEX_TOO_LARGE | vkQueuePresentKHR | VkWsiEnabledLayerTest.TestEnabledWsi | None |
| Can't present a non-owned image | Validates that application only presents images that it owns | INDEX_NOT_IN_USE | vkQueuePresentKHR | TODO | None |
| A VkBool32 must have values of VK_TRUE or VK_FALSE | Validates that a VkBool32 must have values of VK_TRUE or VK_FALSE | BAD_BOOL | vkCreateSwapchainKHR | TODO | None |
| pCount must be set by the API before the other pointer is non-NULL | Validates that app queries for the value of pCount before trying to set it | PRIOR_COUNT | vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfacePresentModesKHR vkGetSwapchainImagesKHR | VkWsiEnabledLayerTest.TestEnabledWsi | None |
| pCount must point to same value regardless of whether other pointer is NULL | Validates that app doesn't change value of pCount returned by a query | INVALID_COUNT | vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfacePresentModesKHR vkGetSwapchainImagesKHR | VkWsiEnabledLayerTest.TestEnabledWsi | None |
| Valid sType | Validates that a struct has correct value for sType | WRONG_STYPE | vkCreateSwapchainKHR vkQueuePresentKHR | VkWsiEnabledLayerTest.TestEnabledWsi | None |
| Valid pNext | Validates that a struct has NULL for the value of pNext | WRONG_NEXT | vkCreateSwapchainKHR vkQueuePresentKHR | VkWsiEnabledLayerTest.TestEnabledWsi | None |
| Non-zero value | Validates that a required value should be non-zero | ZERO_VALUE | vkQueuePresentKHR | TODO | None |
| Compatible Allocator | Validates that pAllocator is compatible (i.e. NULL or not) when an object is created and destroyed | INCOMPATIBLE_ALLOCATOR | vkDestroySurfaceKHR | TODO | None |
| Valid use of queueFamilyIndex | Validates that a queueFamilyIndex not used before vkGetPhysicalDeviceQueueFamilyProperties() was called | DID_NOT_QUERY_QUEUE_FAMILIES | vkGetPhysicalDeviceSurfaceSupportKHR | TODO | None |
| Valid queueFamilyIndex value | Validates that a queueFamilyIndex value is less-than pQueueFamilyPropertyCount returned by vkGetPhysicalDeviceQueueFamilyProperties | QUEUE_FAMILY_INDEX_TOO_LARGE | vkGetPhysicalDeviceSurfaceSupportKHR | TODO | None |
| Supported combination of queue and surface | Validates that the surface associated with a swapchain was seen to support the queueFamilyIndex of a given queue | SURFACE_NOT_SUPPORTED_WITH_QUEUE | vkQueuePresentKHR | TODO | None |
| Proper synchronization of acquired images | vkAcquireNextImageKHR should be called with a valid semaphore and/or fence | NO_SYNC_FOR_ACQUIRE | vkAcquireNextImageKHR | TODO | None |

Note: The following platform-specific functions are not mentioned above, because they are protected by ifdefs, which cause test failures:

- vkCreateAndroidSurfaceKHR
- vkCreateMirSurfaceKHR
- vkGetPhysicalDeviceMirPresentationSupportKHR
- vkCreateWaylandSurfaceKHR
- vkGetPhysicalDeviceWaylandPresentationSupportKHR
- vkCreateWin32SurfaceKHR
- vkGetPhysicalDeviceWin32PresentationSupportKHR
- vkCreateXcbSurfaceKHR
- vkGetPhysicalDeviceXcbPresentationSupportKHR
- vkCreateXlibSurfaceKHR
- vkGetPhysicalDeviceXlibPresentationSupportKHR

### VK_LAYER_LUNARG_Swapchain Pending Work

See the Khronos github repository for Vulkan-LoaderAndValidationLayers for additional pending issues, or to submit new validation requests

## VK_LAYER_GOOGLE_unique_objects

### VK_LAYER_GOOGLE_unique_objects Overview

The unique_objects utility layer that assists with validation. The Vulkan specification allows objects to have non-unique handles. This makes tracking object lifetimes difficult in that it is unclear which object is being referenced upon deletion. The unique_objects layer addresses this by aliasing all objects with a unique identifier allowing proper object lifetime tracking. This layer does no validation on its own and may not be required for the proper operation of all layers or all platforms. One sign that it is needed is the appearance of many errors from the object_tracker layer indicating the use of previously destroyed objects. For optimal effectiveness this layer should be loaded last (to reside in the layer chain closest to the display driver and farthest from the application).
