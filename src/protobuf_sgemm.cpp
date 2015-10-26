#include "kernel_binary.pb.h"
#include <iostream>
#include <fstream>
#include "CL/cl.h"
#include "opencl_helper.h"

const char BUILD_OPTIONS[] = "-cl-std=CL2.0";
const char KERNEL_SOURCE[] = "C:\\Users\\timmy\\Documents\\SVN\\sgemm_kernels\\col_NT\\sgemm_gcn_96_fence.cl";

int main(int argc, char *argv[])
{

    if (argc != 3) {
        std::cerr << "Usage:  " << argv[0] << " kernel_binary_files_path" << " 1 write or 2 read" << std::endl;
        return -1;
    }
    char *source;
    std::string kb_path = argv[1];
    KERNEL_BINARY thisKernelBinary;
    KERNEL_BINARY_HAWAII *hawaiiKernelBinary = NULL;
    char platformName[64];
    char device_name[64];
    char cl_version_name[64];
    int err;
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_kernel kernel;
    cl_event event;
    cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
    char **kernelBinary = new char*[1];
    size_t kernelBinarySize;

    if (atoi(argv[2]) == 1)
    {
        {
            // Read the existing address book.
            std::string system_info = kb_path + "\\system_info.kb";
            std::fstream input_system_info(system_info, std::ios::in | std::ios::binary);
            if (!input_system_info) {
                std::cout << argv[1] << "\\system_info.kb File not found.  Creating a new file." << std::endl;
            }
            else if (!thisKernelBinary.ParseFromIstream(&input_system_info)) {
                std::cerr << "Failed to parse system_info.kb." << std::endl;
                return -1;
            }
        }

        platform = getPlatformname(platformName, 64);
        std::cout << "platform name is " << platformName << std::endl;
        // need to write platform name to a file

        device = getDevice(platform, device_name, 64);
        std::cout << "device name is " << device_name << std::endl;
        // need to write device name to a file

        std::string current_device;
        thisKernelBinary.set_asic_name(device_name);

        std::cout << "thisKernelBinary is for " << thisKernelBinary.asic_name() << std::endl;

        if (thisKernelBinary.asic_name() == "Hawaii")
        {
            hawaiiKernelBinary = new KERNEL_BINARY_HAWAII();
            {
                // Read the existing address book.
                std::string hawaii_kb = kb_path + "\\hawaii.kb";
                std::fstream input_hawaii_kb(hawaii_kb, std::ios::in | std::ios::binary);
                if (!input_hawaii_kb) {
                    std::cout << argv[1] << "\\hawaii.kb File not found.  Creating a new file." << std::endl;
                }
                else if (!hawaiiKernelBinary->ParseFromIstream(&input_hawaii_kb)) {
                    std::cerr << "Failed to parse hawaii.kb." << std::endl;
                    return -1;
                }
            }
        }
        else
        {
            return -1;
        }
        hawaiiKernelBinary->set_num_kernels(0);
        err = getDriverVersion(device, cl_version_name, 64);
        std::cout << "CL version number is " << cl_version_name << std::endl;
        hawaiiKernelBinary->set_driver_name(cl_version_name);
        hawaiiKernelBinary->set_cl_build_options(BUILD_OPTIONS);
        // need to write this version number to a file
        props[1] = (cl_context_properties)platform;
        context = clCreateContext(props, 1, &device, NULL, NULL, &err);
        assert(context != NULL);
        queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
        assert(queue != NULL);
        source = loadFile(KERNEL_SOURCE);
        assert(source != NULL);
        err = getKernelBinaryFromSource(context, source, hawaiiKernelBinary->cl_build_options().c_str(), kernelBinary, &kernelBinarySize);
        std::cout << "kernel binary looks like " << kernelBinary[0] <<std::endl;
        hawaiiKernelBinary->set_num_kernels(hawaiiKernelBinary->num_kernels()+ 1);
        hawaiiKernelBinary->add_kernel_binary(kernelBinary[0]);
        

        {
            // Write the kernel back to disk.
            std::string system_info = kb_path + "\\system_info.kb";
            std::fstream output_system_info(system_info, std::ios::out | std::ios::trunc | std::ios::binary);
            if (!thisKernelBinary.SerializeToOstream(&output_system_info)) {
                std::cerr << "Failed to write kernel binary." << std::endl;
                return -1;
            }
            std::string hawaii_kb = kb_path + "\\hawaii.kb";
            std::fstream output_hawaii_kb(hawaii_kb, std::ios::out | std::ios::trunc | std::ios::binary);
            if (!hawaiiKernelBinary->SerializeToOstream(&output_hawaii_kb)) {
                std::cerr << "Failed to write haiwii.kb." << std::endl;
                return -1;
            }
        }
        
    }


    if (atoi(argv[2]) == 2)
    {
        {
            // Read the existing system_info
            std::string system_info = kb_path + "\\system_info.kb";
            std::fstream input_system_info(system_info, std::ios::in | std::ios::binary);
            if (!thisKernelBinary.ParseFromIstream(&input_system_info)) {
                std::cerr << "Failed to parse system info file." << std::endl;
                return -1;
            }
        }
        std::string asic_name = thisKernelBinary.asic_name();
        std::cout << "loaded kernel file asic is " << asic_name << std::endl;
        if (thisKernelBinary.asic_name() == "Hawaii")
        {
            hawaiiKernelBinary = new KERNEL_BINARY_HAWAII();
            {
                // Read the existing address book.
                std::string hawaii_kb = kb_path + "\\hawaii.kb";
                std::fstream input_hawaii_kb(hawaii_kb, std::ios::in | std::ios::binary);
                if (!hawaiiKernelBinary->ParseFromIstream(&input_hawaii_kb)) {
                    std::cerr << "Failed to parse hawaii file." << std::endl;
                    return -1;
                }
            }

        }
        else
        {
            return -1;
        }
        std::string driver_name = hawaiiKernelBinary->driver_name();
        std::string cl_build_options = hawaiiKernelBinary->cl_build_options();

        std::cout << "hawaii driver is " << driver_name << std::endl;
        std::cout << "hawaii build options are " << cl_build_options << std::endl;

        int num_kernels = hawaiiKernelBinary->num_kernels();
        std::cout << "there were " << num_kernels << " kernel(s) compiled" <<std::endl;
        std::string loaded_kernel_binary = hawaiiKernelBinary->kernel_binary(0);
        std::cout << "kernel binary looks like " << loaded_kernel_binary << std::endl;

    }

   


    if (thisKernelBinary.asic_name() == "Hawaii")
    {
        delete hawaiiKernelBinary;
    }
}