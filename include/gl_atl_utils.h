#pragma once

#include "atl_types.h"
#include "gl_utils.h"

GLenum color_format_to_gl_enum(Atlas::ColorFormat format) {
	switch (format) {
	case Atlas::ColorFormat::R8G8B8A8: return GL_RGBA8;
	case Atlas::ColorFormat::D32: return GL_DEPTH_COMPONENT32F;
	case Atlas::ColorFormat::D24S8: return GL_DEPTH24_STENCIL8;
	}

	CORE_ASSERT(false, "color_format_to_gl_enum: color format {} not defined", (uint32_t)format);
	return 0;
}

bool is_color_attachment(Atlas::ColorFormat format) {
	switch (format) {
	case Atlas::ColorFormat::R8G8B8A8: return true;
	case Atlas::ColorFormat::D32: return false;
	case Atlas::ColorFormat::D24S8: return false;
	}

	CORE_ASSERT(false, "is_color_attachment: color format {} not defined", (uint32_t)format);
	return 0;
}

GLenum color_format_to_gl_attachment(Atlas::ColorFormat format) {
	switch (format)
	{
	case Atlas::ColorFormat::R8G8B8A8: return GL_COLOR_ATTACHMENT0;
	case Atlas::ColorFormat::D32: return GL_DEPTH_ATTACHMENT;
	case Atlas::ColorFormat::D24S8: return GL_DEPTH_STENCIL_ATTACHMENT;
	}

	CORE_ASSERT(false, "color_format_to_gl_attachment: color format {} not defined", (uint32_t)format);
	return 0;
}

GLenum texture_min_filter_to_gl_enum(Atlas::TextureFilter filter, bool mipmap) {
	switch (filter)
	{
	case Atlas::TextureFilter::LINEAR: return mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
	case Atlas::TextureFilter::NEAREST: return GL_NEAREST;
	}

	CORE_ASSERT(false, "texture_min_filter_to_gl_enum: texture format {} not defined", (uint32_t)filter);
	return 0;
}

GLenum texture_mag_filter_to_gl_enum(Atlas::TextureFilter filter, bool mipmap) {
	switch (filter)
	{
	case Atlas::TextureFilter::LINEAR: return GL_LINEAR;
	case Atlas::TextureFilter::NEAREST: return GL_NEAREST;
	}

	CORE_ASSERT(false, "texture_min_filter_to_gl_enum: texture format {} not defined", (uint32_t)filter);
	return 0;
}