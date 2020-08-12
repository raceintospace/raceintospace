/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
*/

#import "SDL.h"
#import "SDLMain.h"
#import <sys/param.h> /* for MAXPATHLEN */
#import <unistd.h>

#import "SmartCrashReportsInstall.h"

/* Portions of CPS.h */
typedef struct CPSProcessSerNum
{
	UInt32		lo;
	UInt32		hi;
} CPSProcessSerNum;

extern OSErr	CPSGetCurrentProcess( CPSProcessSerNum *psn);
extern OSErr 	CPSEnableForegroundOperation( CPSProcessSerNum *psn, UInt32 _arg2, UInt32 _arg3, UInt32 _arg4, UInt32 _arg5);
extern OSErr	CPSSetFrontProcess( CPSProcessSerNum *psn);


static int    gArgc;
static char  **gArgv;
static BOOL   gFinderLaunch;
static BOOL   gCalledAppMainline = FALSE;

@interface SDLApplication : NSApplication
@end

@implementation SDLApplication
/* Invoked from the Quit menu item */
- (void)terminate:(id)sender
{
    /* Post a SDL_QUIT event */
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}
@end

/* The main class of the application, the application's delegate */
@implementation SDLMain

- (IBAction)prefsMenu:(id)sender
{
	/* TODO */
}

- (IBAction)help:(id)sender
{
	/* TODO */
}

- (IBAction)enableCrashReporting:(id)sender
{
	if (UnsanitySCR_InstalledVersion(NULL) < UnsanitySCR_InstallableVersion()) {
		BOOL retry;
		do {
			retry = FALSE;
			
			switch (UnsanitySCR_Install(0)) {
				case kUnsanitySCR_Install_NoError:
					NSRunInformationalAlertPanel(@"Crash Reporting Enabled", @"The crash reporting tool was installed successfully. If Race Into Space crashes, please click the Report... button to submit a detailed crash report.", @"Great!", nil, nil);
					break;
					
				case kUnsanitySCR_Install_InstalledGlobally:
					NSRunCriticalAlertPanel(@"Crash Reporting Problem", @"An earlier version of SmartCrashReports was installed into /Library manually or by another application. Please upgrade this by hand, by downloading the latest version from http://www.smartcrashreports.com/ and running the installer.", @"All right", nil, nil);
					break;
					
				case kUnsanitySCR_Install_UserCancelled:
					// The user cancelled in a dialog already, so we can silently eat this without further bothering them
					break;
					
				case kUnsanitySCR_Install_NoPermissions:
					NSRunAlertPanel(@"Crash Reporting Problem", @"The crash reporter installer failed because of a permissions error, but was not any more specific. If it happens persistently, please contact the Race Into Space developers.", @"All right", nil, nil);
					
				case kUnsanitySCR_Install_AuthFailure:
					if (NSRunAlertPanel(@"Crash Reporting Problem", @"Enabling crash reporting requires you to authenticate yourself.", @"Authenticate again", @"Give up", nil) == 1)
						retry = TRUE;
					else
						retry = FALSE;
					break;
					
				case kUnsanitySCR_Install_WillNotInstall:
				case kUnsanitySCR_Install_OutOfMemory:
				default:
					NSRunCriticalAlertPanel(@"Crash Reporting Problem", @"Something unusual happened that prevented installing the crash reporter. If it happens persistently, please contact the Race Into Space developers.", @"All right", nil, nil);
					break;
			}			
		} while (retry);
	} else {
		NSRunInformationalAlertPanel(@"Crash Reporting Enabled", @"The crash reporting tool is installed. If Race Into Space crashes, please click the Report... button to submit a detailed crash report.", @"Great!", nil, nil);
	}
}

/* Set the working directory to the .app directory */
- (void) setupWorkingDirectory
{
	char appdir[MAXPATHLEN];
	CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
	if (CFURLGetFileSystemRepresentation(url, true, (UInt8 *)appdir, MAXPATHLEN)) {
		chdir(appdir);
	}
	CFRelease(url);
}

/*
 * Catch document open requests...this lets us notice files when the app
 *  was launched by double-clicking a document, or when a document was
 *  dragged/dropped on the app's icon. You need to have a
 *  CFBundleDocumentsType section in your Info.plist to get this message,
 *  apparently.
 *
 * Files are added to gArgv, so to the app, they'll look like command line
 *  arguments. Previously, apps launched from the finder had nothing but
 *  an argv[0].
 *
 * This message may be received multiple times to open several docs on launch.
 *
 * This message is ignored once the app's mainline has been called.
 */
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
    const char *temparg;
    size_t arglen;
    char *arg;
    char **newargv;

    if (!gFinderLaunch)  /* MacOS is passing command line args. */
        return FALSE;

    if (gCalledAppMainline)  /* app has started, ignore this document. */
        return FALSE;

    temparg = [filename UTF8String];
    arglen = SDL_strlen(temparg) + 1;
    arg = (char *) SDL_malloc(arglen);
    if (arg == NULL)
        return FALSE;

    newargv = (char **) realloc(gArgv, sizeof (char *) * (gArgc + 2));
    if (newargv == NULL)
    {
        SDL_free(arg);
        return FALSE;
    }
    gArgv = newargv;

    SDL_strlcpy(arg, temparg, arglen);
    gArgv[gArgc++] = arg;
    gArgv[gArgc] = NULL;
    return TRUE;
}


/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
    int status;

    [self setupWorkingDirectory];

    gCalledAppMainline = TRUE;
    status = SDL_main(gArgc, gArgv);

    exit(status);
}
@end

#ifdef main
#  undef main
#endif

/* Main entry point to executable - should *not* be SDL_main! */
int main (int argc,  const char *argv[])
{
    /* Copy the arguments into a global variable */
    /* This is passed if we are launched by double-clicking */
    if ( argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 ) {
        gArgv = (char **) SDL_malloc(sizeof (char *) * 2);
        gArgv[0] = (char *)argv[0];
        gArgv[1] = NULL;
        gArgc = 1;
        gFinderLaunch = YES;
    } else {
        int i;
        gArgc = argc;
        gArgv = (char **) SDL_malloc(sizeof (char *) * (argc+1));
        for (i = 0; i <= argc; i++)
            gArgv[i] = (char *)argv[i];
        gFinderLaunch = NO;
    }

    [SDLApplication poseAsClass:[NSApplication class]];
    NSApplicationMain (argc, argv);

    return 0;
}
