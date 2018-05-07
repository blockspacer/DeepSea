/*
 * Copyright 2017 Aaron Barany
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

#include "GLMainCommandBuffer.h"

#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "Resources/GLDrawGeometry.h"
#include "Resources/GLFramebuffer.h"
#include "Resources/GLGfxFence.h"
#include "Resources/GLResourceManager.h"
#include "Resources/GLTexture.h"
#include "GLCommandBuffer.h"
#include "GLHelpers.h"
#include "GLRendererInternal.h"
#include "GLRenderStates.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <limits.h>
#include <string.h>

#define DS_TEMP_RENDERBUFFERS 4

typedef struct TempRenderbuffer
{
	GLuint id;
	uint32_t width;
	uint32_t height;
	uint32_t samples;
	uint32_t lruCounter;
} TempRenderbuffer;

struct dsGLMainCommandBuffer
{
	dsGLCommandBuffer commandBuffer;

	dsGLFenceSyncRef** fenceSyncs;
	uint32_t curFenceSyncs;
	uint32_t maxFenceSyncs;
	bool bufferReadback;

	const dsFramebuffer* curFramebuffer;
	dsSurfaceClearValue* clearValues;
	size_t curClearValues;
	size_t maxClearValues;

	TempRenderbuffer tempRenderbuffers[DS_TEMP_RENDERBUFFERS];
	uint32_t tempRenderbufferCounter;

	const dsDrawGeometry* curGeometry;
	const dsGfxBuffer* curDrawIndirectBuffer;
	const dsGfxBuffer* curDispatchIndirectBuffer;
	int32_t curBaseVertex;

	GLuint currentProgram;

	mslRenderState currentState;
	GLuint defaultSamplers[2];
	mslSamplerState defaultSamplerState;
};

static const GLenum primitiveTypeMap[] =
{
	GL_POINTS,
	GL_LINES,
	GL_LINE_STRIP,
	GL_TRIANGLES,
	GL_TRIANGLE_STRIP,
	GL_TRIANGLE_FAN,
	GL_LINES_ADJACENCY,
	GL_TRIANGLES_ADJACENCY,
	GL_TRIANGLE_STRIP_ADJACENCY,
	GL_PATCHES
};

static bool setFences(dsRenderer* renderer, dsGLFenceSyncRef** fenceSyncs, size_t fenceCount,
	bool bufferReadback)
{
	if (ANYGL_SUPPORTED(glMemoryBarrier) && bufferReadback)
		glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);

	GLsync glSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	if (!glSync)
	{
		GLenum lastError = dsGetLastGLError();
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error setting fence: %s",
			AnyGL_errorString(lastError));
		errno = dsGetGLErrno(lastError);
	}
	glFlush();

	dsGLFenceSync* sync = dsGLRenderer_createSync(renderer, glSync);
	if (!sync)
	{
		glDeleteSync(glSync);
		return false;
	}

	for (size_t i = 0; i < fenceCount; ++i)
	{
		dsGLFenceSync_addRef(sync);
		DS_ASSERT(!fenceSyncs[i]->sync);
		DS_ATOMIC_STORE_PTR(&fenceSyncs[i]->sync, &sync);
	}

	dsGLFenceSync_freeRef(sync);
	return true;
}

static void updateSamplers(const dsRenderer* renderer, const dsGLShader* shader)
{
	if (AnyGL_EXT_texture_filter_anisotropic &&
		renderer->defaultAnisotropy != shader->defaultAnisotropy)
	{
		for (uint32_t i = 0; i < shader->pipeline.samplerStateCount; ++i)
		{
			if (shader->samplerStates[i].maxAnisotropy == MSL_UNKNOWN_FLOAT)
			{
				glSamplerParameterf(shader->samplerIds[i], GL_TEXTURE_MAX_ANISOTROPY_EXT,
					renderer->defaultAnisotropy);
			}
		}
		((dsGLShader*)shader)->defaultAnisotropy = renderer->defaultAnisotropy;
	}
}

static GLenum getClearMask(dsGfxFormat format)
{
	switch (format)
	{
		case dsGfxFormat_D16:
		case dsGfxFormat_X8D24:
			return GL_DEPTH_BUFFER_BIT;
		case dsGfxFormat_S8:
			return GL_STENCIL_BUFFER_BIT;
		case dsGfxFormat_D16S8:
		case dsGfxFormat_D24S8:
		case dsGfxFormat_D32S8_Float:
			return GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
		default:
			return GL_COLOR_BUFFER_BIT;
	}
}

static void setClearColor(dsGfxFormat format, const dsSurfaceClearValue* value)
{
	switch (format)
	{
		case dsGfxFormat_D16:
		case dsGfxFormat_X8D24:
		case dsGfxFormat_S8:
		case dsGfxFormat_D16S8:
		case dsGfxFormat_D24S8:
		case dsGfxFormat_D32S8_Float:
			if (ANYGL_SUPPORTED(glClearDepthf))
				glClearDepthf(value->depthStencil.depth);
			else
				glClearDepth(value->depthStencil.depth);
			glClearStencil(value->depthStencil.stencil);
			break;
		default:
			DS_ASSERT((format & dsGfxFormat_DecoratorMask) != dsGfxFormat_UInt);
			DS_ASSERT((format & dsGfxFormat_DecoratorMask) != dsGfxFormat_SInt);
			glClearColor(value->colorValue.floatValue.r, value->colorValue.floatValue.g,
				value->colorValue.floatValue.b, value->colorValue.floatValue.a);
			break;
	}
}

static void clearDrawBuffer(dsGfxFormat format, uint32_t colorIndex,
	const dsSurfaceClearValue* clearValue)
{
	switch (format)
	{
		case dsGfxFormat_D16:
		case dsGfxFormat_X8D24:
			glClearBufferfv(GL_DEPTH, 0, &clearValue->depthStencil.depth);
			break;
		case dsGfxFormat_S8:
			glClearBufferiv(GL_STENCIL, 0, (GLint*)&clearValue->depthStencil.stencil);
			break;
		case dsGfxFormat_D16S8:
		case dsGfxFormat_D24S8:
		case dsGfxFormat_D32S8_Float:
			glClearBufferfi(GL_DEPTH_STENCIL, 0, clearValue->depthStencil.depth,
				clearValue->depthStencil.stencil);
			break;
		default:
		{
			dsGfxFormat decorator = (dsGfxFormat)(format & dsGfxFormat_DecoratorMask);
			if (decorator == dsGfxFormat_UInt)
				glClearBufferuiv(GL_COLOR, colorIndex, clearValue->colorValue.uintValue);
			else if (decorator == dsGfxFormat_SInt)
				glClearBufferiv(GL_COLOR, colorIndex, clearValue->colorValue.intValue);
			else
				glClearBufferfv(GL_COLOR, colorIndex, clearValue->colorValue.floatValue.values);
			break;
		}
	}
}

static void clearDrawBufferPart(GLenum buffer, dsGfxFormat format, uint32_t colorIndex,
	const dsSurfaceClearValue* clearValue)
{
	switch (buffer)
	{
		case GL_DEPTH:
			glClearBufferfv(GL_DEPTH, 0, &clearValue->depthStencil.depth);
			break;
		case GL_STENCIL:
			glClearBufferiv(GL_STENCIL, 0, (GLint*)&clearValue->depthStencil.stencil);
			break;
		case GL_DEPTH_STENCIL:
			glClearBufferfi(GL_DEPTH_STENCIL, 0, clearValue->depthStencil.depth,
				clearValue->depthStencil.stencil);
			break;
		case GL_COLOR:
		{
			dsGfxFormat decorator = (dsGfxFormat)(format & dsGfxFormat_DecoratorMask);
			if (decorator == dsGfxFormat_UInt)
				glClearBufferuiv(GL_COLOR, colorIndex, clearValue->colorValue.uintValue);
			else if (decorator == dsGfxFormat_SInt)
				glClearBufferiv(GL_COLOR, colorIndex, clearValue->colorValue.intValue);
			else
				glClearBufferfv(GL_COLOR, colorIndex, clearValue->colorValue.floatValue.values);
			break;
		}
		default:
			DS_ASSERT(false);
			return;
	}
}

static void clearOtherFramebuffer(const dsRenderPass* renderPass, uint32_t subpassIndex,
	const dsSurfaceClearValue* clearValues)
{
	const dsGLRenderPass* glRenderPass = (dsGLRenderPass*)renderPass;
	const dsRenderSubpassInfo* subpass = renderPass->subpasses + subpassIndex;
	for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
	{
		uint32_t attachment = subpass->colorAttachments[i].attachmentIndex;
		if (attachment != DS_NO_ATTACHMENT &&
			glRenderPass->clearSubpass[attachment] == subpassIndex)
		{
			clearDrawBuffer(renderPass->attachments[attachment].format, i,
				clearValues + attachment);
		}
	}

	if (subpass->depthStencilAttachment != DS_NO_ATTACHMENT)
	{
		uint32_t attachment = subpass->depthStencilAttachment;
		if (glRenderPass->clearSubpass[attachment] == subpassIndex)
		{
			clearDrawBuffer(renderPass->attachments[attachment].format, 0,
				clearValues + attachment);
		}
	}
}

static void clearMainFramebuffer(const dsRenderPass* renderPass, uint32_t subpassIndex,
	const dsSurfaceClearValue* clearValues)
{
	GLenum clearMask = 0;
	const dsGLRenderPass* glRenderPass = (dsGLRenderPass*)renderPass;
	const dsRenderSubpassInfo* subpass = renderPass->subpasses + subpassIndex;
	for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
	{
		uint32_t attachment = subpass->colorAttachments[i].attachmentIndex;
		if (attachment == DS_NO_ATTACHMENT ||
			glRenderPass->clearSubpass[attachment] != subpassIndex)
		{
			continue;
		}

		clearMask |= getClearMask(renderPass->attachments[attachment].format);
		setClearColor(renderPass->attachments[attachment].format, clearValues + attachment);
	}

	if (subpass->depthStencilAttachment != DS_NO_ATTACHMENT)
	{
		uint32_t attachment = subpass->depthStencilAttachment;
		if (glRenderPass->clearSubpass[attachment] == subpassIndex)
		{
			clearMask |= getClearMask(renderPass->attachments[attachment].format);
			setClearColor(renderPass->attachments[attachment].format, clearValues + attachment);
		}
	}

	if (clearMask)
		glClear(clearMask);
}

static bool beginRenderSubpass(dsGLMainCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t subpassIndex)
{
	// Bind the framebuffer with the surfaces for this subpass.
	const dsRenderSubpassInfo* subpass = renderPass->subpasses + subpassIndex;
	GLSurfaceType surfaceType = dsGLFramebuffer_bind(commandBuffer->curFramebuffer,
		subpass->colorAttachments, subpass->colorAttachmentCount, subpass->depthStencilAttachment);
	if (surfaceType == GLSurfaceType_None)
		return false;

	// Clear the buffers for this framebuffer.
	if (commandBuffer->curClearValues > 0)
	{
		DS_ASSERT(commandBuffer->curClearValues == renderPass->attachmentCount);
		if (surfaceType == GLSurfaceType_Framebuffer && ANYGL_SUPPORTED(glClearBufferfv))
			clearOtherFramebuffer(renderPass, subpassIndex, commandBuffer->clearValues);
		else
			clearMainFramebuffer(renderPass, subpassIndex, commandBuffer->clearValues);
	}

	return true;
}

static bool endRenderSubpass(dsGLMainCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t subpassIndex)
{
	// Resolve any targets that are set to resolve.
	GLuint readFbo = 0;
	GLuint writeFbo = 0;
	dsTexture* texture = NULL;
	dsRenderer* renderer = ((dsCommandBuffer*)commandBuffer)->renderer;
	const dsRenderSubpassInfo* subpass = renderPass->subpasses + subpassIndex;
	const dsFramebuffer* framebuffer = commandBuffer->curFramebuffer;
	for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
	{
		uint32_t attachment = subpass->colorAttachments[i].attachmentIndex;
		if (attachment == DS_NO_ATTACHMENT || !subpass->colorAttachments[i].resolve)
			continue;

		if (framebuffer->surfaces[attachment].surfaceType != dsGfxSurfaceType_Texture)
			continue;

		texture = (dsTexture*)framebuffer->surfaces[attachment].surface;
		dsGLTexture* glTexture = (dsGLTexture*)texture;
		if (!glTexture->drawBufferId)
			continue;

		if (!readFbo)
		{
			readFbo = dsGLRenderer_tempFramebuffer(renderer);
			writeFbo = dsGLRenderer_tempCopyFramebuffer(renderer);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, writeFbo);
		}

		GLenum buffers = dsGLTexture_attachment(texture->info.format);
		GLbitfield bufferMask = dsGLTexture_buffers(texture->info.format);
		glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, buffers, GL_RENDERBUFFER,
			glTexture->drawBufferId);
		dsGLTexture_bindFramebufferTextureAttachment(texture, GL_DRAW_FRAMEBUFFER, buffers,
			framebuffer->surfaces[attachment].mipLevel, framebuffer->surfaces[attachment].layer);

		glBlitFramebuffer(0, 0, texture->info.width, texture->info.height, 0, 0,
			texture->info.width, texture->info.height, bufferMask, GL_NEAREST);
	}

	if (readFbo)
	{
		DS_ASSERT(writeFbo);
		DS_ASSERT(texture);
		dsGLTexture_unbindFramebuffer(texture, GL_DRAW_FRAMEBUFFER);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		dsGLRenderer_restoreFramebuffer(renderer);
	}

	return true;
}

static GLuint createTempRenderbuffer(dsGLMainCommandBuffer* commandBuffer, uint32_t width,
	uint32_t height, uint32_t samples)
{
	TempRenderbuffer* renderbuffers = commandBuffer->tempRenderbuffers;
	unsigned int index = 0;
	unsigned int prevCount = UINT_MAX;
	for (unsigned int i = 0; i < DS_TEMP_RENDERBUFFERS; ++i)
	{
		if (!renderbuffers[i].id)
		{
			index = i;
			prevCount = 0;
			continue;
		}

		if (renderbuffers[i].width == width && renderbuffers[i].height == height &&
			renderbuffers[i].samples == samples)
		{
			renderbuffers[i].lruCounter = commandBuffer->tempRenderbufferCounter++;
			return renderbuffers[i].id;
		}

		if (renderbuffers[i].lruCounter < prevCount)
		{
			index = i;
			prevCount = renderbuffers[i].lruCounter;
		}
	}

	glGenRenderbuffers(1, &renderbuffers[index].id);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffers[index].id);
	if (samples > 1)
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA, width, height);
	else
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	renderbuffers[index].width = width;
	renderbuffers[index].height = height;
	renderbuffers[index].samples = samples;
	renderbuffers[index].width = width;
	renderbuffers[index].lruCounter = commandBuffer->tempRenderbufferCounter++;
	return renderbuffers[index].id;
}

static dsGfxFormat getSurfaceFormat(dsRenderer* renderer, dsGfxSurfaceType surfaceType, void* surface)
{
	switch (surfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurface:
		case dsGfxSurfaceType_ColorRenderSurfaceLeft:
		case dsGfxSurfaceType_ColorRenderSurfaceRight:
			return renderer->surfaceColorFormat;
		case dsGfxSurfaceType_DepthRenderSurface:
		case dsGfxSurfaceType_DepthRenderSurfaceLeft:
		case dsGfxSurfaceType_DepthRenderSurfaceRight:
			return renderer->surfaceDepthStencilFormat;
		case dsGfxSurfaceType_Texture:
			return ((dsTexture*)surface)->info.format;
		case dsGfxSurfaceType_Renderbuffer:
			return ((dsRenderbuffer*)surface)->format;
		default:
			DS_ASSERT(false);
			return dsGfxFormat_Unknown;
	}
}

static void getSurfaceInfo(uint32_t* outWidth, uint32_t* outHeight, uint32_t* outFaces,
	bool* outInvertY, dsGfxSurfaceType surfaceType, void* surface)
{
	switch (surfaceType)
	{;
		case dsGfxSurfaceType_Texture:
		{
			dsTexture* texture = (dsTexture*)surface;
			*outWidth = texture->info.width;
			*outHeight = texture->info.height;
			*outFaces = texture->info.dimension == dsTextureDim_Cube ? 6 : 1;
			*outInvertY = false;
			break;
		}
		case dsGfxSurfaceType_Renderbuffer:
		{
			dsRenderbuffer* renderbuffer = (dsRenderbuffer*)surface;
			*outWidth = renderbuffer->width;
			*outHeight = renderbuffer->height;
			*outFaces = 1;
			*outInvertY = false;
			break;
		}
		default:
		{
			dsRenderSurface* renderSurface = (dsRenderSurface*)surface;
			*outWidth = renderSurface->width;
			*outHeight = renderSurface->height;
			*outFaces = 1;
			*outInvertY = true;
			break;
		}
	}
}

static void bindBlitSurface(GLenum framebufferType, dsGfxSurfaceType surfaceType, void* surface,
	uint32_t mipLevel, uint32_t layer)
{
	switch (surfaceType)
	{
		case dsGfxSurfaceType_Texture:
			dsGLTexture_bindFramebufferTexture((dsTexture*)surface, framebufferType, mipLevel,
				layer);
			break;
		case dsGfxSurfaceType_Renderbuffer:
		{
			dsGLRenderbuffer* renderbuffer = (dsGLRenderbuffer*)surface;
			glFramebufferRenderbuffer(framebufferType,
				dsGLTexture_attachment(renderbuffer->renderbuffer.format), GL_RENDERBUFFER,
				renderbuffer->renderbufferId);
			break;
		}
		default:
			break;
	}
}

static void unbindBlitSurface(GLenum framebufferType, dsGfxSurfaceType surfaceType, void* surface)
{
	switch (surfaceType)
	{
		case dsGfxSurfaceType_Texture:
			dsGLTexture_unbindFramebuffer((dsTexture*)surface, framebufferType);
			break;
		case dsGfxSurfaceType_Renderbuffer:
		{
			dsRenderbuffer* renderbuffer = (dsRenderbuffer*)surface;
			glFramebufferRenderbuffer(framebufferType, dsGLTexture_attachment(renderbuffer->format),
				GL_RENDERBUFFER, 0);
			break;
		}
		default:
			break;
	}
}

bool dsGLMainCommandBuffer_copyBufferData(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size)
{
	DS_UNUSED(commandBuffer);

	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	GLenum bufferType = dsGetGLBufferType(buffer->usage);
	glBindBuffer(bufferType, glBuffer->bufferId);
	glBufferSubData(bufferType, offset, size, data);
	glBindBuffer(bufferType, 0);
	return true;
}

bool dsGLMainCommandBuffer_copyBuffer(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size)
{
	DS_UNUSED(commandBuffer);

	dsGLGfxBuffer* glSrcBuffer = (dsGLGfxBuffer*)srcBuffer;
	dsGLGfxBuffer* glDstBuffer = (dsGLGfxBuffer*)dstBuffer;
	glCopyBufferSubData(glSrcBuffer->bufferId, glDstBuffer->bufferId, srcOffset, dstOffset, size);
	return true;
}

bool dsGLMainCommandBuffer_copyTextureData(dsCommandBuffer* commandBuffer, dsTexture* texture,
	const dsTexturePosition* position, uint32_t width, uint32_t height, uint32_t layers,
	const void* data, size_t size)
{
	DS_UNUSED(commandBuffer);

	dsGLTexture* glTexture = (dsGLTexture*)texture;
	GLenum target = dsGLTexture_target(texture);

	bool compressed = dsGfxFormat_compressedIndex(texture->info.format) > 0;
	GLenum internalFormat;
	GLenum glFormat;
	GLenum type;
	DS_VERIFY(dsGLResourceManager_getTextureFormatInfo(&internalFormat, &glFormat, &type,
		texture->resourceManager, texture->info.format));

	dsGLRenderer_beginTextureOp(commandBuffer->renderer, target, glTexture->textureId);
	switch (texture->info.dimension)
	{
		case dsTextureDim_1D:
			if (texture->info.depth > 0)
			{
				if (compressed)
				{
					glCompressedTexSubImage2D(GL_TEXTURE_1D_ARRAY, position->mipLevel, position->x,
						position->depth, width, layers, internalFormat, (GLsizei)size, data);
				}
				else
				{
					glTexSubImage2D(GL_TEXTURE_1D_ARRAY, position->mipLevel, position->x,
						position->depth, width, layers, glFormat, type, data);
				}
			}
			else
			{
				if (compressed)
				{
					glCompressedTexSubImage1D(GL_TEXTURE_1D, position->mipLevel, position->x,
						width, internalFormat, (GLsizei)size, data);
				}
				else
				{
					glTexSubImage1D(GL_TEXTURE_1D, position->mipLevel, position->x, width, glFormat,
						type, data);
				}
			}
			break;
		case dsTextureDim_2D:
			if (texture->info.depth > 0)
			{
				if (compressed)
				{
					glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, position->mipLevel, position->x,
						position->y, position->depth, width, height, layers, internalFormat,
						(GLsizei)size, data);
				}
				else
				{
					glTexSubImage3D(GL_TEXTURE_2D_ARRAY, position->mipLevel, position->x,
						position->y, position->depth, width, height, layers, glFormat, type, data);
				}
			}
			else
			{
				if (compressed)
				{
					glCompressedTexSubImage2D(GL_TEXTURE_2D, position->mipLevel, position->x,
						position->y, width, height, internalFormat, (GLsizei)size, data);
				}
				else
				{
					glTexSubImage2D(GL_TEXTURE_2D, position->mipLevel, position->x, position->y,
						width, height, glFormat, type, data);
				}
			}
			break;
		case dsTextureDim_3D:
			if (compressed)
			{
				glCompressedTexSubImage3D(GL_TEXTURE_3D, position->mipLevel, position->x,
					position->y, position->depth, width, height, layers, internalFormat,
					(GLsizei)size, data);
			}
			else
			{
				glTexSubImage3D(GL_TEXTURE_3D, position->mipLevel, position->x, position->y,
					position->depth, width, height, layers, glFormat, type, data);
			}
			break;
		case dsTextureDim_Cube:
			if (texture->info.depth > 0)
			{
				if (compressed)
				{
					glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, position->mipLevel,
						position->x, position->y, position->depth*6 + position->face, width, height,
						layers, internalFormat, (GLsizei)size, data);
				}
				else
				{
					glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, position->mipLevel, position->x,
						position->y, position->depth*6 + position->face, width, height, layers,
						glFormat, type, data);
				}
			}
			else
			{
				for (unsigned int j = 0; j < layers; ++j)
				{
					if (compressed)
					{
						glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + position->face,
							position->mipLevel, position->x, position->y, width, height,
							internalFormat, (GLsizei)size, data);
					}
					else
					{
						glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + position->face,
							position->mipLevel, position->x, position->y, width, height, glFormat,
							type, data);
					}
				}
			}
			break;
		default:
			DS_ASSERT(false);
	}
	dsGLRenderer_endTextureOp(commandBuffer->renderer);

	return true;
}

bool dsGLMainCommandBuffer_copyTexture(dsCommandBuffer* commandBuffer, dsTexture* srcTexture,
	dsTexture* dstTexture, const dsTextureCopyRegion* regions, size_t regionCount)
{
	DS_UNUSED(commandBuffer);

	dsGLTexture* glSrcTexture = (dsGLTexture*)srcTexture;
	dsGLTexture* glDstTexture = (dsGLTexture*)dstTexture;
	if (ANYGL_SUPPORTED(glCopyImageSubData))
	{
		GLenum srcTarget = dsGLTexture_target(srcTexture);
		GLenum dstTarget = dsGLTexture_target(dstTexture);

		for (size_t i = 0; i < regionCount; ++i)
		{
			uint32_t srcLayer = regions[i].srcPosition.depth;
			if (srcTexture->info.dimension == dsTextureDim_Cube)
				srcLayer = srcLayer*6 + regions[i].dstPosition.face;
			uint32_t dstLayer = regions[i].dstPosition.depth;
			if (dstTexture->info.dimension == dsTextureDim_Cube)
				dstLayer = dstLayer*6 + regions[i].dstPosition.face;

			glCopyImageSubData(glSrcTexture->textureId, srcTarget, regions[i].srcPosition.mipLevel,
				regions[i].srcPosition.x, regions[i].srcPosition.y, srcLayer,
				glDstTexture->textureId, dstTarget, regions[i].dstPosition.mipLevel,
				regions[i].dstPosition.x, regions[i].dstPosition.y, dstLayer, regions[i].width,
				regions[i].height, regions[i].layers);
		}
	}
	else
	{
		dsRenderer* renderer = commandBuffer->renderer;
		GLuint tempFramebuffer = dsGLRenderer_tempFramebuffer(renderer);
		GLuint tempCopyFramebuffer = dsGLRenderer_tempCopyFramebuffer(renderer);
		DS_ASSERT(tempFramebuffer);
		DS_ASSERT(tempCopyFramebuffer);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, tempFramebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempCopyFramebuffer);

		GLbitfield buffers = dsGLTexture_buffers(srcTexture->info.format);
		for (uint32_t i = 0; i < regionCount; ++i)
		{
			uint32_t srcLayer = regions[i].srcPosition.depth;
			if (srcTexture->info.dimension == dsTextureDim_Cube)
				srcLayer = srcLayer*6 + regions[i].dstPosition.face;
			uint32_t dstLayer = regions[i].dstPosition.depth;
			if (dstTexture->info.dimension == dsTextureDim_Cube)
				dstLayer = dstLayer*6 + regions[i].dstPosition.face;

			for (uint32_t j = 0; j < regions[i].layers; ++j)
			{
				dsGLTexture_bindFramebufferTexture(srcTexture, GL_READ_FRAMEBUFFER,
					regions[i].srcPosition.mipLevel, srcLayer + j);
				dsGLTexture_bindFramebufferTexture(dstTexture, GL_DRAW_FRAMEBUFFER,
					regions[i].dstPosition.mipLevel, dstLayer + j);
				glBlitFramebuffer(regions[i].srcPosition.x, regions[i].srcPosition.y,
					regions[i].srcPosition.x + regions[i].width,
					regions[i].srcPosition.y + regions[i].height,
					regions[i].dstPosition.x, regions[i].dstPosition.y,
					regions[i].dstPosition.x + regions[i].width,
					regions[i].dstPosition.y + regions[i].height, buffers, GL_NEAREST);
			}
		}

		dsGLTexture_unbindFramebuffer(srcTexture, GL_READ_FRAMEBUFFER);
		dsGLTexture_unbindFramebuffer(dstTexture, GL_DRAW_FRAMEBUFFER);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		dsGLRenderer_restoreFramebuffer(renderer);
	}

	return true;
}

bool dsGLMainCommandBuffer_generateTextureMipmaps(dsCommandBuffer* commandBuffer,
	dsTexture* texture)
{
	dsGLTexture* glTexture = (dsGLTexture*)texture;
	if (ANYGL_SUPPORTED(glGenerateTextureMipmap))
		glGenerateTextureMipmap(glTexture->textureId);
	else
	{
		DS_ASSERT(ANYGL_SUPPORTED(glGenerateMipmap));
		GLenum target = dsGLTexture_target(texture);
		dsGLRenderer_beginTextureOp(commandBuffer->renderer, target, glTexture->textureId);
		// Some drivers may need the texture to be enabled.
		glEnable(target);
		glGenerateMipmap(target);
		glDisable(target);
		dsGLRenderer_endTextureOp(commandBuffer->renderer);
	}

	return true;
}

bool dsGLMainCommandBuffer_setFenceSyncs(dsCommandBuffer* commandBuffer, dsGLFenceSyncRef** syncs,
	uint32_t syncCount, bool bufferReadback)
{
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	if (((dsGLCommandBuffer*)commandBuffer)->boundRenderPass)
	{
		size_t index = glCommandBuffer->curFenceSyncs;
		if (!DS_RESIZEABLE_ARRAY_ADD(commandBuffer->allocator, glCommandBuffer->fenceSyncs,
			glCommandBuffer->curFenceSyncs, glCommandBuffer->maxFenceSyncs, syncCount))
		{
			return false;
		}

		DS_ASSERT(index + syncCount <= glCommandBuffer->maxFenceSyncs);
		for (size_t i = 0; i < syncCount; ++i)
		{
			glCommandBuffer->fenceSyncs[index + i] = syncs[i];
			dsGLFenceSyncRef_addRef(syncs[i]);
		}
		glCommandBuffer->curFenceSyncs += syncCount;

		if (bufferReadback)
			glCommandBuffer->bufferReadback = bufferReadback;

		return true;
	}
	else
		return setFences(commandBuffer->renderer, syncs, syncCount, bufferReadback);
}

bool dsGLMainCommandBuffer_bindShader(dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsDynamicRenderStates* renderStates)
{
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	const dsGLShader* glShader = (const dsGLShader*)shader;
	if (glCommandBuffer->currentProgram != glShader->programId)
	{
		glUseProgram(glShader->programId);
		glCommandBuffer->currentProgram = glShader->programId;
	}

	dsGLRenderStates_updateGLState(commandBuffer->renderer, &glCommandBuffer->currentState,
		&glShader->renderState, renderStates);
	updateSamplers(commandBuffer->renderer, glShader);
	DS_ASSERT(glCommandBuffer->curFramebuffer);
	dsGLFramebuffer_setDefaultSamples(glCommandBuffer->curFramebuffer, shader->samples);

	// Set the internal information on the shader.
	if (glShader->internalUniform >= 0)
	{
		DS_ASSERT(glCommandBuffer->curFramebuffer);
		dsGLRenderer* renderer = (dsGLRenderer*)commandBuffer->renderer;
		bool offscreen = renderer->curSurfaceType == GLSurfaceType_Framebuffer;
		float invertY = offscreen ? -1.0f : 1.0f;
		float height = (float)glCommandBuffer->curFramebuffer->height;
		float invWidth = 1.0f/(float)glCommandBuffer->curFramebuffer->width;
		float invHeight = invertY/height;
		glUniform4f(glShader->internalUniform, invertY, height, invWidth, invHeight);
	}
	return true;
}

bool dsGLMainCommandBuffer_setTexture(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsTexture* texture)
{
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	const dsGLShader* glShader = (const dsGLShader*)shader;
	dsGLTexture* glTexture = (dsGLTexture*)texture;

	uint32_t textureIndex = glShader->uniforms[element].location;
	uint32_t samplerIndex = glShader->uniforms[element].samplerIndex;
	GLuint textureId = glTexture ? glTexture->textureId : 0;
	GLenum target = dsGLTexture_target(texture);
	dsGLRenderer_bindTexture(commandBuffer->renderer, textureIndex, target, textureId);

	bool isShadowSampler = glShader->uniforms[element].isShadowSampler != 0;
	if (ANYGL_SUPPORTED(glBindSampler))
	{
		if (samplerIndex == MSL_UNKNOWN)
			glBindSampler(textureIndex, glCommandBuffer->defaultSamplers[isShadowSampler]);
		else
			glBindSampler(textureIndex, glShader->samplerIds[samplerIndex]);
	}
	else if (glTexture)
	{
		mslSamplerState* samplerState = NULL;;
		if (samplerIndex != MSL_UNKNOWN)
			samplerState = glShader->samplerStates + samplerIndex;
		dsGLTexture_setState(texture, samplerState, isShadowSampler);
	}

	return true;
}

bool dsGLMainCommandBuffer_setTextureBuffer(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsGfxBuffer* buffer, dsGfxFormat format, size_t offset, size_t count)
{
	DS_UNUSED(commandBuffer);
	DS_ASSERT(buffer);
	const dsGLShader* glShader = (const dsGLShader*)shader;
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	GLenum internalFormat;
	DS_VERIFY(dsGLResourceManager_getTextureFormatInfo(&internalFormat, NULL, NULL,
		shader->resourceManager, format));

	uint32_t textureIndex = glShader->uniforms[element].location;
	glActiveTexture(GL_TEXTURE0 + textureIndex);
	if (ANYGL_SUPPORTED(glTexBufferRange))
	{
		glTexBufferRange(GL_TEXTURE_BUFFER, internalFormat, glBuffer->bufferId, offset,
			dsGfxFormat_size(format)*count);
	}
	else
		glTextureBuffer(GL_TEXTURE_BUFFER, internalFormat, glBuffer->bufferId);

	return true;
}

bool dsGLMainCommandBuffer_setShaderBuffer(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsGfxBuffer* buffer, size_t offset, size_t size)
{
	DS_UNUSED(commandBuffer);
	const dsGLShader* glShader = (const dsGLShader*)shader;
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;

	GLenum type = 0;
	switch (shader->materialDesc->elements[element].type)
	{
		case dsMaterialType_UniformBlock:
		case dsMaterialType_VariableGroup:
			type = GL_UNIFORM_BUFFER;
			break;
		case dsMaterialType_UniformBuffer:
			type = GL_SHADER_STORAGE_BUFFER;
			break;
		default:
			DS_ASSERT(false);
	}

	glBindBufferRange(type, glShader->uniforms[element].location,
		glBuffer ? glBuffer->bufferId : 0, offset, size);

	return true;
}

bool dsGLMainCommandBuffer_setUniform(dsCommandBuffer* commandBuffer, GLint location,
	dsMaterialType type, uint32_t count, const void* data)
{
	DS_UNUSED(commandBuffer);
	count = dsMax(1U, count);
	// Compiling and getting the uniform locations should have already given errors for unsupporte
	// types, so shouldn't have to do error checking here.
	switch (type)
	{
		case dsMaterialType_Float:
			glUniform1fv(location, count, (const float*)data);
			break;
		case dsMaterialType_Vec2:
			glUniform2fv(location, count, (const float*)data);
			break;
		case dsMaterialType_Vec3:
			glUniform3fv(location, count, (const float*)data);
			break;
		case dsMaterialType_Vec4:
			glUniform4fv(location, count, (const float*)data);
			break;
		case dsMaterialType_Double:
			glUniform1dv(location, count, (const double*)data);
			break;
		case dsMaterialType_DVec2:
			glUniform2dv(location, count, (const double*)data);
			break;
		case dsMaterialType_DVec3:
			glUniform3dv(location, count, (const double*)data);
			break;
		case dsMaterialType_DVec4:
			glUniform4dv(location, count, (const double*)data);
			break;
		case dsMaterialType_Int:
		case dsMaterialType_Bool:
			glUniform1iv(location, count, (const int*)data);
			break;
		case dsMaterialType_IVec2:
		case dsMaterialType_BVec2:
			glUniform2iv(location, count, (const int*)data);
			break;
		case dsMaterialType_IVec3:
		case dsMaterialType_BVec3:
			glUniform3iv(location, count, (const int*)data);
			break;
		case dsMaterialType_IVec4:
		case dsMaterialType_BVec4:
			glUniform4iv(location, count, (const int*)data);
			break;
		case dsMaterialType_UInt:
			glUniform1uiv(location, count, (const unsigned int*)data);
			break;
		case dsMaterialType_UVec2:
			glUniform2uiv(location, count, (const unsigned int*)data);
			break;
		case dsMaterialType_UVec3:
			glUniform3uiv(location, count, (const unsigned int*)data);
			break;
		case dsMaterialType_UVec4:
			glUniform4uiv(location, count, (const unsigned int*)data);
			break;
		case dsMaterialType_Mat2:
			glUniformMatrix2fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat3:
			glUniformMatrix3fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat4:
			glUniformMatrix4fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat2x3:
			glUniformMatrix2x3fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat2x4:
			glUniformMatrix2x4fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat3x2:
			glUniformMatrix3x2fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat3x4:
			glUniformMatrix3x4fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat4x2:
			glUniformMatrix4x2fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat4x3:
			glUniformMatrix4x3fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_DMat2:
			glUniformMatrix2dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat3:
			glUniformMatrix3dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat4:
			glUniformMatrix4dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat2x3:
			glUniformMatrix2x3dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat2x4:
			glUniformMatrix2x3dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat3x2:
			glUniformMatrix3x2dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat3x4:
			glUniformMatrix3x4dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat4x2:
			glUniformMatrix4x2dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat4x3:
			glUniformMatrix4x3dv(location, count, false, (const double*)data);
			break;
		default:
			DS_ASSERT(false);
	}

	return true;
}

bool dsGLMainCommandBuffer_unbindShader(dsCommandBuffer* commandBuffer, const dsShader* shader)
{
	DS_UNUSED(commandBuffer);
	DS_UNUSED(shader);
	return true;
}

bool dsGLMainCommandBuffer_bindComputeShader(dsCommandBuffer* commandBuffer, const dsShader* shader)
{
	DS_UNUSED(commandBuffer);
	const dsGLShader* glShader = (const dsGLShader*)shader;
	glUseProgram(glShader->programId);
	return true;
}

bool dsGLMainCommandBuffer_unbindComputeShader(dsCommandBuffer* commandBuffer,
	const dsShader* shader)
{
	DS_UNUSED(commandBuffer);
	DS_UNUSED(shader);
	glUseProgram(0);
	return true;
}

bool dsGLMainCommandBuffer_beginRenderSurface(dsCommandBuffer* commandBuffer, void* glSurface)
{
	return dsGLRenderer_bindSurface(commandBuffer->renderer, glSurface);
}

bool dsGLMainCommandBuffer_endRenderSurface(dsCommandBuffer* commandBuffer, void* glSurface)
{
	DS_UNUSED(commandBuffer);
	DS_UNUSED(glSurface);
	return true;
}

bool dsGLMainCommandBuffer_beginRenderPass(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount)
{
	DS_ASSERT(clearValueCount == 0 || clearValueCount == renderPass->attachmentCount);
	DS_ASSERT(renderPass->attachmentCount == framebuffer->surfaceCount);

	// Cache the clear values so they can be executed when binding the framebuffer.
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	if (clearValueCount > glCommandBuffer->maxClearValues)
	{
		if (glCommandBuffer->clearValues)
			DS_VERIFY(dsAllocator_free(commandBuffer->allocator, glCommandBuffer->clearValues));

		dsSurfaceClearValue* newClearValues = (dsSurfaceClearValue*)dsAllocator_reallocWithFallback(
			commandBuffer->allocator, glCommandBuffer->clearValues, 0,
			clearValueCount*sizeof(dsSurfaceClearValue));
		if (!newClearValues)
			return false;

		glCommandBuffer->clearValues = newClearValues;
		glCommandBuffer->curClearValues = 0;
		glCommandBuffer->maxClearValues = clearValueCount;
	}

	glCommandBuffer->curClearValues = clearValueCount;
	if (clearValueCount > 0)
	{
		memcpy(glCommandBuffer->clearValues, clearValues,
			clearValueCount*sizeof(dsSurfaceClearValue));
	}

	// Set the viewport parameters.
	if (viewport)
	{
		glViewport((GLint)viewport->min.x, framebuffer->height - (GLint)viewport->min.y,
			(GLsizei)(viewport->max.x - viewport->min.x),
			(GLsizei)(viewport->max.y - viewport->min.y));
		if (ANYGL_SUPPORTED(glDepthRangef))
			glDepthRangef(viewport->min.z, viewport->max.z);
		else
			glDepthRange(viewport->min.z, viewport->max.z);
	}
	else
	{
		glViewport(0, 0, framebuffer->width, framebuffer->height);
		if (ANYGL_SUPPORTED(glDepthRangef))
			glDepthRangef(0, 1);
		else
			glDepthRange(0, 1);
	}

	glCommandBuffer->curFramebuffer = framebuffer;
	if (!beginRenderSubpass(glCommandBuffer, renderPass, 0))
	{
		glCommandBuffer->curFramebuffer = NULL;
		return false;
	}

	return true;
}

bool dsGLMainCommandBuffer_nextRenderSubpass(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t subpassIndex)
{
	DS_ASSERT(subpassIndex > 0);
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	if (!endRenderSubpass(glCommandBuffer, renderPass, subpassIndex - 1))
		return false;
	return beginRenderSubpass(glCommandBuffer, renderPass, subpassIndex);
}

bool dsGLMainCommandBuffer_endRenderPass(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass)
{
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	if (!endRenderSubpass(glCommandBuffer, renderPass, renderPass->subpassCount - 1))
		return false;

	// Resolve any remaining attachments.
	GLuint readFbo = 0;
	GLuint writeFbo = 0;
	dsTexture* texture = NULL;
	dsRenderer* renderer = commandBuffer->renderer;
	const dsFramebuffer* framebuffer = glCommandBuffer->curFramebuffer;
	for (uint32_t i = 0; i < renderPass->attachmentCount; ++i)
	{
		if (!(renderPass->attachments[i].usage & dsAttachmentUsage_Resolve))
			continue;

		if (framebuffer->surfaces[i].surfaceType != dsGfxSurfaceType_Texture)
			continue;

		texture = (dsTexture*)framebuffer->surfaces[i].surface;
		dsGLTexture* glTexture = (dsGLTexture*)texture;
		if (!glTexture->drawBufferId)
			continue;

		if (!readFbo)
		{
			readFbo = dsGLRenderer_tempFramebuffer(renderer);
			writeFbo = dsGLRenderer_tempCopyFramebuffer(renderer);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, writeFbo);
		}

		GLenum buffers = dsGLTexture_attachment(texture->info.format);
		GLbitfield bufferMask = dsGLTexture_buffers(texture->info.format);
		glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, buffers, GL_RENDERBUFFER,
			glTexture->drawBufferId);
		dsGLTexture_bindFramebufferTextureAttachment(texture, GL_DRAW_FRAMEBUFFER, buffers,
			framebuffer->surfaces[i].mipLevel, framebuffer->surfaces[i].layer);

		glBlitFramebuffer(0, 0, texture->info.width, texture->info.height, 0, 0,
			texture->info.width, texture->info.height, bufferMask, GL_NEAREST);
	}

	if (readFbo)
	{
		DS_ASSERT(writeFbo);
		DS_ASSERT(texture);
		dsGLTexture_unbindFramebuffer(texture, GL_DRAW_FRAMEBUFFER);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		dsGLRenderer_restoreFramebuffer(renderer);
	}

	glCommandBuffer->curFramebuffer = NULL;

	// Clear these out at the end of the render pass to avoid bad states if deleting and re-creating
	// objects.
	glUseProgram(0);
	glCommandBuffer->curGeometry = NULL;
	glCommandBuffer->curDrawIndirectBuffer = NULL;
	glCommandBuffer->curDispatchIndirectBuffer = NULL;
	glCommandBuffer->currentProgram = 0;
	return true;
}

bool dsGLMainCommandBuffer_clearColorSurface(dsCommandBuffer* commandBuffer,
	const dsFramebufferSurface* surface, const dsSurfaceColorValue* colorValue)
{
	GLSurfaceType surfaceType = dsGLFramebuffer_getSurfaceType(surface->surfaceType);
	dsSurfaceClearValue value;
	value.colorValue = *colorValue;
	if (surfaceType == GLSurfaceType_Framebuffer)
	{
		uint32_t fbo = dsGLRenderer_tempCopyFramebuffer(commandBuffer->renderer);
		DS_ASSERT(fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		dsGfxFormat format;
		if (surface->surfaceType == dsGfxSurfaceType_Texture)
		{
			dsTexture* texture = (dsTexture*)surface->surface;
			format = texture->info.format;
			format = ((dsRenderbuffer*)surface->surface)->format;
			dsGLTexture_bindFramebuffer(texture, GL_FRAMEBUFFER, surface->mipLevel, surface->layer);
		}
		else
		{
			DS_ASSERT(surface->surfaceType == dsGfxSurfaceType_Renderbuffer);
			format = ((dsRenderbuffer*)surface->surface)->format;
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
				((dsGLRenderbuffer*)surface->surface)->renderbufferId);
		}

		if (ANYGL_SUPPORTED(glClearBufferfv))
			clearDrawBuffer(format, 0, &value);
		else
		{
			dsGLRenderer_bindFramebuffer(commandBuffer->renderer, surfaceType, 0,
				GLFramebufferFlags_Default);
			setClearColor(format, &value);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
		if (surface->surfaceType == dsGfxSurfaceType_Texture)
		{
			dsTexture* texture = (dsTexture*)surface->surface;
			dsGLTexture_unbindFramebuffer(texture, GL_FRAMEBUFFER);
		}
		dsGLRenderer_restoreFramebuffer(commandBuffer->renderer);
	}
	else
	{
		dsGLRenderer_bindFramebuffer(commandBuffer->renderer, surfaceType, 0,
			GLFramebufferFlags_Default);
		setClearColor(commandBuffer->renderer->surfaceColorFormat, &value);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	return true;
}

bool dsGLMainCommandBuffer_clearDepthStencilSurface(dsCommandBuffer* commandBuffer,
	const dsFramebufferSurface* surface, dsClearDepthStencil surfaceParts,
	const dsDepthStencilValue* depthStencilValue)
{
	GLSurfaceType surfaceType = dsGLFramebuffer_getSurfaceType(surface->surfaceType);
	dsSurfaceClearValue value;
	value.depthStencil = *depthStencilValue;
	if (surfaceType == GLSurfaceType_Framebuffer)
	{
		GLenum attachment = 0;
		switch (surfaceParts)
		{
			case dsClearDepthStencil_Depth:
				attachment = GL_DEPTH;
				break;
			case dsClearDepthStencil_Stencil:
				attachment = GL_STENCIL;
				break;
			case dsClearDepthStencil_Both:
				attachment = GL_DEPTH_STENCIL;
				break;
		}

		uint32_t fbo = dsGLRenderer_tempCopyFramebuffer(commandBuffer->renderer);
		DS_ASSERT(fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		dsGfxFormat format;
		uint32_t width;
		uint32_t height;
		uint32_t samples;
		if (surface->surfaceType == dsGfxSurfaceType_Texture)
		{
			dsTexture* texture = (dsTexture*)surface->surface;
			format = texture->info.format;
			width = dsMax(1U, texture->info.width >> surface->mipLevel);
			height = dsMax(1U, texture->info.height >> surface->mipLevel);
			samples = texture->info.samples;
			dsGLTexture_bindFramebuffer(texture, GL_FRAMEBUFFER, surface->mipLevel, surface->layer);
		}
		else
		{
			DS_ASSERT(surface->surfaceType == dsGfxSurfaceType_Renderbuffer);
			dsRenderbuffer* renderbuffer = (dsRenderbuffer*)surface->surface;
			format = renderbuffer->format;
			width = renderbuffer->width;
			height = renderbuffer->height;
			samples = renderbuffer->samples;
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
				((dsGLRenderbuffer*)surface->surface)->renderbufferId);
		}

		if (ANYGL_SUPPORTED(glClearBufferfv))
			clearDrawBufferPart(attachment, format, 0, &value);
		else
		{
			if (ANYGL_SUPPORTED(glDrawBuffer))
				glDrawBuffer(GL_NONE);
			else
			{
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
					createTempRenderbuffer((dsGLMainCommandBuffer*)commandBuffer, width, height,
						samples));
			}

			setClearColor(commandBuffer->renderer->surfaceColorFormat, &value);
			switch (surfaceParts)
			{
				case dsClearDepthStencil_Depth:
					glClear(GL_DEPTH_BUFFER_BIT);
					break;
				case dsClearDepthStencil_Stencil:
					glClear(GL_STENCIL_BUFFER_BIT);
					break;
				case dsClearDepthStencil_Both:
					glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
					break;
			}

			if (ANYGL_SUPPORTED(glDrawBuffer))
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
			else
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
		}

		glFramebufferTexture(GL_FRAMEBUFFER, attachment, 0, 0);
		if (surface->surfaceType == dsGfxSurfaceType_Texture)
		{
			dsTexture* texture = (dsTexture*)surface->surface;
			dsGLTexture_unbindFramebuffer(texture, GL_FRAMEBUFFER);
		}
		dsGLRenderer_restoreFramebuffer(commandBuffer->renderer);
	}
	else
	{
		dsGLRenderer_bindFramebuffer(commandBuffer->renderer, surfaceType, 0,
			GLFramebufferFlags_Default);
		setClearColor(commandBuffer->renderer->surfaceColorFormat, &value);
		switch (surfaceParts)
		{
			case dsClearDepthStencil_Depth:
				glClear(GL_DEPTH_BUFFER_BIT);
				break;
			case dsClearDepthStencil_Stencil:
				glClear(GL_STENCIL_BUFFER_BIT);
				break;
			case dsClearDepthStencil_Both:
				glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				break;
		}
	}

	return true;
}

bool dsGLMainCommandBuffer_draw(dsCommandBuffer* commandBuffer, const dsDrawGeometry* geometry,
	const dsDrawRange* drawRange)
{
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	if (glCommandBuffer->curGeometry != geometry || glCommandBuffer->curBaseVertex != 0)
	{
		dsGLDrawGeometry_bind(geometry, 0);
		glCommandBuffer->curGeometry = geometry;
		glCommandBuffer->curBaseVertex = 0;
	}

	dsPrimitiveType primitiveType = ((dsGLCommandBuffer*)commandBuffer)->boundShader->primitiveType;
	DS_ASSERT(primitiveType < DS_ARRAY_SIZE(primitiveTypeMap));
	if (drawRange->instanceCount == 1)
	{
		glDrawArrays(primitiveTypeMap[primitiveType], drawRange->firstVertex,
			drawRange->vertexCount);
	}
	else
	{
		if (drawRange->firstInstance == 0)
		{
			glDrawArraysInstanced(primitiveTypeMap[primitiveType],
				drawRange->firstVertex, drawRange->vertexCount, drawRange->instanceCount);
		}
		else
		{
			glDrawArraysInstancedBaseInstance(primitiveTypeMap[primitiveType],
				drawRange->firstVertex, drawRange->vertexCount, drawRange->firstInstance,
				drawRange->instanceCount);
		}
	}

	return true;
}

bool dsGLMainCommandBuffer_drawIndexed(dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawIndexedRange* drawRange)
{
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	int32_t baseVertex = 0;
	if (!ANYGL_SUPPORTED(glDrawElementsBaseVertex))
		baseVertex = drawRange->vertexOffset;
	if (glCommandBuffer->curGeometry != geometry || glCommandBuffer->curBaseVertex != baseVertex)
	{
		dsGLDrawGeometry_bind(geometry, baseVertex);
		glCommandBuffer->curGeometry = geometry;
		glCommandBuffer->curBaseVertex = baseVertex;
	}

	dsPrimitiveType primitiveType = ((dsGLCommandBuffer*)commandBuffer)->boundShader->primitiveType;
	DS_ASSERT(primitiveType < DS_ARRAY_SIZE(primitiveTypeMap));
	GLenum indexType = geometry->indexBuffer.indexSize == sizeof(uint32_t) ? GL_UNSIGNED_INT :
		GL_UNSIGNED_SHORT;
	if (drawRange->instanceCount == 1)
	{
		if (ANYGL_SUPPORTED(glDrawElementsBaseVertex))
		{
			glDrawElementsBaseVertex(primitiveTypeMap[primitiveType], drawRange->indexCount,
				indexType, (void*)(size_t)geometry->indexBuffer.offset, drawRange->vertexOffset);
		}
		else
		{
			glDrawElements(primitiveTypeMap[primitiveType], drawRange->indexCount, indexType,
				(void*)(size_t)geometry->indexBuffer.offset);
		}
	}
	else
	{
		if (drawRange->firstInstance == 0)
		{
			glDrawElementsInstancedBaseVertex(primitiveTypeMap[primitiveType],
				drawRange->indexCount, indexType, (void*)(size_t)geometry->indexBuffer.offset,
				drawRange->instanceCount, drawRange->vertexOffset);
		}
		else
		{
			glDrawElementsInstancedBaseVertexBaseInstance(primitiveTypeMap[primitiveType],
				drawRange->indexCount, indexType, (void*)(size_t)geometry->indexBuffer.offset,
				drawRange->instanceCount, drawRange->vertexOffset, drawRange->firstInstance);
		}
	}

	return true;
}

bool dsGLMainCommandBuffer_drawIndirect(dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride)
{
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	if (glCommandBuffer->curGeometry != geometry || glCommandBuffer->curBaseVertex != 0)
	{
		dsGLDrawGeometry_bind(geometry, 0);
		glCommandBuffer->curGeometry = geometry;
		glCommandBuffer->curBaseVertex = 0;
	}

	if (glCommandBuffer->curDrawIndirectBuffer != indirectBuffer)
	{
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ((const dsGLGfxBuffer*)indirectBuffer)->bufferId);
		glCommandBuffer->curDrawIndirectBuffer = indirectBuffer;
	}

	dsPrimitiveType primitiveType = ((dsGLCommandBuffer*)commandBuffer)->boundShader->primitiveType;
	DS_ASSERT(primitiveType < DS_ARRAY_SIZE(primitiveTypeMap));
	if (ANYGL_SUPPORTED(glMultiDrawArraysIndirect))
	{
		glMultiDrawArraysIndirect(primitiveTypeMap[primitiveType], (void*)(size_t)offset,
			count, stride);
	}
	else
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			glDrawArraysIndirect(primitiveTypeMap[primitiveType],
				(void*)(size_t)(offset + i*stride));
		}
	}

	return true;
}

bool dsGLMainCommandBuffer_drawIndexedIndirect(dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride)
{
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	if (glCommandBuffer->curGeometry != geometry || glCommandBuffer->curBaseVertex != 0)
	{
		dsGLDrawGeometry_bind(geometry, 0);
		glCommandBuffer->curGeometry = geometry;
		glCommandBuffer->curBaseVertex = 0;
	}

	if (glCommandBuffer->curDrawIndirectBuffer != indirectBuffer)
	{
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ((const dsGLGfxBuffer*)indirectBuffer)->bufferId);
		glCommandBuffer->curDrawIndirectBuffer = indirectBuffer;
	}

	dsPrimitiveType primitiveType = ((dsGLCommandBuffer*)commandBuffer)->boundShader->primitiveType;
	DS_ASSERT(primitiveType < DS_ARRAY_SIZE(primitiveTypeMap));
	GLenum indexType = geometry->indexBuffer.indexSize == sizeof(uint32_t) ? GL_UNSIGNED_INT :
		GL_UNSIGNED_SHORT;
	if (ANYGL_SUPPORTED(glMultiDrawElementsIndirect))
	{
		glMultiDrawElementsIndirect(primitiveTypeMap[primitiveType], indexType,
			(void*)(size_t)offset, count, stride);
	}
	else
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			glDrawElementsIndirect(primitiveTypeMap[primitiveType], indexType,
				(void*)(size_t)(offset + i*stride));
		}
	}

	return true;
}

bool dsGLMainCommandBuffer_dispatchCompute(dsCommandBuffer* commandBuffer, uint32_t x, uint32_t y,
	uint32_t z)
{
	DS_UNUSED(commandBuffer);
	glDispatchCompute(x, y, z);
	return true;
}

bool dsGLMainCommandBuffer_dispatchComputeIndirect(dsCommandBuffer* commandBuffer,
	const dsGfxBuffer* indirectBuffer, size_t offset)
{
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	if (glCommandBuffer->curDispatchIndirectBuffer != indirectBuffer)
	{
		glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, ((const dsGLGfxBuffer*)indirectBuffer)->bufferId);
		glCommandBuffer->curDispatchIndirectBuffer = indirectBuffer;
	}

	glDispatchComputeIndirect(offset);
	return true;
}

bool dsGLMainCommandBuffer_blitSurface(dsCommandBuffer* commandBuffer,
	dsGfxSurfaceType srcSurfaceType, void* srcSurface, dsGfxSurfaceType dstSurfaceType,
	void* dstSurface, const dsSurfaceBlitRegion* regions, size_t regionCount, dsBlitFilter filter)
{
	dsRenderer* renderer = commandBuffer->renderer;

	GLSurfaceType srcGLSurfaceType = dsGLFramebuffer_getSurfaceType(srcSurfaceType);
	GLSurfaceType dstGLSurfaceType = dsGLFramebuffer_getSurfaceType(dstSurfaceType);

	GLuint srcFramebuffer = 0;
	if (srcGLSurfaceType == GLSurfaceType_Framebuffer)
	{
		srcFramebuffer = dsGLRenderer_tempFramebuffer(renderer);
		DS_ASSERT(srcFramebuffer);
	}

	GLuint dstFramebuffer = 0;
	if (dstGLSurfaceType == GLSurfaceType_Framebuffer)
	{
		dstFramebuffer = dsGLRenderer_tempCopyFramebuffer(renderer);
		DS_ASSERT(dstFramebuffer);
	}

	dsGLRenderer_bindFramebuffer(renderer, srcGLSurfaceType, srcFramebuffer,
		GLFramebufferFlags_Read | GLFramebufferFlags_Temporary);
	dsGLRenderer_bindFramebuffer(renderer, dstGLSurfaceType, dstFramebuffer,
		GLFramebufferFlags_Temporary);

	GLbitfield buffers = dsGLTexture_buffers(
		getSurfaceFormat(renderer, srcSurfaceType, srcSurface));
	uint32_t srcWidth, srcHeight, srcFaces;
	bool srcInvertY;
	uint32_t dstWidth, dstHeight, dstFaces;
	bool dstInvertY;
	getSurfaceInfo(&srcWidth, &srcHeight, &srcFaces, &srcInvertY, srcSurfaceType, srcSurface);
	getSurfaceInfo(&dstWidth, &dstHeight, &dstFaces, &dstInvertY, dstSurfaceType, dstSurface);
	for (uint32_t i = 0; i < regionCount; ++i)
	{
		uint32_t srcLayer = regions[i].srcPosition.depth;
		if (srcFaces == 6)
			srcLayer = srcLayer*6 + regions[i].dstPosition.face;
		uint32_t dstLayer = regions[i].dstPosition.depth;
		if (dstFaces == 6)
			dstLayer = dstLayer*6 + regions[i].dstPosition.face;

		uint32_t curSrcHeight = dsMax(srcHeight >> regions[i].srcPosition.mipLevel, 1U);
		uint32_t curDstHeight = dsMax(dstHeight >> regions[i].dstPosition.mipLevel, 1U);

		for (uint32_t j = 0; j < regions[i].layers; ++j)
		{
			bindBlitSurface(GL_READ_FRAMEBUFFER, srcSurfaceType, srcSurface,
				regions[i].srcPosition.mipLevel, srcLayer + j);
			bindBlitSurface(GL_DRAW_FRAMEBUFFER, dstSurfaceType, dstSurface,
				regions[i].srcPosition.mipLevel, srcLayer + j);

			uint32_t srcY = regions[i].srcPosition.y;
			int srcYMult = 1;
			if (srcInvertY)
			{
				srcY = curSrcHeight - srcY;
				srcYMult = -1;
			}

			uint32_t dstY = regions[i].dstPosition.y;
			int dstYMult = 1;
			if (dstInvertY)
			{
				dstY = curDstHeight - dstY;
				dstYMult = -1;
			}

			glBlitFramebuffer(regions[i].srcPosition.x, srcY,
				regions[i].srcPosition.x + regions[i].srcWidth,
				srcY + srcYMult*regions[i].srcHeight,
				regions[i].dstPosition.x, dstY,
				regions[i].dstPosition.x + regions[i].dstWidth,
				dstY + dstYMult*regions[i].dstHeight,
				buffers, filter == dsBlitFilter_Linear ? GL_LINEAR : GL_NEAREST);
		}
	}

	unbindBlitSurface(GL_READ_FRAMEBUFFER, srcSurfaceType, srcSurface);
	unbindBlitSurface(GL_DRAW_FRAMEBUFFER, dstSurfaceType, dstSurface);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	dsGLRenderer_restoreFramebuffer(renderer);

	return true;
}

bool dsGLMainCommandBuffer_begin(dsCommandBuffer* commandBuffer, const dsRenderPass* renderPass,
	uint32_t subpassIndex, const dsFramebuffer* framebuffer)
{
	DS_UNUSED(commandBuffer);
	DS_UNUSED(renderPass);
	DS_UNUSED(subpassIndex);
	DS_UNUSED(framebuffer);

	errno = EPERM;
	DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Cannot begin or end the main command buffer.");
	return false;
}

bool dsGLMainCommandBuffer_end(dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);

	errno = EPERM;
	DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Cannot begin or end the main command buffer.");
	return false;
}

bool dsGLMainCommandBuffer_submit(dsCommandBuffer* commandBuffer, dsCommandBuffer* submitBuffer)
{
	DS_UNUSED(commandBuffer);
	DS_UNUSED(submitBuffer);
	errno = EPERM;
	DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Cannot submit the main command buffer.");
	return false;
}

static CommandBufferFunctionTable functionTable =
{
	&dsGLMainCommandBuffer_copyBufferData,
	&dsGLMainCommandBuffer_copyBuffer,
	&dsGLMainCommandBuffer_copyTextureData,
	&dsGLMainCommandBuffer_copyTexture,
	&dsGLMainCommandBuffer_generateTextureMipmaps,
	&dsGLMainCommandBuffer_setFenceSyncs,
	&dsGLMainCommandBuffer_bindShader,
	&dsGLMainCommandBuffer_setTexture,
	&dsGLMainCommandBuffer_setTextureBuffer,
	&dsGLMainCommandBuffer_setShaderBuffer,
	&dsGLMainCommandBuffer_setUniform,
	&dsGLMainCommandBuffer_unbindShader,
	&dsGLMainCommandBuffer_bindComputeShader,
	&dsGLMainCommandBuffer_unbindComputeShader,
	&dsGLMainCommandBuffer_beginRenderSurface,
	&dsGLMainCommandBuffer_endRenderSurface,
	&dsGLMainCommandBuffer_beginRenderPass,
	&dsGLMainCommandBuffer_nextRenderSubpass,
	&dsGLMainCommandBuffer_endRenderPass,
	&dsGLMainCommandBuffer_clearColorSurface,
	&dsGLMainCommandBuffer_clearDepthStencilSurface,
	&dsGLMainCommandBuffer_draw,
	&dsGLMainCommandBuffer_drawIndexed,
	&dsGLMainCommandBuffer_drawIndirect,
	&dsGLMainCommandBuffer_drawIndexedIndirect,
	&dsGLMainCommandBuffer_dispatchCompute,
	&dsGLMainCommandBuffer_dispatchComputeIndirect,
	&dsGLMainCommandBuffer_blitSurface,
	&dsGLMainCommandBuffer_begin,
	&dsGLMainCommandBuffer_end,
	&dsGLMainCommandBuffer_submit
};

dsGLMainCommandBuffer* dsGLMainCommandBuffer_create(dsRenderer* renderer, dsAllocator* allocator)
{
	DS_ASSERT(allocator->freeFunc);
	dsGLMainCommandBuffer* commandBuffer = DS_ALLOCATE_OBJECT(allocator, dsGLMainCommandBuffer);
	if (!commandBuffer)
		return NULL;

	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	baseCommandBuffer->renderer = renderer;
	baseCommandBuffer->allocator = allocator;
	baseCommandBuffer->usage = dsCommandBufferUsage_Standard;

	((dsGLCommandBuffer*)commandBuffer)->functions = &functionTable;
	commandBuffer->fenceSyncs = NULL;
	commandBuffer->curFenceSyncs = 0;
	commandBuffer->maxFenceSyncs = 0;
	commandBuffer->bufferReadback = false;

	memset(commandBuffer->tempRenderbuffers, 0, sizeof(commandBuffer->tempRenderbuffers));
	commandBuffer->tempRenderbufferCounter = 0;

	commandBuffer->curGeometry = NULL;
	commandBuffer->curDrawIndirectBuffer = NULL;
	commandBuffer->curDispatchIndirectBuffer = NULL;
	commandBuffer->curBaseVertex = 0;

	commandBuffer->clearValues = NULL;
	commandBuffer->curClearValues = 0;
	commandBuffer->maxClearValues = 0;

	commandBuffer->currentProgram = 0;

	if (ANYGL_SUPPORTED(glGenSamplers))
	{
		glGenSamplers(2, commandBuffer->defaultSamplers);
		glSamplerParameteri(commandBuffer->defaultSamplers[1], GL_TEXTURE_COMPARE_MODE,
			GL_COMPARE_R_TO_TEXTURE);
	}

	commandBuffer->defaultSamplerState.minFilter = mslFilter_Unset;
	commandBuffer->defaultSamplerState.magFilter = mslFilter_Unset;
	commandBuffer->defaultSamplerState.mipFilter = mslMipFilter_Unset;
	commandBuffer->defaultSamplerState.addressModeU = mslAddressMode_Unset;
	commandBuffer->defaultSamplerState.addressModeV = mslAddressMode_Unset;
	commandBuffer->defaultSamplerState.addressModeW = mslAddressMode_Unset;
	commandBuffer->defaultSamplerState.mipLodBias = MSL_UNKNOWN_FLOAT;
	commandBuffer->defaultSamplerState.minLod = MSL_UNKNOWN_FLOAT;
	commandBuffer->defaultSamplerState.maxLod = MSL_UNKNOWN_FLOAT;
	commandBuffer->defaultSamplerState.borderColor = mslBorderColor_Unset;
	commandBuffer->defaultSamplerState.compareOp = mslCompareOp_Unset;

	dsGLCommandBuffer_initialize(baseCommandBuffer, false);

	return commandBuffer;
}

void dsGLMainCommandBuffer_resetState(dsGLMainCommandBuffer* commandBuffer)
{
	dsGLRenderStates_initialize(&commandBuffer->currentState);

	if (AnyGL_atLeastVersion(3, 2, false) || AnyGL_ARB_depth_clamp)
		glDisable(GL_DEPTH_CLAMP);
	if (ANYGL_SUPPORTED(glPolygonMode))
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0, 0);
	glLineWidth(1.0f);

	if (AnyGL_atLeastVersion(1, 3, false))
	{
		glEnable(GL_MULTISAMPLE);
		glDisable(GL_SAMPLE_ALPHA_TO_ONE);
	}

	if (ANYGL_SUPPORTED(glMinSampleShading))
	{
		glDisable(GL_SAMPLE_SHADING);
		glMinSampleShading(1.0f);
	}

	if (ANYGL_SUPPORTED(glSampleMaski))
	{
		glDisable(GL_SAMPLE_MASK);
		glSampleMaski(0, 0xFFFFFFFF);
	}

	glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(true);
	glDepthFunc(GL_LESS);
	if (AnyGL_EXT_depth_bounds_test)
	{
		glDisable(GL_DEPTH_BOUNDS_TEST_EXT);
		glDepthBoundsEXT(0, 1);
	}
	glDisable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);

	if (ANYGL_SUPPORTED(glLogicOp))
	{
		glDisable(GL_COLOR_LOGIC_OP);
		glLogicOp(GL_COPY);
	}
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);
	glBlendEquation(GL_FUNC_ADD);
	glColorMask(true, true, true, true);
	glBlendColor(0, 0, 0, 0);

	if (AnyGL_atLeastVersion(3, 2, false) || AnyGL_ARB_seamless_cube_map)
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
}

bool dsGLMainCommandBuffer_destroy(dsGLMainCommandBuffer* commandBuffer)
{
	if (!commandBuffer)
		return true;

	dsAllocator* allocator = ((dsCommandBuffer*)commandBuffer)->allocator;

	if (commandBuffer->fenceSyncs)
	{
		for (size_t i = 0; i < commandBuffer->curFenceSyncs; ++i)
			dsGLFenceSyncRef_freeRef(commandBuffer->fenceSyncs[i]);
		DS_VERIFY(dsAllocator_free(allocator, commandBuffer->fenceSyncs));
	}

	DS_VERIFY(dsAllocator_free(allocator, commandBuffer->clearValues));

	if (ANYGL_SUPPORTED(glDeleteSamplers))
		glDeleteSamplers(2, commandBuffer->defaultSamplers);

	dsGLCommandBuffer_shutdown((dsCommandBuffer*)commandBuffer);
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer));
	return true;
}