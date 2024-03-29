#pragma once

#include "atl_types.h"
#include "gl_utils.h"

GLenum color_format_to_int_gl_enum(const Atlas::ColorFormat format) {
	switch (format) {
	case Atlas::ColorFormat::R8G8B8: return GL_RGB;
	case Atlas::ColorFormat::R8G8B8A8: return GL_RGBA;
	case Atlas::ColorFormat::D32: return GL_DEPTH_COMPONENT;
	}

	CORE_ASSERT(false, "color_format_to_gl_enum: color format {} not defined", (uint32_t)format);
	return 0;
}

GLenum color_format_to_gl_enum(const Atlas::ColorFormat format) {
	switch (format) {
	case Atlas::ColorFormat::R8G8B8: return GL_RGB8;
	case Atlas::ColorFormat::R8G8B8A8: return GL_RGBA8;
	case Atlas::ColorFormat::D32: return GL_DEPTH_COMPONENT32F;
	case Atlas::ColorFormat::D24S8: return GL_DEPTH24_STENCIL8;
	}

	CORE_ASSERT(false, "color_format_to_gl_enum: color format {} not defined", (uint32_t)format);
	return 0;
}

bool is_color_attachment(const Atlas::ColorFormat format) {
	switch (format) {
	case Atlas::ColorFormat::R8G8B8:
	case Atlas::ColorFormat::R8G8B8A8: return true;

	case Atlas::ColorFormat::D32: return false;
	case Atlas::ColorFormat::D24S8: return false;
	}

	CORE_ASSERT(false, "is_color_attachment: color format {} not defined", (uint32_t)format);
	return 0;
}

GLenum color_format_to_gl_attachment(const Atlas::ColorFormat format) {
	switch (format)
	{
	case Atlas::ColorFormat::R8G8B8A8: return GL_COLOR_ATTACHMENT0;
	case Atlas::ColorFormat::R8G8B8: return GL_COLOR_ATTACHMENT0;
	case Atlas::ColorFormat::D32: return GL_DEPTH_ATTACHMENT;
	case Atlas::ColorFormat::D24S8: return GL_DEPTH_STENCIL_ATTACHMENT;
	}

	CORE_ASSERT(false, "color_format_to_gl_attachment: color format {} not defined", (uint32_t)format);
	return 0;
}

uint32_t color_format_to_bytes(const Atlas::ColorFormat format) {
	switch (format)
	{
	case Atlas::ColorFormat::R8G8B8A8:
	case Atlas::ColorFormat::D32:
	case Atlas::ColorFormat::D24S8: return 4;
	case Atlas::ColorFormat::R8G8B8: return 3;
	}

	CORE_ASSERT(false, "color_format_to_gl_attachment: color format {} not defined", (uint32_t)format);
	return 0;
}

GLenum texture_min_filter_to_gl_enum(const Atlas::TextureFilter filter, bool mipmap) {
	switch (filter)
	{
	case Atlas::TextureFilter::LINEAR: return mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
	case Atlas::TextureFilter::NEAREST: return GL_NEAREST;
	}

	CORE_ASSERT(false, "texture_min_filter_to_gl_enum: texture format {} not defined", (uint32_t)filter);
	return 0;
}

GLenum texture_mag_filter_to_gl_enum(const Atlas::TextureFilter filter, bool mipmap) {
	switch (filter)
	{
	case Atlas::TextureFilter::LINEAR: return GL_LINEAR;
	case Atlas::TextureFilter::NEAREST: return GL_NEAREST;
	}

	CORE_ASSERT(false, "texture_min_filter_to_gl_enum: texture format {} not defined", (uint32_t)filter);
	return 0;
}

GLenum buffer_usage_to_gl_enum(const Atlas::BufferUsage usage) {
	switch (usage)
	{
	case Atlas::BufferUsage::STATIC: return GL_STATIC_DRAW;
	case Atlas::BufferUsage::DYNAMIC: return GL_DYNAMIC_DRAW;
	}

	CORE_ASSERT(false, "buffer_usage_to_gl_enum: texture format {} not defined", (uint32_t)usage);
	return 0;
}

GLenum shader_type_to_gl_enum(const Atlas::ShaderType type) {
	switch (type)
	{
	case Atlas::ShaderType::VERTEX: return GL_VERTEX_SHADER;
	case Atlas::ShaderType::FRAGMENT: return GL_FRAGMENT_SHADER;
	case Atlas::ShaderType::COMPUTE: return GL_COMPUTE_SHADER;
	}

	CORE_ASSERT(false, "shader_type_to_gl_enum: shader type {} not defined", (uint32_t)type);
	return 0;
}

std::pair<uint32_t, uint32_t> vertex_attrib_to_gl_enum(const Atlas::VertexAttribute a)
{
	switch (a)
	{
	case Atlas::VertexAttribute::INT:		return { GL_INT, 1 };
	case Atlas::VertexAttribute::INT2:		return { GL_INT, 2 };
	case Atlas::VertexAttribute::INT3:		return { GL_INT, 3 };
	case Atlas::VertexAttribute::INT4:		return { GL_INT, 4 };
	case Atlas::VertexAttribute::UINT:		return { GL_UNSIGNED_INT, 1 };
	case Atlas::VertexAttribute::UINT2:		return { GL_UNSIGNED_INT, 2 };
	case Atlas::VertexAttribute::UINT3:		return { GL_UNSIGNED_INT, 3 };
	case Atlas::VertexAttribute::UINT4:		return { GL_UNSIGNED_INT, 4 };
	case Atlas::VertexAttribute::FLOAT:		return { GL_FLOAT, 1 };
	case Atlas::VertexAttribute::FLOAT2:	return { GL_FLOAT, 2 };
	case Atlas::VertexAttribute::FLOAT3:	return { GL_FLOAT, 3 };
	case Atlas::VertexAttribute::FLOAT4:	return { GL_FLOAT, 4 };
	}

	CORE_ASSERT(false, "vertex_attrib_to_gl_enum: enum not defined!");
	return { 0, 0 };
}

