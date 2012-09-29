/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#ifndef AUTOPROBEERROR_H
#define AUTOPROBEERROR_H

#include <string>
#include <sstream>


using namespace std;

//All of these still operate in release mode, use the macros for dbg only stuff
void ap_logerror( const string& strEntry, bool bTimestamp = true );
void ap_error( const string& strError, bool bLog = false );
bool ap_error( bool expr, const string& strError, bool bLog = false );
void ap_fatalerror( const string& strError ); //Always logged
bool ap_assert( bool expr, const string& strError );

//Macros
//Using two of these is apparently the only way to get the preprocessor to convert numeric->string literal
//http://www.decompile.com/cpp/faq/file_and_line_error_string.htm
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

//RLSASSERT remains in for release builds
#define AP_RLSASSERT(x) \
    ap_assert( x, "Assertion failed\n" #x "\n" __FILE__ ":" TOSTRING(__LINE__) )

//These will automatically generate the message for you with file and line
//DBGASSERT is compiled out in release builds (standard assert behavior)
#ifdef NDEBUG
    #define AP_DBGASSERT(x)
#else
    #define AP_DBGASSERT(x) \
        AP_RLSASSERT((x))
#endif

#endif // AUTOPROBEERROR_H
