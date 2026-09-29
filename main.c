#include <ApplicationServices/ApplicationServices.h>
#include <unistd.h>

int main(void)
{
    ProcessSerialNumber psn = {0, kCurrentProcess};
    if (TransformProcessType(&psn,
                             kProcessTransformToBackgroundApplication) != noErr)
        return 1;

    for (;;)
        pause();
}
