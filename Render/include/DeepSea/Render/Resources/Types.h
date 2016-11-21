/*
 * Copyright 2016 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Thread/Types.h>
#include <DeepSea/Math/Types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types for graphics resources used in the DeepSea/Render library.
 */

/**
 * @brief Constant for mapping the full buffer.
 */
#define DS_MAP_FULL_BUFFER (size_t)-1

/**
 * @brief Flags used as hints for how graphics memory will be used.
 */
typedef enum dsGfxMemory
{
	dsGfxMemory_GpuOnly = 0x01,    ///< The memory will only ever be accessed by the GPU.
	dsGfxMemory_Static = 0x02,     ///< The memory will be modified on the CPU very rarely.
	dsGfxMemory_Dynamic = 0x04,    ///< The memory will be modified on the CPU somewhat often.
	dsGfxMemory_Stream = 0x08,     ///< The memory will be modified on the CPU every time it's used.
	dsGfxMemory_Draw = 0x10,       ///< The memory will be used by draw commands.
	dsGfxMemory_Read = 0x20,       ///< The memory will be read back from the GPU.
	dsGfxMemory_Coherent = 0x40,   ///< The memory should remain coherent to avoid manual flushing.
	dsGfxMemory_Synchronize = 0x80 ///< Wait for the memory to not be in use when mapping.
} dsGfxMemory;

/**
 * @brief Enum for how a graphics buffer will be used.
 *
 * These are bitmask values, allowing a buffer to be used for multiple purposes.
 */
typedef enum dsGfxBufferUsage
{
	dsGfxBufferUsage_Index = 0x001,         ///< Index buffer.
	dsGfxBufferUsage_Vertex  = 0x002,       ///< Vertex buffer.
	dsGfxBufferUsage_Indirect = 0x004,      ///< Indirect draw information.
	dsGfxBufferUsage_UniformBlock = 0x008,  ///< Shader uniform block.
	dsGfxBufferUsage_UniformBuffer = 0x010, ///< Shader uniform buffer, modifiable by the shader.
	dsGfxBufferUsage_Image = 0x020,         ///< Shader image buffer.
	dsGfxBufferUsage_Sampler = 0x040,       ///< Shader sampler buffer.
	dsGfxBufferUsage_CopyFrom = 0x080,      ///< Source for GPU copy operations.
	dsGfxBufferUsage_CopyTo = 0x100         ///< Destination for GPU and CPU copy operations.
} dsGfxBufferUsage;

/**
 * @brief Flags for how to map a graphics buffer to memory.
 */
typedef enum dsGfxBufferMap
{
	dsGfxBufferMap_Read = 0x1,      ///< Read data from the buffer.
	dsGfxBufferMap_Write = 0x2,     ///< Write data to the buffer.
	dsGfxBufferMap_Persistent = 0x4 ///< Allow the buffer to remain locked.
} dsGfxBufferMap;

/**
 * @brief Enum for what kind of mapping is supported on the system.
 *
 * Each level assumes that the features of the previous enum values are also supported.
 */
typedef enum dsGfxBufferMapSupport
{
	dsGfxBufferMapSupport_None,      ///< Mapping of buffers isn't supported.
	dsGfxBufferMapSupport_Full,      ///< May only map the full buffer.
	dsGfxBufferMapSupport_Ranges,    ///< May arbitrary ranges of buffers.
	dsGfxBufferMapSupport_Persistent ///< Buffers may be persistently locked.
} dsGfxBufferMapSupport;

/**
 * @brief Enum for formats used for vertex buffers and images.
 *
 * There are multiple sections of this enum, which are mutually exclusive:
 * - Standard formats. These require a decorator.
 * - Special formats. These may not be used with any decorators.
 * - Compressed formats. These are compressed blocks of memory, and require a decorator.
 * - Decorators. These are OR'd with standard and compressed formats to determine the final format.
 */
