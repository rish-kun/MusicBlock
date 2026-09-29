#include <CoreFoundation/CoreFoundation.h>

static void idle_source(void *info)
{
    (void)info;
}

int main(void)
{
    CFRunLoopSourceContext context = {0};
    context.perform = idle_source;

    CFRunLoopSourceRef source = CFRunLoopSourceCreate(kCFAllocatorDefault, 0,
                                                    &context);
    if (source == NULL)
        return 1;

    CFRunLoopAddSource(CFRunLoopGetMain(), source, kCFRunLoopDefaultMode);
    CFRelease(source);
    CFRunLoopRun();
    return 0;
}
