#ifndef OPENCL_HELPER
#define OPENCL_HELPER

#include "CL/cl.h"

#ifdef __cplusplus
extern "C"{
#endif

cl_platform_id getPlatformname(char *name, int name_size);
cl_device_id getDevice(cl_platform_id platform, char *name, int name_size);
int getDriverVersion(cl_device_id device, char* version_name, int version_name_size);
char* loadFile(const char* path);
cl_int getKernelBinaryFromSource(cl_context context,
                                 const char *source,
                                 const char *buildOptions,
                                 char **binary,
                                 size_t *binarySize);


#ifdef __cplusplus
}
#endif
#endif;