/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.

     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


struct pslFwdRef
{
  char       *symbol ;
  pslAddress  where ;

  void set ( const char *s, pslAddress w )
  {
    symbol = ulStrDup ( s ) ;
    where = w ;
  }

  pslAddress getWhere () const { return where ; }

  int matches ( const char *s ) const
  {
    return symbol != NULL && strcmp ( s, symbol ) == 0 ;
  }

} ;



class pslProgram ;


class pslCompiler
{
  /* File I/O and preprocessor */

  int  getChar               () ;

  void doIncludeStatement    () ;
  void doDefineStatement     () ;
  void doIfdefStatement      () ;
  void doUndefStatement      () ;
  void doIfndefStatement     () ;
  void doElseStatement       () ;
  void doEndifStatement      () ;

  int  doPreProcessorCommand () ;
  void getToken              ( char *s, int define_sub = TRUE ) ;
  void ungetToken            ( const char *s ) ;
  void skipToEOL             () ;


  /* Write data into Code space */

  void pushCodeByte ( unsigned char b ) ;
  void pushCodeAddr ( pslAddress a ) ;
  int  printOpcode  ( FILE *fd, int addr ) const ;

  void pushLineNumber ( int l ) ;

  /* Write single byte-coded instructions into code space.  */

  void pushStackDup     () ;
  void pushPop          () ;
  void pushSubtract     () ;
  void pushAdd          () ;
  void pushDivide       () ;
  void pushMultiply     () ;
  void pushModulo       () ;
  void pushNegate       () ;
  void pushNot          () ;
  void pushTwiddle      () ;
  void pushOrOr         () ;
  void pushAndAnd       () ;
  void pushOr           () ;
  void pushAnd          () ;
  void pushXor          () ;
  void pushShiftLeft    () ;
  void pushShiftRight   () ;
  void pushLess         () ;
  void pushLessEqual    () ;
  void pushGreater      () ;
  void pushGreaterEqual () ;
  void pushNotEqual     () ;
  void pushEqual        () ;
  int  pushPeekJumpIfFalse ( int l ) ;
  int  pushPeekJumpIfTrue  ( int l ) ;
  int  pushJumpIfFalse  ( int l ) ;
  int  pushJumpIfTrue   ( int l ) ;
  int  pushJump         ( int l ) ;

  void pushGetParameter  ( pslAddress var, int argpos ) ;

  void pushIncrement     ( const char *s ) ;
  void pushDecrement     ( const char *s ) ;

  void makeIntVariable   ( const char *s ) ;
  void makeFloatVariable ( const char *s ) ;
  void makeStringVariable( const char *s ) ;

  void pushConstant      ( const char *s ) ;
  void pushIntConstant   ( const char *s ) ;
  void pushFloatConstant ( const char *s ) ;
  void pushStringConstant( const char *s ) ;
  void pushCharConstant  ( char c ) ;
  void pushIntConstant   ( int i ) ;

  void pushVoidConstant  () ;

  void pushVariable      ( const char *s ) ;
  void pushAssignment    ( const char *s ) ;
  void pushAddAssignment ( const char *s ) ;
  void pushSubAssignment ( const char *s ) ;
  void pushMulAssignment ( const char *s ) ;
  void pushModAssignment ( const char *s ) ;
  void pushDivAssignment ( const char *s ) ;
  void pushAndAssignment ( const char *s ) ;
  void pushOrAssignment  ( const char *s ) ;
  void pushXorAssignment ( const char *s ) ;
  void pushSHLAssignment ( const char *s ) ;
  void pushSHRAssignment ( const char *s ) ;

  void pushCall          ( const char *s, int argc ) ;
  void pushReturn        () ;

  /* Expression parsers & code generators.  */

  int pushPrimitive            () ;
  int pushBitwiseExpression    () ;
  int pushMultExpression       () ;
  int pushAddExpression        () ;
  int pushShiftExpression      () ;
  int pushRelExpression        () ;
  int pushBoolExpression       () ;
  int pushExpression           () ;

  /* Statement-level parsers & code generators. */

  int  pushBreakStatement      () ;
  int  pushContinueStatement   () ;
  int  pushReturnStatement     () ;
  int  pushPauseStatement      () ;
  int  pushSwitchStatement     () ;
  int  pushWhileStatement      () ;
  int  pushDoWhileStatement    () ;
  int  pushForStatement        () ;
  int  pushIfStatement         () ;
  int  pushFunctionCall        ( const char *s ) ;
  int  pushAssignmentStatement ( const char *s ) ;
  int  pushCompoundStatement   () ;
  int  pushStatement           () ;

  int  pushLocalVarDecl  ( pslType t ) ;
  int  pushGlobalVarDecl ( const char *fn, pslType t ) ;
  int  pushStaticVarDecl () ;

  /* Top level constructs */

  int  pushFunctionDeclaration       ( const char *fn ) ;
  int  pushGlobalDeclaration         () ;
  void pushProgram                   () ;

  /* The symbol tables for variables, code and define's */

  int next_label ;
  int next_code_symbol ;
  int next_define ;

  int generate_line_numbers ;

  char *define_token       [ MAX_SYMBOL ] ;
  char *define_replacement [ MAX_SYMBOL ] ;

  int searchDefines ( const char *s ) const ;

  int skipOverride ;
  int skippingFlag  ;
  int next_skippingLevel ;

  int skipping () const
  {
    return ! skipOverride && skippingFlag != 0 ;
  }