typedef enum dsGfxFormat
{
	dsGfxFormat_Unknown = 0, ///< No known format.

	// Standard formats
	dsGfxFormat_R4G4 = 1,            ///< RG 4 bits each.
	dsGfxFormat_R4G4B4A4,            ///< RGBA 4 bits each.
	dsGfxFormat_B4G4R4A4,            ///< BGRA 4 bits each.
	dsGfxFormat_R5G6B5,              ///< RGB with 5, 6, 5 bits.
	dsGfxFormat_B5G6R5,              ///< BGR with 5, 6, 5 bits.
	dsGfxFormat_R5G5B5A1,            ///< RGBA with 5, 5, 5, 1 bits.
	dsGfxFormat_B5G5R5A1,            ///< BGRA with 5, 5, 5, 1 bits.
	dsGfxFormat_A1R5G5B5,            ///< ARGB with 1, 5, 5, 5 bits.
	dsGfxFormat_R8,                  ///< R with 8 bits.
	dsGfxFormat_R8G8,                ///< RG with 8 bits each.
	dsGfxFormat_R8G8B8,              ///< RGB with 8 bits each.
	dsGfxFormat_B8G8R8,              ///< BGR with 8 bits each.
	dsGfxFormat_R8G8B8A8,            ///< RGBA with 8 bits each.
	dsGfxFormat_B8G8R8A8,            ///< BGRA with 8 bits each.
	dsGfxFormat_A8B8G8R8,            ///< ABGR with 8 bits each.
	dsGfxFormat_A2R10G10B10,         ///< ARGB with 2, 10, 10, 10 bits.
	dsGfxFormat_A2B10G10R10,         ///< ABGR with 2, 10, 10, 10 bits.
	dsGfxFormat_R16,                 ///< R with 16 bits.
	dsGfxFormat_R16G16,              ///< RG wtih 16 bits each.
	dsGfxFormat_R16G16B16,           ///< RGB with 16 bits each.
	dsGfxFormat_R16G16B16A16,        ///< RGBA with 16 bits each.
	dsGfxFormat_R32,                 ///< R with 32 bits.
	dsGfxFormat_R32G32,              ///< RG with 32 bits each.
	dsGfxFormat_R32G32B32,           ///< RGB with 32 bits each.
	dsGfxFormat_R32G32B32A32,        ///< RGBA with 32 bits each.
	dsGfxFormat_R64,                 ///< R with 64 bits.
	dsGfxFormat_R64G64,              ///< RG with 64 bits each.
	dsGfxFormat_R64G64B64,           ///< RGB with 64 bits each.
	dsGfxFormat_R64G64B64A64,        ///< RGBA with 64 bits each.
	dsGfxFormat_StandardCount,       ///< The number of standard formats.
	dsGfxFormat_StandardMask = 0xFF, ///< Bitmask for standard formats.

	// Aliases for standard formats.
	dsGfxFormat_X4Y4 = dsGfxFormat_R4G4,                 ///< XY 4 bits each.
	dsGfxFormat_X4Y4Z4W4 = dsGfxFormat_R4G4B4A4,         ///< XYZW with 4 bits each.
	dsGfxFormat_X5Y5Z5W1 = dsGfxFormat_R5G5B5A1,         ///< XYZW with 5, 5, 5, 1 bits.
	dsGfxFormat_X8 = dsGfxFormat_R8,                     ///< X with 8 bits.
	dsGfxFormat_X8Y8 = dsGfxFormat_R8G8,                 ///< XY with 8 bits each.
	dsGfxFormat_X8Y8Z8 = dsGfxFormat_R8G8B8,             ///< XYZ with 8 bits each.
	dsGfxFormat_X8Y8Z8W8 = dsGfxFormat_R8G8B8A8,         ///< XYZW with 8 bits each.
	dsGfxFormat_W2Z10Y10X10 = dsGfxFormat_A2B10G10R10,   ///< WZYX with 2, 10, 10, 10 bits.
	dsGfxFormat_X16 = dsGfxFormat_R16,                   ///< X with 16 bits.
	dsGfxFormat_X16Y16 = dsGfxFormat_R16G16,             ///< XY with 16 bits each.
	dsGfxFormat_X16Y16Z16 = dsGfxFormat_R16G16B16,       ///< XYZ with 16 bits each.
	dsGfxFormat_X16Y16Z16W16 = dsGfxFormat_R16G16B16A16, ///< XYZW with 16 bits each.
	dsGfxFormat_X32 = dsGfxFormat_R32,                   ///< X with 32 bits.
	dsGfxFormat_X32Y32 = dsGfxFormat_R32G32,             ///< XY with 32 bits each.
	dsGfxFormat_X32Y32Z32 = dsGfxFormat_R32G32B32,       ///< XYZ with 32 bits each.
	dsGfxFormat_X32Y32Z32W32 = dsGfxFormat_R32G32B32A32, ///< XYZW with 32 bits each.
	dsGfxFormat_X64 = dsGfxFormat_R64,                   ///< X with 64 bits.
	dsGfxFormat_X64Y64 = dsGfxFormat_R64G64,             ///< XY with 64 bits each.
	dsGfxFormat_X64Y64Z64 = dsGfxFormat_R64G64B64,       ///< XYZ with 64 bits each.
	dsGfxFormat_X64Y64Z64W64 = dsGfxFormat_R64G64B64A64, ///< XYZW with 64 bits each.

	// Special formats.
	dsGfxFormat_B10G11R11_UFloat = 0x100, ///< BGR with 10, 11, 11 bits as unsigned floats.
	dsGfxFormat_E5B9G9R9_UFloat = 0x200,  ///< BGR with 9 bits each as unsigned floats with 5 bits
	                                      ///< shared exponent.
	dsGfxFormat_D16 = 0x300,              ///< Depth with 16 bits.
	dsGfxFormat_X8D24 = 0x400,            ///< Depth with 24 bits and padding.
	dsGfxFormat_D32_Float = 0x500,        ///< Depth with 32 bits as a float.
	dsGfxFormat_S8 = 0x600,               ///< Stencil with 8 bits.
	dsGfxFormat_D16S8 = 0x700,            ///< Depth stencil with 16, 8 bits.
	dsGfxFormat_D24S8 = 0x800,            ///< Depth stencil with 24, 8 bits.
	dsGfxFormat_D32S8_Float = 0x900,      ///< Depth stencil with 32, 8 bits. Depth is float.
	dsGfxFormat_SpecialCount = 0xA,       ///< The number of special formats.
	dsGfxFormat_SpecialMask = 0xF00,      ///< Bitmask for special formats.

	// Aliases for special formats.

	/** ZYX with 10, 11, 11 bits as unsigned floats. */
	dsGfxFormat_Z10Y11X11_UFloat = dsGfxFormat_B10G11R11_UFloat,
	/** ZYX with 9 bits each as unsigned floats with 5 bits shared exponent */
	dsGfxFormat_E5Z9Y9X9_UFloat = dsGfxFormat_E5B9G9R9_UFloat,

	// Compressed formats.
	dsGfxFormat_BC1_RGB = 0x01000,          ///< S3TC BC1 format (DXT1) with RGB.
	dsGfxFormat_BC1_RGBA = 0x02000,         ///< S3TC BC1 format (DXT1) with RGBA with 1 bit alpha.
	dsGfxFormat_BC2 = 0x03000,              ///< S3TC BC2 format (DXT2/3).
	dsGfxFormat_BC3 = 0x04000,              ///< S3TC BC3 format (DXT4/5).
	dsGfxFormat_BC4 = 0x05000,              ///< S3TC BC4 format.
	dsGfxFormat_BC5 = 0x06000,              ///< S3TC BC5 format.
	dsGfxFormat_BC6H = 0x07000,             ///< S3TC BC6H format.
	dsGfxFormat_BC7 = 0x08000,              ///< S3TC BC7 format.
	dsGfxFormat_ETC1 = 0x09000,             ///< ETC1 format.
	dsGfxFormat_ETC2_R8G8B8 = 0x0A000,      ///< ETC2 format with RGB with 8 bits each.
	dsGfxFormat_ETC2_R8G8B8A1 = 0x0B000,    ///< ETC2 format with RGBA with 8, 8, 8, 1 bits.
	dsGfxFormat_ETC2_R8G8B8A8 = 0x0C000,    ///< ETC2 format with RGBA with 8 bits each.
	dsGfxFormat_EAC_R11 = 0x0D000,          ///< EAC format with R with 11 bits.
	dsGfxFormat_EAC_R11G11 = 0x0E000,       ///< EAC format with RG with 11 bits each.
	dsGfxFormat_ASTC_4x4 = 0x0F000,         ///< ASTC with a 4x4 block.
	dsGfxFormat_ASTC_5x4 = 0x10000,         ///< ASTC with a 5x4 block.
	dsGfxFormat_ASTC_5x5 = 0x11000,         ///< ASTC with a 5x5 block.
	dsGfxFormat_ASTC_6x5 = 0x12000,         ///< ASTC with a 6x5 block.
	dsGfxFormat_ASTC_6x6 = 0x13000,         ///< ASTC with a 6x6 block.
	dsGfxFormat_ASTC_8x5 = 0x14000,         ///< ASTC with a 8x5 block.
	dsGfxFormat_ASTC_8x6 = 0x15000,         ///< ASTC with a 8x6 block.
	dsGfxFormat_ASTC_8x8 = 0x16000,         ///< ASTC with a 8x8 block.
	dsGfxFormat_ASTC_10x5 = 0x17000,        ///< ASTC with a 10x5 block.
	dsGfxFormat_ASTC_10x6 = 0x18000,        ///< ASTC with a 10x6 block.
	dsGfxFormat_ASTC_10x8 = 0x19000,        ///< ASTC with a 10x8 block.
	dsGfxFormat_ASTC_10x10 = 0x1A000,       ///< ASTC with a 10x10 block.
	dsGfxFormat_ASTC_12x10 = 0x1B000,       ///< ASTC with a 12x10 block.
	dsGfxFormat_ASTC_12x12 = 0x1C000,       ///< ASTC with a 12x12 block.
	dsGfxFormat_PVRTC1_RGB_2BPP = 0x1D000,  ///< PVRTC1 with RGB with 2 bits per pixel.
	dsGfxFormat_PVRTC1_RGBA_2BPP = 0x1E000, ///< PVRTC1 with RGBA with 2 bits per pixel.
	dsGfxFormat_PVRTC1_RGB_4BPP = 0x1F000,  ///< PVRTC1 with RGB with 4 bits per pixel.
	dsGfxFormat_PVRTC1_RGBA_4BPP = 0x20000, ///< PVRTC1 with RGBA with 4 bits per pixel.
	dsGfxFormat_PVRTC2_RGBA_2BPP = 0x21000, ///< PVRTC2 with RGBA with 2 bits per pixel.
	dsGfxFormat_PVRTC2_RGBA_4BPP = 0x22000, ///< PVRTC2 with RGBA with 4 bits per pixel.
	dsGfxFormat_CompressedCount = 0x23,     ///< The number of compressed formats.
	dsGfxFormat_CompressedMask = 0xFF000,   ///< Bitmask for compressed formats.

	// Decorators
	dsGfxFormat_UNorm = 0x100000,        ///< Integer converted to a float in the range [0, 1].
	dsGfxFormat_SNorm = 0x200000,        ///< Integer converted to a float in the range [-1, 1].
	dsGfxFormat_UScaled = 0x300000,      ///< Unsigned integer converted to a float.
	dsGfxFormat_SScaled = 0x400000,      ///< Signed integer converted to a float.
	dsGfxFormat_UInt = 0x500000,         ///< Unsigned integer.
	dsGfxFormat_SInt = 0x600000,         ///< Signed integer.
	dsGfxFormat_Float = 0x700000,        ///< Signed floating point.
	dsGfxFormat_UFloat = 0x800000,       ///< Unsigned floating point.
	dsGfxFormat_SRGB = 0x900000,         ///< RGB encoded in gamma space.
	dsGfxFormat_DecoratorCount = 0xA,    ///< The number of decorators.
	dsGfxFormat_DecoratorMask = 0xF00000 ///< Bitmask for decorators.
} dsGfxFormat;

