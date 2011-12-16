/*!
    @header		SmartCrashReportsAPI
    @abstract   Contains API definitions to use with Smart Crash Reports.
    @discussion Smart Crash Reports is an enhancement for the Apple's CrashReporter application introduced in Mac OS X 10.4. It allows 3rd party developers to
				register their own match specifiers, and if the crash log the user is about to submit contains the match specifier, the crash log will be
				sent to the developer as well as Apple. This greatly enhances the user experience of the OS, and allows developers to receive crashes and improve
				their software in a timely manner.
				
				<p>Smart Crash Reports requires no Application Enhancer or similar "patching" frameworks users have to install; they operate on the InputManager
				mechanism that built-in Mac OS X.

				<p>Smart Crash Reports is completely free to use for both users and developers; <a href="http://www.unsanity.com/">Unsanity</a> is providing it as a
				service to the community being certain it will benefit all the users on the platform.
				
				<p><b>How does Smart Crash Reports work?</b>
				<p>Whenever an application crashes on user's computer and they click on the Report... button, Smart Crash Reports analyzes the crash log that is about to be submitted
				to Apple. If the crash log is determined to be related to a developer that registered the application or product (Contextual Menu Module, InputManager, or any other type of
				system plugin), then the user is given an option to submit the bug report to both that developer and Apple, thus giving the developer the chance to fix the bug in the crashed
				product in a timely manner.
				<p>For developers, there is two ways to make Smart Crash Reports recognize your product. First, and probably the simplest one, is to add a couple of new keys into the
				application's Info.plist file that instruct Smart Crash Reports what to do in case that application crashes. However, this method only works for stand-alone applications and
				not plugins.
				<p>Second method involves using of the supplied API calls to register a Match Specifier with the system. A match specifier is a simple string (no wildcards or regexp) that contains full
				or partial name of developer product(s) CFBundleIdentifier (for example, "com.somecompany."). When an application crashes on user's computer,
				Smart Crash Reports system analyzes the crashed thread of the crash log to determine if it contains any
				of the match specifiers registered with it. If so, the crash is likely related to the developer who registered the match specifier, so Smart Crash Reports takes the
				appropriate action from there on.
				
				<p><b>How Developer Receives The Crash Reports</b>
				<p>Developer can receive the crash reports that Smart Crash Reports detected as belonging to them in either of two ways:
				<ul>
					<li>By E-Mail to the address provided during the match specifier registration (Unsanity server will handle the mailing for you)</li>
					<li>By a custom CGI script on your web server that processes CrashReporter logs</li>
				</ul>

				<p><b>So, How to Make My Application Work With Smart Crash Reports so I Receive Crash Logs?</b>
				<p>If you develop an application and would like to simply receive all the crash logs related to it, do this:
				<ol>
					<li>Obtain an E-mail Ticket ID for the E-mail you wish to receive reports at by going to the following URL: <a href="http://www.unsanity.com/goto/email-ticket/">http://www.unsanity.com/goto/email-ticket/</a>.
					<li>Add the following keys to your application Info.plist:<br/>
						<code>&lt;key&gt;SmartCrashReports_CompanyName&lt;/key&gt;<br/>
						&lt;string&gt;Your Company Name&lt;/string&gt;<br/>
						&lt;key&gt;SmartCrashReports_EmailTicket&lt;/key&gt;<br/>
						&lt;string&gt;SCR-XXXXXXXX&lt;/string&gt;</code><br/>
						Replace "Your Company Name" with a readable (short) company name, and "SCR-XXXXXXXX" with the Email Ticket ID you will receive in the E-mail after performing Step 1.<br/>
					<li>You're all done! If you'd like to use your own CGI to process the crash logs, replace <code>SmartCrashReports_EmailTicket</code> key with the <code>SmartCrashReports_URL</code> key.
					Now your application is successfully registered with the Smart Crash Reports system and you will receive crash logs for it in the mail.
				</ol>
				
				<p>If you are developing a plugin, InputManager, and whatnot, you have to use the API provided to register your Match Specifier:
				<ol>
					<li><a href="http://www.unsanity.com/smartcrashreports/">Download</a> the Smart Crash Reports SDK. It will contain two files (among others), <code>SmartCrashReportsAPI.o</code> and <code>SmartCrashReportsAPI.h</code>. Add them to your project.
					<li>Obtain an E-mail Ticket ID for the E-mail you wish to receive reports at by going to the following URL: <a href="http://www.unsanity.com/goto/email-ticket/">http://www.unsanity.com/goto/email-ticket/</a>.
					<li>Somewhere in your application, or plugin, add the following line:<br/>
						<code>UnsanitySCR_RegisterMatchSpecifier(NULL, CFSTR("Your Company Name"), NULL, CFSTR("SCR-XXXXXXXX"), NULL);</code><br/>
						Replace "Your Company Name" with a readable (short) company name, and "SCR-XXXXXXXX" with the Email Ticket ID you will receive in the E-mail after performing Step 2.<br/>
						<i>Note</i>: The best way to call this is after your application has finished launching, such as applicationDidFinishLaunching: in the NSApp delegate for Cocoa apps, or
						whatever for any other types of projects.
					<li>There's no Step 4! Now your product is successfully registered with the Smart Crash Reports system and you will receive crash logs containing the match specifier in the crashed thread in the mail.
				</ol>
				<p><i>Notes:</i> It is not necessary to register with Smart Crash Reports on every application launch; once you register, it is permanent and will exist until user deletes the
				SCR database somehow. However, it takes little speed/memory overhead so you can safely call it on every launch to avoid doing 'is registered already' checks and whatnot.
				
				<p><b>How to Make a CGI to Accept Crash Reports On Your Site?</b>
				<p>If you wish to receive crash reports on your Web site, you can implement the custom Submit URL script that user's CrashReporter.app will contact directly. However,
				keep in mind that the following criteria should be met:
				<ul>
					<li>Your web server script has to be in the same place over next several years, at the very least, to allow delivery of crash reports to you.
					<li>You are responsible for adjusting the web script if the format CrashReporter.app sends crash reports in changes.
					<li>Your web server should be accessible 24/7 and be on a reasonably fast connection to not delay user's submission process.
				</ul>
				<p>For the above reasons, it is recommended to <a href="http://www.unsanity.com/goto/email-ticket/">obtain the E-mail ticket</a> from Unsanity and use the E-mail feature
				of Smart Crash Reports. Unsanity will make sure the server is accessible, knows how to parse all formats of submissions, and will safely redirect the crash reports to the
				email you provide.
				<p>If your own Web script is still the way you want to go, here is the list of keys currently known to be passed to the script by the CrashReporter.app in a POST form data:
				<ul>
					<li><code>url_from</code>: contains the subject of the crash report, usually "<i>&lt;Some Application&gt; crash</i>"
					<li><code>feedback_comments</code>: user comments they entered in the additional info fields
					<li><code>bug_type</code>: unknown constant, probably determines which POST elements are available; presently 9.
					<li><code>app_name</code>: full crashed application name
					<li><code>app_version</code>: version of the crashed application
					<li><code>os_version</code>: Mac OS X version, in form 10.X.X:BUILD (for example, "<i>10.4.2:8C46</i>")
					<li><code>machine_config</code>: Computer configuration string, for example, "<i>PowerMac7,2 (2048 MB)</i>"
					<li><code>page_source</code>: Contains BASE64-encoded crash report
					<li><code>system_profile</code>: Contains BASE64-encoded property list with a brief system profile of the user's system.
				</ul>
				<p>Please note that these form keys are subject to change by Apple, so it is your responsibility to adjust the scripts on your Web servers if such thing happens.
				
				<p><b>What Are The Licensing Terms for Smart Crash Reports and its SDK?</b>
				<p>You are free to use and distribute Smart Crash Reports enabled products freely in any form without prior permission from Unsanity, LLC. If you like the system and
				would like to return the favor Unsanity is doing to the developer community, link to Smart Crash Reports somewhere on your SCR-enabled product page:<br>
				<code>This product uses &lt;a href="http://www.unsanity.com/smartcrashreports/"&gt;Smart Crash Reports&lt;a&gt; by &lt;a href="http://www.unsanity.com/"&gt;Unsanity&lt;a&gt;.</code>
*/

