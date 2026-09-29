#include <ApplicationServices/ApplicationServices.h>
#include <dlfcn.h>
#include <unistd.h>

typedef OSErr (*get_current_process_fn)(ProcessSerialNumber *);

int main(void)
{
    void *framework = dlopen(
        "/System/Library/Frameworks/ApplicationServices.framework/"
        "Versions/A/Frameworks/HIServices.framework/Versions/A/HIServices",
        RTLD_NOW | RTLD_LOCAL);
    if (framework == NULL)
        return 1;

    get_current_process_fn check_in =
        (get_current_process_fn)dlsym(framework, "GetCurrentProcess");
    ProcessSerialNumber psn;
    if (check_in == NULL || check_in(&psn) != noErr) {
        dlclose(framework);
        return 1;
    }

    dlclose(framework);
    for (;;)
        pause();
}