/**
 * @brief Enum for the dimension of a texture.
 */
typedef enum dsTextureDim
{
	dsTextureDim_1D,        ///< 1-dimensional
	dsTextureDim_2D,        ///< 2-dimensional
	dsTextureDim_3D,        ///< 3-dimensional
	dsTextureDim_Cube,      ///< Cube map
	dsTextureDim_1DArray,   ///< 1-dimensional array
	dsTextureDim_2DArray,   ///< 2-dimensional array
	dsTextureDim_CubeArray  ///< Cube map array
} dsTextureDim;

/**
 * @brief Struct holding information about a graphics buffer.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsResourceManager and the true internal type.
 */
typedef struct dsGfxBuffer
{
	/**
	 * @brief The usage of type of the buffer.
	 */
	dsGfxBufferUsage usage;

	/**
	 * @brief Memory hints for how the memory will be accessed.
	 */
	dsGfxMemory memoryHints;

	/**
	 * @brief The size of the buffer in bytes.
	 */
	size_t size;
} dsGfxBuffer;

/// \{
typedef struct dsRenderer dsRenderer;
/// \}

/**
 * @brief Manager for graphics resources.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsResourceManager and the true internal type.
 *
 * All manipulation of graphics resources requires a resource context to be created. There will
 * always be a resource context available on the main thread, while other threads require a resource
 * context to be created. Up to maxResourceContexts contexts may be created, which may be 0 for
 * platforms that don't allow multiple threads to access graphics resources.
 */
