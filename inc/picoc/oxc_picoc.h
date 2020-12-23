// atu: based on picoc project https://gitlab.com/zsaleeba/picoc (new BSD license)
//
/* picoc external interface. This should be the only header you need to use if
 * you're using picoc as a library. Internal details are in interpreter.h */
#ifndef PICOC_H
#define PICOC_H

#ifdef __cplusplus
extern "C" {
#endif

/* picoc version number */
#ifdef VER
#define PICOC_VERSION "v2.2 beta r" VER         /* VER is the subversion version number, obtained via the Makefile */
#else
#define PICOC_VERSION "v2.2"
#endif

/* handy definitions */
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#include <oxc_picoc_interpreter.h>


#if defined(UNIX_HOST) || defined(WIN32)

/* this has to be a macro, otherwise errors will occur due to the stack being corrupt */
#define PicocPlatformSetExitPoint(pc) setjmp((pc)->PicocExitBuf)
#endif

#ifdef SURVEYOR_HOST
/* mark where to end the program for platforms which require this */
extern int PicocExitBuf[];

#define PicocPlatformSetExitPoint(pc) setjmp((pc)->PicocExitBuf)
#else
#ifdef USE_OXC
#include <setjmp.h>
/* mark where to end the program for platforms which require this */
#define PicocPlatformSetExitPoint(pc) setjmp((pc)->PicocExitBuf)
#endif
#endif

/* parse.c */
void PicocParse(Picoc *pc, const char *FileName, const char *Source, int SourceLen, int RunIt, int CleanupNow, int CleanupSource, int EnableDebugger);
void PicocParseInteractive(Picoc *pc);

/* *platform*.c */
void PicocCallMain( Picoc *pc, int argc, char **argv );
void PicocInitialise(Picoc *pc, int StackSize );
void PicocCleanup( Picoc *pc );
char *PlatformReadFile( Picoc *pc, const char *FileName );
void PicocPlatformScanFile( Picoc *pc, const char *FileName );
typedef char *(*PlatformReadFile_fun_t)( Picoc *pc, const char *FileName );
extern PlatformReadFile_fun_t PlatformReadFile_fun;

/* include.c */
void PicocIncludeAllSystemHeaders(Picoc *pc);

#ifdef __cplusplus
}
#endif

#endif /* PICOC_H */
