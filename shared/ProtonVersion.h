//Proton SDK engine version.  Bump when cutting a notable engine milestone and
//create a matching git tag (tag v1.2.3 means these say 1.2.3).  Apps can do
//compile-time checks like #if PROTON_VERSION >= 10100 (that would be v1.1.0).
//Note: BaseApp::GetAppVersion() is the app's own version, unrelated to this.

#ifndef ProtonVersion_h__
#define ProtonVersion_h__

#define PROTON_VERSION_MAJOR 1
#define PROTON_VERSION_MINOR 0
#define PROTON_VERSION_PATCH 0

//single number for easy comparisons: v1.2.3 becomes 10203
#define PROTON_VERSION (PROTON_VERSION_MAJOR*10000 + PROTON_VERSION_MINOR*100 + PROTON_VERSION_PATCH)

#define PROTON_VERSION_STRING "1.0.0"

#endif // ProtonVersion_h__