  pslSymbol         symtab [ MAX_SYMBOL ] ;
  pslSymbol    code_symtab [ MAX_SYMBOL ] ;

  pslAddress getVarSymbol       ( const char *s ) ;
  pslAddress setVarSymbol       ( const char *s ) ;

  pslAddress getCodeSymbol      ( const char *s, pslAddress fixupLoc ) ;
  void       setCodeSymbol      ( const char *s, pslAddress v ) ;

  int        getExtensionSymbol ( const char *s ) const ;

  const pslExtension *extensions ;

  /* Forward references to code symbols that are not yet defined */

  int next_fwdref ;
  pslFwdRef    forward_ref [ MAX_SYMBOL ] ;

  int locality_stack [ MAX_NESTING ] ;
  int locality_sp ;

  void fixup                  ( const char *s, pslAddress v ) ;
  void addFwdRef              ( const char *s, pslAddress where ) ;
  void checkUnresolvedSymbols () ;

  void pushLocality ()
  {
    if ( locality_sp >= MAX_NESTING-1 )
      error ( "Too many nested {}'s" ) ;
    else
      locality_stack [ locality_sp++ ] = next_var ;
  }

  void popLocality  ()
  {
    if ( locality_sp <= 0 )
      error ( "Locality stack underflow !!" ) ;

    /* Delete any local symbols */

    for ( int i = locality_stack [ locality_sp-1 ] ;
              i < next_var ; i++ )
    {
      delete [] symtab [ i ] . symbol ;
      symtab [ i ] . symbol = NULL ;
    }

    /* Put the next_var pointer back where it belongs */

    next_var = locality_stack [ --locality_sp ] ;
  }

  /* Ikky stuff to remember where break and continue should jump */

  int breakToAddressStack    [ MAX_LABEL ] ;
  int continueToAddressStack [ MAX_LABEL ] ;
  int next_break     ;
  int next_tmp_label ;
  int next_continue  ;

  void pushBreakToLabel    () ;
  int  pushContinueToLabel () ;
  void  setContinueToLabel ( int which ) ;
  void pushNoContinue      () ;

  void popBreakToLabel     () ;
  void popContinueToLabel  () ;

  /* Error and warning handlers */

  int num_errors   ;
  int num_warnings ;

  void bumpErrors   () { num_errors++   ; }
  void bumpWarnings () { num_warnings++ ; }

  int error   ( const char *fmt, ... ) ;
  int warning ( const char *fmt, ... ) ;

  /* Remember the name of the program for debug purposes */

  char *progName ;

  char *getProgName () const { return progName ; }

  /* Major storage for symbols and byte-codes */

  int next_var   ;
  int next_code  ;
  pslOpcode    *code       ;
  pslContext   *context    ;
  pslProgram   *program    ;

public:

  pslCompiler ( pslProgram   *prog,
                pslOpcode    *_code,
                const pslExtension *_extn,
                const char   *_progName )
  {
    program  = prog ;
    progName = ulStrDup ( _progName ) ;

    generate_line_numbers = FALSE ;
    code       = _code ;
    extensions = _extn ;

    for ( int i = 0 ; i < MAX_SYMBOL ; i++ )
    {
      define_token       [ i ] = NULL ;
      define_replacement [ i ] = NULL ;
      symtab      [ i ] . symbol = NULL ;
      forward_ref [ i ] . symbol = NULL ;
      code_symtab [ i ] . symbol = NULL ;
    }

    init () ;
  }

  ~pslCompiler ()
  {
    for ( int i = 0 ; i < MAX_SYMBOL ; i++ )
    {
      delete [] define_token       [ i ] ;
      delete [] define_replacement [ i ] ;
      delete [] symtab      [ i ] . symbol ;
      delete [] code_symtab [ i ] . symbol ;
      delete [] forward_ref [ i ] . symbol ;
    }
  }

  const pslExtension *getExtensions () const { return extensions ; }

  int printInstruction ( FILE *fd, int addr ) const ;


  void generateLineNumbers ()
  {
    generate_line_numbers = TRUE ;
  }


  void init () 
  {
    int i ;

    for ( i = 0 ; i < MAX_CODE   ; i++ ) code   [ i ] = OPCODE_HALT ; 

    for ( i = 0 ; i < MAX_SYMBOL ; i++ )
    {
      delete [] define_token       [ i ] ; define_token       [ i ] = NULL ;
      delete [] define_replacement [ i ] ; define_replacement [ i ] = NULL ;
      delete [] symtab      [ i ] . symbol ; symtab      [ i ] . symbol = NULL ;
      delete [] code_symtab [ i ] . symbol ; code_symtab [ i ] . symbol = NULL ;
      delete [] forward_ref [ i ] . symbol ; forward_ref [ i ] . symbol = NULL ;
    }

    num_errors = num_warnings = 0 ;

    locality_sp = 0 ;

    next_define   = 0 ;
    next_continue = 0 ;
    next_break    = 0 ;
    next_tmp_label= 0 ;
    next_fwdref   = 0 ;
    next_label    = 0 ;
    next_code_symbol = 0 ;
    next_code     = 0 ;
    next_var      = 0 ;
    skippingFlag  = 0 ;
    next_skippingLevel = 1 ;
  }

  void dump () const ;
  int  compile ( const char *fname ) ;
  int  compile ( FILE *fd, const char *fname = NULL ) ;
} ;

