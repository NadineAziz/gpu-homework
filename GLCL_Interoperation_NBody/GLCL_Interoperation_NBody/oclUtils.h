#pragma once

// All OpenCL headers
#if defined (__APPLE__) || defined(MACOSX)
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif 

const char* oclChannelOrderString(cl_channel_order order)
{
	switch (order)
	{
	case CL_R        : return "CL_R";
	case CL_A        : return "CL_A";
	case CL_RG       : return "CL_RG";
	case CL_RA       : return "CL_RA";
	case CL_RGB      : return "CL_RGB";
	case CL_RGBA     : return "CL_RGBA";
	case CL_BGRA     : return "CL_BGRA";
	case CL_ARGB     : return "CL_ARGB";
	case CL_INTENSITY: return "CL_INTENSITY";
	case CL_LUMINANCE: return "CL_LUMINANCE";
	case CL_Rx       : return "CL_Rx";
	case CL_RGx      : return "CL_RGx";
	case CL_RGBx     : return "CL_RGBx";
	default:
		return "error!";
	}
}

const char* oclChannelTypeString(cl_channel_type type)
{
	switch (type)
	{
	case CL_SNORM_INT8      : return "CL_SNORM_INT8";
	case CL_SNORM_INT16     : return "CL_SNORM_INT16";
	case CL_UNORM_INT8      : return "CL_UNORM_INT8";
	case CL_UNORM_INT16     : return "CL_UNORM_INT16";
	case CL_UNORM_SHORT_565 : return "CL_UNORM_SHORT_565";
	case CL_UNORM_SHORT_555 : return "CL_UNORM_SHORT_555";
	case CL_UNORM_INT_101010: return "CL_UNORM_INT_101010";
	case CL_SIGNED_INT8     : return "CL_SIGNED_INT8";
	case CL_SIGNED_INT16    : return "CL_SIGNED_INT16";
	case CL_SIGNED_INT32    : return "CL_SIGNED_INT32";
	case CL_UNSIGNED_INT8   : return "CL_UNSIGNED_INT8";
	case CL_UNSIGNED_INT16  : return "CL_UNSIGNED_INT16";
	case CL_UNSIGNED_INT32  : return "CL_UNSIGNED_INT32";
	case CL_HALF_FLOAT      : return "CL_HALF_FLOAT";
	case CL_FLOAT           : return "CL_FLOAT";
	default:
		return "error!";
	}
}

const char* oclDeviceTypeString(cl_int devType)
{
	switch (devType)
	{
	case CL_DEVICE_TYPE_DEFAULT: return "CL_DEVICE_TYPE_DEFAULT";
	case CL_DEVICE_TYPE_CPU: return "CL_DEVICE_TYPE_CPU";
	case CL_DEVICE_TYPE_GPU: return "CL_DEVICE_TYPE_GPU";
	case CL_DEVICE_TYPE_ACCELERATOR: return "CL_DEVICE_TYPE_ACCELERATOR";
	case CL_DEVICE_TYPE_ALL: return "CL_DEVICE_TYPE_ALL";
	default:
		return "error!";
	}       
}

// Helper function to get OpenCL error string from constant
// *********************************************************************
const char* oclErrorString(cl_int error)
{
	static const char* errorString[] = {
		"CL_SUCCESS",
		"CL_DEVICE_NOT_FOUND",
		"CL_DEVICE_NOT_AVAILABLE",
		"CL_COMPILER_NOT_AVAILABLE",
		"CL_MEM_OBJECT_ALLOCATION_FAILURE",
		"CL_OUT_OF_RESOURCES",
		"CL_OUT_OF_HOST_MEMORY",
		"CL_PROFILING_INFO_NOT_AVAILABLE",
		"CL_MEM_COPY_OVERLAP",
		"CL_IMAGE_FORMAT_MISMATCH",
		"CL_IMAGE_FORMAT_NOT_SUPPORTED",
		"CL_BUILD_PROGRAM_FAILURE",
		"CL_MAP_FAILURE",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"CL_INVALID_VALUE",
		"CL_INVALID_DEVICE_TYPE",
		"CL_INVALID_PLATFORM",
		"CL_INVALID_DEVICE",
		"CL_INVALID_CONTEXT",
		"CL_INVALID_QUEUE_PROPERTIES",
		"CL_INVALID_COMMAND_QUEUE",
		"CL_INVALID_HOST_PTR",
		"CL_INVALID_MEM_OBJECT",
		"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
		"CL_INVALID_IMAGE_SIZE",
		"CL_INVALID_SAMPLER",
		"CL_INVALID_BINARY",
		"CL_INVALID_BUILD_OPTIONS",
		"CL_INVALID_PROGRAM",
		"CL_INVALID_PROGRAM_EXECUTABLE",
		"CL_INVALID_KERNEL_NAME",
		"CL_INVALID_KERNEL_DEFINITION",
		"CL_INVALID_KERNEL",
		"CL_INVALID_ARG_INDEX",
		"CL_INVALID_ARG_VALUE",
		"CL_INVALID_ARG_SIZE",
		"CL_INVALID_KERNEL_ARGS",
		"CL_INVALID_WORK_DIMENSION",
		"CL_INVALID_WORK_GROUP_SIZE",
		"CL_INVALID_WORK_ITEM_SIZE",
		"CL_INVALID_GLOBAL_OFFSET",
		"CL_INVALID_EVENT_WAIT_LIST",
		"CL_INVALID_EVENT",
		"CL_INVALID_OPERATION",
		"CL_INVALID_GL_OBJECT",
		"CL_INVALID_BUFFER_SIZE",
		"CL_INVALID_MIP_LEVEL",
		"CL_INVALID_GLOBAL_WORK_SIZE",
	};

	const int errorCount = sizeof(errorString) / sizeof(errorString[0]);

	const int index = -error;

	return (index >= 0 && index < errorCount) ? errorString[index] : "Unspecified Error";
}