typedef struct dsResourceManager dsResourceManager;

/**
 * @brief Struct for a resource context.
 *
 * A resource context must be created for each thread that manages resources. The context will be
 * globally bound for that thread when created, then un-bound when finally destroyed. (generally
 * when a thread will exit)
 *
 * This is declared here for internal use, and implementations will provide the final definition.
 */
typedef struct dsResourceContext dsResourceContext;

/**
 * @brief Function for creating a resource context for the current thread.
 * @param resourceManager The resource manager to create the resource context with.
 * @param mainContext True if this is the context for the main thread.
 * @return The created resource context, or NULL if it could not be created.
 */
typedef dsResourceContext* (*dsCreateResourceContextFunction)(dsResourceManager* resourceManager,
	bool mainContext);

/**
 * @brief Function for destroying a resource context.
 * @param resourceManager The resource manager that the context was created with.
 * @param context The resource context to destroy.
 * @return False if the destruction is invalid. errno should be set if the destruction failed.
 */
typedef bool (*dsDestroyResourceContextFunction)(dsResourceManager* resourceManager,
	dsResourceContext* context);

/**
 * @brief Function for creating a graphics buffer.
 * @param resourceManager The resource manager to create the buffer from.
 * @param usage How the buffer will be used. This should be a combination of dsGfxBufferUsage flags.
 * @param memoryHints Hints for how the memory for the buffer will be used. This should be a
 *     combination of dsGfxMemory flags.
 * @param size The size of the buffer.
 * @param data The initial data for the buffer, or NULL to leave uninitialized.
 * @return The created buffer, or NULL if it couldn't be created.
 */
