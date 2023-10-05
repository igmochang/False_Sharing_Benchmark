#include <iostream>
#include <cpuid.h>

int main() {
    uint32_t eax, ebx, ecx, edx;
    
    // Get the maximum supported input value for CPUID
    eax = 0;
    __cpuid(0, eax, ebx, ecx, edx);
    int max_supported_value = eax;

    // Get cache parameters
    for (int i = 0; i <= max_supported_value; ++i) {
        eax = 4; // Cache parameters are obtained using CPUID function 4
        ecx = i; // Cache level index
        __cpuid_count(eax, ecx, eax, ebx, ecx, edx);

        if (eax == 0) {
            break; // No more caches
        }

        // Cache level
        int cache_level = (eax >> 5) & 7;

        // Cache type
        int cache_type = (eax >> 0) & 31;

        // Cache size and associativity
        int cache_size = (ebx & 0xffff) * 1024; // Size is in KB
        int cache_associativity = ((ebx >> 22) & 0x3ff) + 1;

        // Cache line size
        int cache_line_size = (ecx & 0xfff) + 1;

        std::cout << "Cache Level " << cache_level << " (Type " << cache_type << "):" << std::endl;
        std::cout << "Size: " << cache_size << " KB" << std::endl;
        std::cout << "Associativity: " << cache_associativity << " way(s)" << std::endl;
        std::cout << "Line Size: " << cache_line_size << " bytes" << std::endl;
        std::cout << std::endl;
    }

    // Get the number of cores and threads
    eax = 1;
    __cpuid(eax, ebx, ecx, edx);
    int num_cores = (ebx >> 16) & 0xFF;
    int num_threads = (ecx >> 16) & 0xFF;

    std::cout << "Number of Cores: " << num_cores << std::endl;
    std::cout << "Number of Threads: " << num_threads << std::endl;

    return 0;
}