#pragma once
#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
	extern "C" {
#endif

/*!
    @function	UnsanitySCR_RegisterMatchSpecifier
	@abstract	Registers your match specifier with the Smart Crash Reports system.
    @discussion	Register your match specifier with the Smart Crash Reports system. Once this call is done, crash reports containing inMatchString in the crashed thread
				will be submitted to you, either to the submission URL (you have to implement the script on your web server), or to the submission email (Unsanity server will
				forward the report to you).
    @param      inMatchString						The product ID string to match or NULL to use the current bundle identifier. Normally, this is a part of a CFBundleIdentifier of your product.
													For example, if your product CFBundleIdentifier is "com.mycompany.myapp", it is a good idea to set inProductIDMatchString
													to "com.mycompany.", since it will catch all of the products of your company. If you pass NULL to this argument, an attempt to use
													CFBundleIdentifier from the main bundle will be used, which is handy for most applications out there, but not acceptable for Contextual
													Menu Modules, Preference Panes and other plugins.
	@param		inCompanyName						The company name in user-readable format. Smart Crash Reports will substitute "Apple" with this string, so make sure it's
													not too long. Example: "My Company"
	@param		inSubmissionURL						The full URL to the script on your Web server that will handle the submissions, or NULL if you prefer the crash report to be sent
													to you via email (in this case, inSubmissionEmail must be not NULL). Example: "http://www.mycompany.com/cgi-bin/processor.cgi"
	@param		inSubmissionEmailTicket				The email ticket for email you wish to receive crash report at, or NULL if you are handling it yourself on your web server. If this is set, 
													Unsanity server will handle the submission and email you the crash log. <i>Important:</i> this is NOT the email address. It is a unique
													email ticket that you have to register with Unsanity by going to
													<a href="http://www.unsanity.com/goto/email-ticket/">http://www.unsanity.com/goto/email-ticket/</a>. This is done for email verification
													purposes, and also to not expose your real email to end users, if they peek inside of the SCR database somehow
	@param		inOptionalCommentsTemplate			If this argument is not NULL, this string will be inserted into the "Description" field of the CrashReporter dialog.
													This can be useful if you want to put something like "Put your email here if you wish to be contacted by the developer regarding this
													crash log", or anything else.
    @result     One of the following:
				<ul>
				<li> noErr:							Registration successful. If there is already such a match specifier registered, it is replaced with the fresh information
													supplied to this call.
				<li> paramErr:						inSubmissionURL and inSubmissionEmailTicket are NULL, or inMatchString is NULL, or inCompanyName is NULL
				<li> cNoMemErr:						Not enough memory to perform selected operation.
				<li> permErr:						Not enough permissions to write changes to disk.
				<li> bdNamErr:						The match string is malformed, or too wide to be accepted (don't try to register "com.", etc!)
				</ul>
*/
OSStatus		UnsanitySCR_RegisterMatchSpecifier(CFStringRef inMatchString, CFStringRef inCompanyName, CFStringRef inSubmissionURL, CFStringRef inSubmissionEmailTicket, CFStringRef inOptionalCommentsTemplate);

/*!
    @function	UnsanitySCR_UnregisterMatchSpecifier
    @abstract   Unregisters your product(s) with the Smart Crash Reports system.
    @discussion Unregisters your product(s) with the Smart Crash Reports system. Performing this call will cause Smart Crash Reports to stop submitting bug reports for the
				inMatchString provided. Note that for this call to succeed, inMatchString must exactly match the one used in call to UnsanitySCR_RegisterProduct.
    @param      inMatchString						The match string to unregister. This must be the same match string as used in the UnsanitySCR_RegisterProduct call before.
    @result     One of the following:
				<ul>
				<li> noErr:							Unregistration successful.
				<li> paramErr:						inMatchString is NULL
				<li> cNoMemErr:						Not enough memory to perform selected operation.
				<li> permErr:						Not enough permissions to write changes to disk.
				<li> fnfErr:						The provided inMatchString was not found in the Smart Crash Reports database. You can safely ignore this error, as
													it means that the match string you've provided does not exist already.
				</ul>
*/
OSStatus		UnsanitySCR_UnregisterMatchSpecifier(CFStringRef inMatchString);

/*!
    @function	UnsanitySCR_IsMatchSpecifierRegistered
    @abstract   Query whether the match specifier is registered with the Smart Crash Reports system.
    @discussion Queries whether the product is registered with the Smart Crash Reports system.
    @param      inMatchString						The match string to query for. Note that SCR will perform partial matches search, too, so for example if you have
													"com.mycompany" registered, and you query against "com.mycompany.otherapp", this function will return positive result.
	@param		outOptionalRealMatchString			If not NULL, this variable will contain the "real" match string that SCR contains in the database. You can
													use it in calls to UnsanitySCR_UnregisterProduct later, if you want. <i>Important</i>: You are responsible for releasing this
													value, if you have requested it, and the function returns 1.
    @result     <ul>
				<li> 0:								The match string you have provided is not registered with SCR.
				<li> 1:								The match string you have provided is registered with SCR.
				<li> -1:							inMatchString is NULL
				</ul>
*/
int				UnsanitySCR_IsMatchSpecifierRegistered(CFStringRef inMatchString, CFStringRef* outOptionalRealMatchString);

/*
	These are the keys defined for the Info.plist of the Application. Please look in the UnsanitySCR_RegisterMatchSpecifier documentation for the explanation of which is which.
*/

#define			kUnsanitySCR_InfoPlist_CompanyName					CFSTR("SmartCrashReports_CompanyName")
#define			kUnsanitySCR_InfoPlist_SubmissionURL				CFSTR("SmartCrashReports_URL")
#define			kUnsanitySCR_InfoPlist_SubmissionEmailTicket		CFSTR("SmartCrashReports_EmailTicket")
#define			kUnsanitySCR_InfoPlist_CommentsTemplate				CFSTR("SmartCrashReports_CommentsTemplate")

#ifdef __cplusplus
	}
#endif