typedef dsGfxBuffer* (*dsCreateGfxBufferFunction)(dsResourceManager* resourceManager, int usage,
	int memoryHints, size_t size, const void* data);

/**
 * @brief Function for destroying a graphics buffer.
 * @param resourceManager The resource manager the buffer was created with.
 * @param buffer The buffer to destroy.
 * @return False if the buffer couldn't be destroyed.
 */
typedef bool (*dsDestroyGfxBufferFunction)(dsResourceManager* resourceManager, dsGfxBuffer* buffer);

/**
 * @brief Function for mapping a range of a buffer to memory.
 * @param resourceManager The resource manager that the buffer was created with.
 * @param buffer The buffer to map.
 * @param flags The flags describing how to map the memory. This should be a combination of
 *     dsGfxBufferMap flags
 * @param offset The offset into the buffer to map. This must be aligned with minMappingAlignment
 *     from dsResourceManager.
 * @param size The number of bytes to map. This may be set to DS_MAP_FULL_BUFFER to map from the
 *     offset to the end of the buffer.
 * @return A pointer to the mapped memory or NULL if the memory couldn't be mapped.
 */
typedef void* (*dsMapGfxBufferFunction)(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	int flags, size_t offset, size_t size);

/**
 * @brief Function for unmapping previously mapped memory from a buffer.
 * @param resourceManager The resource manager that the buffer was created with.
 * @param buffer The buffer to unmap.
 * @return False if the memory couldn't be unmapped.
 */
