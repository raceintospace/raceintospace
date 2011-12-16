/*!
    @header		SmartCrashReportsInstall
    @abstract   API to the Installer of Smart Crash Reports
    @discussion This API allows you to painlessly install Smart Crash Reports from within your application. It will properly
				detect the versions and whether it should upgrade/install over an older version on the user's computer.
				
				In order to use this library, you have to add the following frameworks to your project:
				<ul>
					<li>/System/Library/Frameworks/Carbon.framework
					<li>/System/Library/Frameworks/CoreFoundation.framework
					<li>/System/Library/Frameworks/Security.framework
					<li>/usr/lib/libz.1.2.3.dylib (or /usr/lib/libz.dylib).
				</ul>
*/

#pragma once
#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
	extern "C" {
#endif

/*!
    @enum		UnsanitySCR_Install_ErrorCode
    @abstract   Result codes returned from the UnsanitySCR_Install() function
    @discussion These error codes will be returned from the UnsanitySCR_Install() function.
    @constant   kUnsanitySCR_Install_NoError			No error. Installation succeeded.
    @constant   kUnsanitySCR_Install_OutOfMemory		Could not allocate needed memory for the installation.
    @constant   kUnsanitySCR_Install_InstalledGlobally	Smart Crash Reports are already installed globally and this API can not install into /Library
														as it would require additional fiddling with the authorization which is beyond the scope of this API.
	
    @constant   kUnsanitySCR_Install_NoPermissions		Could not install because of file permission issues.
	@constant	kUnsanitySCR_Install_AuthFailure		Could not install because user failed to authenticate for the global install.
    @constant   kUnsanitySCR_Install_WillNotInstall		Could not install because a newer or same version of Smart Crash Reports is already installed.
	@constant	kUnsanitySCT_Install_UserCancelled		User hit "Don't Install" in the confirmation UI dialog box.
*/

extern enum
{
	kUnsanitySCR_Install_NoError			= 0,
	kUnsanitySCR_Install_OutOfMemory		= -108,
	kUnsanitySCR_Install_InstalledGlobally	= -13,
	kUnsanitySCR_Install_NoPermissions		= -54,
	kUnsanitySCR_Install_WillNotInstall		= -14,
	kUnsanitySCR_Install_AuthFailure		= -111,
	kUnsanitySCR_Install_UserCancelled		= -15
} UnsanitySCR_Install_ErrorCode;

/*!
    @function	UnsanitySCR_InstalledVersion
    @abstract   Get the version of Smart Crash Reports installed on user's system.
    @discussion Returns the version of Smart Crash Reports installed on user's system.
	@param		outOptionalIsInstalledGlobally	If this is not NULL, the Boolean the param points to will hold TRUE if Smart Crash Reports are installed for all
												users, or FALSE if they are installed for the current user only.
    @result     UInt32 containing the version of Smart Crash Reports (in CFBundleGetVersionNumber() format), or 0 if Smart Crash Reports
				are not installed.
*/
UInt32			UnsanitySCR_InstalledVersion(Boolean* outOptionalIsInstalledGlobally);

/*!
    @function	UnsanitySCR_InstallableVersion
    @abstract   Get the version of Smart Crash Reports that this library can install.
    @discussion Returns the version of Smart Crash Reports that this library can install.
    @result     UInt32 containing the version of Smart Crash Reports (in CFBundleGetVersionNumber() format) bundled in this library.
*/
UInt32			UnsanitySCR_InstallableVersion();

/*!
    @function	UnsanitySCR_CanInstall
    @abstract   Check whether this library can install Smart Crash Reports on user's system.
    @discussion Returns whether Smart Crash Reports can be installed.
	@param		outOptionalAuthenticationWillBeRequired		If this is not NULL, the Boolean the param points to will be set to TRUE if an authentication will be
															required to install Smart Crash Reports.
    @result     TRUE if Smart Crash Reports can be installed, or FALSE if they are already installed (of the same or newer version).
*/
Boolean			UnsanitySCR_CanInstall(Boolean* outOptionalAuthenticationWillBeRequired);

/*!
    @function	UnsanitySCR_Install
    @abstract   Attempt to install Smart Crash Reports on user's system.
    @discussion Presents a dialog asking the user if he/she want to install Smart Crash Reports, along with a link for more information, and Don't Show Again checkbox,
				and, if confirmed by the user, installs Smart Crash Reports from the library's archive, if possible. The confirmation dialog will block the host application
				until the user makes the choice.
	@param		inInstallFlags			A bit mask containing the installation options. Please see the constants UnsanitySCR_Install_Flags for more information on
										individual flags.
	@result     One of the <code>UnsanitySCR_Install_ErrorCode</code> result codes.
*/
OSStatus		UnsanitySCR_Install(UInt32 inInstallFlags);

/*!
    @enum		UnsanitySCR_Install_Flags
    @abstract   Flags that can be used in the UnsanitySCR_Install() function
    @discussion These flags can be passed to the UnsanitySCR_Install() function, bitwise OR'ed, if needed.
    @constant   kUnsanitySCR_GlobalInstall			If set, a global installation will be attempted (provided the user can authenticate). If not set, Smart Crash Reports will
													be installed for current user only.
	@constant	kUnsanitySCR_DoNotPresentInstallUI	If set, the confirmation dialog shown to the user will not be used and the silent installation will be attempted.
													If not set, the confirmation dialog will be shown to the user asking whether he/she wants to install Smart Crash Reports or not.
													The confirmation dialog will block the application until dismissed.
*/

extern enum
{
	kUnsanitySCR_GlobalInstall			= (1L << 1),
	kUnsanitySCR_DoNotPresentInstallUI	= (1L << 2)
} UnsanitySCR_Install_Flags;
#ifdef __cplusplus
	}
#endif