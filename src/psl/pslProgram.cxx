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


#include "pslLocal.h"

pslProgram::pslProgram ( pslExtension *ext, const char *_prgnm )
{
  if ( ! _pslInitialised )
    ulSetError ( UL_FATAL,
          "PSL: You didn't call pslInit() before using PSL functions." ) ;

  code = new pslOpcode [ MAX_CODE ] ;

  extensions = ext ;

  progName = NULL ;

  if ( _prgnm == NULL ) _prgnm = "PSLptogram" ;

  setProgName ( _prgnm ) ;

  compiler = new pslCompiler ( code, ext, getProgName () ) ;
  context  = new pslContext ( this ) ;

  compiler-> init  () ;
  context -> reset () ;
}
 

pslProgram::pslProgram ( pslProgram *src, const char *_prgnm )
{
  progName = NULL ;

  if ( _prgnm == NULL ) _prgnm = src -> getProgName () ;

  setProgName ( _prgnm ) ;

  code       = src -> getCode       () ;
  compiler   = src -> getCompiler   () ;
  extensions = src -> getExtensions () ;
  userData   = src -> getUserData   () ;

  context = new pslContext ( this ) ;
  context -> reset () ;
}
 

pslProgram::~pslProgram ()
{
  delete [] progName ;
  delete    compiler ;
  delete    context  ;
  delete [] code     ;
}


void       pslProgram::dump  () const {        compiler-> dump  () ; }
void       pslProgram::reset ()       {        context -> reset () ; }
pslResult  pslProgram::step  ()       { return context -> step  () ; }
pslResult  pslProgram::trace ()       { return context -> trace () ; }



int pslProgram::compile ( const char *fname )
{
  if ( strcmp ( getProgName(), "PSLprogram" ) == 0 )
    setProgName ( fname ) ;

  return compiler -> compile ( fname ) ;
}


int pslProgram::compile ( FILE *fd )
{
  return compiler -> compile ( fd ) ;
}