typedef bool (*dsUnmapGfxBufferFunction)(dsResourceManager* resourceManager, dsGfxBuffer* buffer);

/**
 * @brief Function for flushing writes to a mapped memory range for a buffer.
 *
 * This is generally used for persistently mapped memory for a non-coherent buffer. This guarantees
 * writes from the CPU will be visible from the GPU.
 *
 * @param resourceManager The resource manager that the buffer was created with.
 * @param buffer The buffer to flush.
 * @param offset The offset of the range to flush.
 * @param size The size of the memory to flush.
 * @return False if the memory couldn't be flushed.
 */
typedef bool (*dsFlushGfxBufferFunction)(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size);

/**
 * @brief Function for invalidating reads to a mapped memory range for a buffer.
 *
 * This is generally used for persistently mapped memory for a non-coherent buffer. This guarantees
 * writes from the GPU will be visible from the CPU.
 *
 * @param resourceManager The resource manager that the buffer was created with.
 * @param buffer The buffer to invalidate.
 * @param offset The offset of the range to invalidate.
 * @param size The size of the memory to invalidate.
 * @return False if the memory couldn't be invalidated.
 */
typedef bool (*dsInvalidateGfxBufferFunction)(dsResourceManager* resourceManager,
	dsGfxBuffer* buffer, size_t offset, size_t size);

/**
 * @brief Function for copying data to a buffer.
 * @param resourceManager The resource manager that the buffer was created with.
 * @param buffer The buffer to copy the data to.
 * @param size The size of the data to copy.
 * @param data The data to copy to the buffer.
 * @return False if the data couldn't be copied.
 */
typedef bool (*dsCopyGfxBufferData)(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size, const void* data);

/** @copydoc dsResourceManager */
struct dsResourceManager
{
	// Public members

	/**
	 * The renderer this belongs to.
	 */
	dsRenderer* renderer;

	/**
	 * @brief The number of resource contexts that may be created for other threads.
	 */
	unsigned int maxResourceContexts;

	/**
	 * @brief The minimum alignment when mapping the range of a buffer.
	 */
	unsigned int minMappingAlignment;

	/**
	 * @brief Bitmask for the supported buffer types.
	 */
	dsGfxBufferUsage supportedBuffers;

	/**
	 * @brief Enum describing how buffers may be mapped.
	 */
	dsGfxBufferMapSupport bufferMapSupport;

	// Private members

	/**
	 * Current thread's resource context.
	 */
	dsThreadStorage _resourceContext;

	// Virtual function table

	/**
	 * @brief Resource context creation function.
	 */
	dsCreateResourceContextFunction createResourceContextFunc;

	/**
	 * @brief Resource context destruction function.
	 */
	dsDestroyResourceContextFunction destroyResourceContextFunc;

	/**
	 * @brief Buffer creation function.
	 */
	dsCreateGfxBufferFunction createBufferFunc;

	/**
	 * @brief Buffer destruction function.
	 */
	dsDestroyGfxBufferFunction destroyBufferFunc;

	/**
	 * @brief Buffer mapping function.
	 */
	dsMapGfxBufferFunction mapBufferFunc;

	/**
	 * @brief Buffer unmapping function.
	 */
	dsUnmapGfxBufferFunction unmapBufferFunc;

	/**
	 * @brief Buffer flushing function.
	 */
	dsFlushGfxBufferFunction flushBufferFunc;

	/**
	 * @brief Buffer invalidating function.
	 */
	dsInvalidateGfxBufferFunction invalidateBufferFunc;

	/**
	 * @brief Buffer data copying function.
	 */
	dsCopyGfxBufferData copyBufferDataFunc;
};

#ifdef __cplusplus
}
#endif