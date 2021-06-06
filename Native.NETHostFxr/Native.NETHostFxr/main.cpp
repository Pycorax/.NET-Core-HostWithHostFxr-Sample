#include <iostream>
#include <Windows.h>
#include <shlwapi.h>

#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <cassert>

using string_t = std::basic_string<char_t>;

namespace
{
    // Globals to hold hostfxr exports
    hostfxr_initialize_for_runtime_config_fn init_runtimeCfg_fptr;
    hostfxr_get_runtime_delegate_fn get_delegate_fptr;
    hostfxr_close_fn close_fptr;

    // Handles
    hostfxr_handle cxt = nullptr;

    // Forward declarations
    load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t *assembly);
    bool load_hostfxr();
    void* load_library(const char_t* path);
    void* get_export(void* h, const char* name);
}


int main()
{
    // STEP 0: Get the current executable directory
    string_t runtimePath(MAX_PATH, '\0');
    GetModuleFileNameW(nullptr, runtimePath.data(), MAX_PATH);
    PathRemoveFileSpecW(runtimePath.data()); 
    // Since PathRemoveFileSpecA() removes from data(), the size is not updated, so we must manually update it
    runtimePath.resize(std::wcslen(runtimePath.data())); 

    // STEP 1: Load HostFxr and get exported hosting functions
    if (!load_hostfxr())
    {
        assert(false && "Failure: load_hostfxr()");
        return EXIT_FAILURE;
    }

    // STEP 2: Initialize and start the .NET Core runtime
    const string_t config_path = runtimePath + L"\\Native.NETHostFxr.runtimeconfig.json";
    int rc = init_runtimeCfg_fptr(config_path.c_str(), nullptr, &cxt);
    if (rc != 0 || cxt == nullptr)
    {
        std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
        close_fptr(cxt);
        return EXIT_FAILURE;
    }
    load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = get_dotnet_load_assembly(config_path.c_str());
    assert(load_assembly_and_get_function_pointer != nullptr && "Failure: get_dotnet_load_assembly()");

    // STEP 3: Load managed assembly and get function pointer to a managed method
    const string_t dotnetlib_path = runtimePath + L"\\ManagedLibrary.dll";
    const char_t *dotnet_type = L"ManagedLibrary.ManagedWorker, ManagedLibrary";

    typedef void (CORECLR_DELEGATE_CALLTYPE *custom_entry_point_fn)(void);
    custom_entry_point_fn custom = nullptr;
    rc = load_assembly_and_get_function_pointer
    (
        dotnetlib_path.c_str(),
        dotnet_type,
        L"Print", /* Function Name */
        UNMANAGEDCALLERSONLY_METHOD,
        nullptr,
        (void**)&custom
    );
    assert(rc == 0 && custom != nullptr && "Failure: load_assembly_and_get_function_pointer()");

    // STEP 4: Run managed code
    custom();

    // STEP 5: Shut Down NET Core runtime
    close_fptr(cxt);
}
namespace
{
    bool load_hostfxr()
    {
        // Pre-allocate a large buffer for the path to hostfxr
        char_t buffer[MAX_PATH];
        size_t buffer_size = sizeof(buffer) / sizeof(char_t);
        int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);
        if (rc != 0)
            return false;

        // Load hostfxr and get desired exports
        void* lib = load_library(buffer);
        init_runtimeCfg_fptr = (hostfxr_initialize_for_runtime_config_fn)get_export(lib, "hostfxr_initialize_for_runtime_config");
        get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
        close_fptr = (hostfxr_close_fn)get_export(lib, "hostfxr_close");

        return (init_runtimeCfg_fptr && get_delegate_fptr && close_fptr);
    }
    
    void *load_library(const char_t *path)
    {
        HMODULE h = ::LoadLibraryW(path);
        assert(h != nullptr);
        return (void*)h;
    }

    void *get_export(void *h, const char *name)
    {
        void *f = ::GetProcAddress((HMODULE)h, name);
        assert(f != nullptr);
        return f;
    }

    // Load and initialize .NET Core and get desired function pointer for scenario
    load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t *config_path)
    {
        void *load_assembly_and_get_function_pointer = nullptr;
        

        // Get the load assembly function pointer
        int rc = get_delegate_fptr(
            cxt,
            hdt_load_assembly_and_get_function_pointer,
            &load_assembly_and_get_function_pointer);
        if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
            std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;

        return (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
    }
}