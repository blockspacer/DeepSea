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

#include <DeepSea/Render/Resources/VertexFormat.h>

#include <DeepSea/Core/Bits.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <errno.h>
#include <string.h>

bool dsVertexFormat_initialize(dsVertexFormat* format)
{
	if (!format)
	{
		errno = EINVAL;
		return false;
	}

	memset(format->elements, 0, sizeof(format->elements));
	format->enabledMask = 0;
	format->size = 0;
	return true;
}

bool dsVertexFormat_getAttribEnabled(const dsVertexFormat* format, unsigned int attrib)
{
	if (!format || attrib >= DS_MAX_ALLOWED_VERTEX_ATTRIBS)
		return false;

	return (format->enabledMask & (1 << attrib)) != 0;
}

bool dsVertexFormat_setAttribEnabled(dsVertexFormat* format, unsigned int attrib, bool enabled)
{
	if (!format)
	{
		errno = EINVAL;
		return false;
	}

	if (attrib >= DS_MAX_ALLOWED_VERTEX_ATTRIBS)
	{
		errno = ERANGE;
		return false;
	}

	if (enabled)
		format->enabledMask |= 1 << attrib;
	else
		format->enabledMask &= ~(1 << attrib);
	return true;
}

bool dsVertexFormat_computeOffsetsAndSize(dsVertexFormat* format)
{
	if (!format)
	{
		errno = EINVAL;
		return false;
	}

	format->size = 0;
	for (uint32_t mask = format->enabledMask; mask; mask = dsRemoveLastBit(mask))
	{
		uint32_t i = dsBitmaskIndex(mask);
		uint16_t curSize = (uint16_t)dsGfxFormat_size(format->elements[i].format);
		if (!curSize)
		{
			errno = EINVAL;
			return false;
		}

		format->elements[i].offset = format->size;
		format->elements[i].size = curSize;
		format->size = (uint16_t)(format->size + curSize);
	}

	return true;
}
