/*
 *  Tamgu (탐구)
 *
 * Copyright 2019-present NAVER Corp.
 * under BSD 3-clause
 */
/* --- CONTENTS ---
 Project    : Tamgu (탐구)
 Version    : See tamgu.cxx for the version number
 filename   : globaltamgu.h
 Date       : 2017/09/01
 Purpose    :  
 Programmer : Claude ROUX (claude.roux@naverlabs.com)
 Reviewer   :
*/

#ifndef globaltamgu_h
#define globaltamgu_h

#ifdef WIN32
const long DEFAUTTHREADMAX = 1000;
#else
const long DEFAUTTHREADMAX = 500;
#endif

//------------------------------------------------------------------------
// We use these methods to create different Jtamgu objects...
Exporting int TamguCreateGlobal(long nbthreads = DEFAUTTHREADMAX);
Exporting bool TamguDeleteGlobal(int idx);
Exporting bool TamguSelectglobal(int idx);
Exporting bool TamguReleaseglobal(int idx);
Exporting void TamguCleanAllGlobals();
Exporting TamguGlobal* GlobalTamgu(int idx);
//------------------------------------------------------------------------
//These methods are used to create an tamgu engine...
void FinalTamguConstantCleaning(void);

Exporting TamguGlobal* TamguCreate(long nbthreads = 50);
Exporting bool TamguExtinguish();
Exporting short TamguLoad(string filename);
Exporting short TamguCompile(string& code, string filename, bool display = false);
Exporting size_t TamguCompile(string& code, short idcode);
Exporting bool TamguRun(short icode, bool glock = false);
Exporting bool TamguLoading(short icode);
Exporting short TamguLastCodeSpace();
Exporting TamguCode* TamguCurrentSpace();
Exporting TamguCode* TamguCodeSpace(short idcode);
Exporting string TamguCurrentFilename();
Exporting string TamguErrorMessage();
Exporting void TamguSetinitline(long i);
Exporting long TamguCurrentLine();
Exporting bool TamguRunning();
Exporting TamguGlobal* GlobalTamgu();
Exporting short TamguCurrentThreadId();
Exporting string TamguIndentation(string& codestr,string blanc);
Exporting bool TamguStop();
Exporting void TamguDisplayFunction(DisplayFunctionCall call, void* o);
Exporting void TamguSetArguments(string args);
Exporting void TamguSetArguments(vector<string>& args);
Exporting void TamguAllObjects(vector<string>& vs);
Exporting Tamgu* TamguExecute(TamguCode* code, string name, vector<Tamgu*>& params);
Exporting Tamgu* TamguExecute(TamguCode* code, string name, vector<Tamgu*>& params, short idthread);
Exporting void TamguSetos();
Exporting string TamguDisplayValue();
Exporting string TamguListing();
Exporting wstring TamguUListing();
Exporting void TamguSetCode(string& code);
Exporting void TamguSetCode(wstring& code);
Exporting void TamguAddCode(string& code);
Exporting void TamguAddCode(wstring& code);
Exporting void TamguSpaceInit(string filename);
#endif

