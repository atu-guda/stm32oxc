#include <oxc_base.h>

#include <oxc_picoc.h>


#ifndef NO_DEBUGGER
#include <signal.h>

Picoc *break_pc = NULL;


static void BreakHandler( int Signal )
{
  break_pc->DebugManualBreak = TRUE;
}

void PlatformInit( Picoc *pc )
{
  /* capture the break signal and pass it to the debugger */
  break_pc = pc;
  signal( SIGINT, BreakHandler );
}
#else
void PlatformInit( Picoc *pc )
{
}
#endif

void PlatformCleanup( Picoc *pc )
{
}

/* get a line of interactive input */
char *PlatformGetLine( char *Buf, int MaxLen, const char *Prompt )
{
  if( Prompt != NULL ) {
    printf("%s", Prompt);
  }

  fflush(stdout);
  return fgets(Buf, MaxLen, stdin);
}

/* get a character of interactive input */
int PlatformGetCharacter()
{
  fflush( stdout );
  return getchar();
}

/* write a character to the console */
void PlatformPutc( unsigned char OutCh, union OutputStreamInfo *Stream )
{
  putchar( OutCh );
}

PlatformReadFile_fun_t PlatformReadFile_fun = NULL;

/* read a file into memory */
char *PlatformReadFile( Picoc *pc, const char *FileName )
{
  if( PlatformReadFile_fun ) {
    return PlatformReadFile_fun( pc, FileName );
  }

  return 0;
}

/* read and scan a file for definitions */
void PicocPlatformScanFile( Picoc *pc, const char *FileName )
{
  char *SourceStr = PlatformReadFile(pc, FileName);

  /* ignore "#!/path/to/picoc" .. by replacing the "#!" with "//" */
  if( SourceStr != NULL && SourceStr[0] == '#' && SourceStr[1] == '!' ) {
    SourceStr[0] = '/';
    SourceStr[1] = '/';
  }

  PicocParse(pc, FileName, SourceStr, strlen(SourceStr), TRUE, FALSE, TRUE, TRUE);
}

/* exit the program */
void PlatformExit( Picoc *pc, int RetVal )
{
  PlatformPrintf( pc->CStdOut, "######## platform exit %d\r\n", RetVal );
  pc->PicocExitValue = RetVal;
  longjmp( pc->PicocExitBuf, 1 ); // ??? TMP
}

