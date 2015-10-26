#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "opencl_helper.h"

#define CL_CHECK(STATUS) \
  if(STATUS != CL_SUCCESS) { \
    printf("OpenCL error %i on line %u\n", STATUS, __LINE__); \
    assert(false); \
      }

cl_platform_id
getPlatformname(char *name, int name_size)
{
    //get the first platform's name

    cl_uint err, nrPlatforms;
    cl_platform_id *list, platform;
    char platformName[64];
    err = clGetPlatformIDs(0, NULL, &nrPlatforms);
    if (err != CL_SUCCESS) {
        return NULL;
    }

    list = (cl_platform_id*)calloc(nrPlatforms, sizeof(*list));
    if (list == NULL) {
        return NULL;
    }

    err = clGetPlatformIDs(nrPlatforms, list, NULL);
    if (err != CL_SUCCESS) {
        free(list);
        return NULL;
    }
    err = clGetPlatformInfo(list[0], CL_PLATFORM_NAME,
        name_size, name, NULL);
    if (err != CL_SUCCESS) {
        free(list);
        return NULL;
    }
    platform = list[0];
    free(list);
    return platform;

}

cl_device_id
getDevice(cl_platform_id platform, char *name, int name_size)
{

    cl_int err;
    cl_uint nrDevices;
    cl_device_id *list, device;
    //char deviceName[64];

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &nrDevices);
    if (err != CL_SUCCESS) {
        return NULL;
    }
    list = (cl_device_id*)calloc(nrDevices, sizeof(*list));
    if (list == NULL) {
        return NULL;
    }

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, nrDevices, list, NULL);
    if (err != CL_SUCCESS) {
        free(list);
        return NULL;
    }

    device = NULL;
    err = clGetDeviceInfo(list[0], CL_DEVICE_NAME,
            name_size, name, NULL);

    device = list[0];
    free(list);
    return device;
}

int getDriverVersion(cl_device_id device, char* version_name, int version_name_size)
{
    cl_int err;
    err = clGetDeviceInfo(device, CL_DRIVER_VERSION, version_name_size, version_name, NULL);
    return err;
}

char*
loadFile(const char* path)
{
    FILE *f;
    long size;
    char *text;

    f = fopen(path, "rb");
    if (f == NULL) {
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    size = ftell(f);
    if (size == -1) {
        fclose(f);
        return NULL;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }

    text = (char*)calloc(size + 1, 1);
    if (text == NULL) {
        fclose(f);
        return NULL;
    }

    if (fread(text, 1, size, f) == 0) {
        free(text);
        fclose(f);
        return NULL;
    }
    fclose(f);
    return text;
}

cl_int getKernelBinaryFromSource(cl_context context,
    const char *source,
    const char *buildOptions,
    char **binary,
    size_t *binarySize)
{
    cl_int status = CL_SUCCESS;

    // create program
    cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, &status);
    CL_CHECK(status);

    cl_uint numDevicesInContext;
    status = clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint), &numDevicesInContext, NULL);
    CL_CHECK(status);

    // get devices
    cl_device_id* devices = (cl_device_id*)malloc( sizeof(cl_device_id) *numDevicesInContext);
    clGetContextInfo(context, CL_CONTEXT_DEVICES, numDevicesInContext*sizeof(cl_device_id), devices, NULL);
    CL_CHECK(status);

    // choose device 0
    cl_device_id device = devices[0];
    free(devices);

    // build program for device
    status = clBuildProgram(program, 1, &device, buildOptions, NULL, NULL);


    // print build failure
    if (status != CL_SUCCESS) {
        printf("clBuildProgram Failed\n");
        printf("status = %d\n", status);

        size_t len = 0;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
        char* buildLog = (char*)malloc(len*sizeof(char)); 

        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, len*sizeof(char), buildLog, 0);
        printf("\nBuild Log:\n\n");
        printf("%s\n", buildLog);
        printf("\n\nKernel String:\n\n");
        printf("%s\n", source);
        free(buildLog);
        binary[0] = 0;
        *binarySize = 0;
        return status;
    }


    // get binary from program
    status = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), binarySize, NULL);

    binary[0] = (char*)malloc((*binarySize)*sizeof(char));

    status = clGetProgramInfo(program, CL_PROGRAM_BINARIES, 8 /*?*/, binary, NULL);
    CL_CHECK(status);

    return CL_SUCCESS;
}