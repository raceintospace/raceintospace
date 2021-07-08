#import <Cocoa/Cocoa.h>

void ugly_crash(const char * title, const char * message)
{
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:[NSString stringWithCString:title encoding:NSUTF8StringEncoding]];
    [alert setInformativeText:[NSString stringWithCString:message encoding:NSUTF8StringEncoding]];
    [alert runModal];
    [alert release];

    exit(1);
}
