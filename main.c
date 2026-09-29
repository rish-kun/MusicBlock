#include <ApplicationServices/ApplicationServices.h>
#include <unistd.h>

int main(void)
{
    ProcessSerialNumber psn;
    if (GetCurrentProcess(&psn) != noErr)
        return 1;

    for (;;)
        pause();
}
