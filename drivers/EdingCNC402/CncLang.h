#ifndef __CNCLANG__H__
#define __CNCLANG__H__

#ifdef EXP2DF
#undef EXP2DF
#endif

#ifdef CNCAPI_EXPORTS
#define EXP2DF __declspec(dllexport)
#else
#define EXP2DF __declspec(dllimport)
#endif

//Every place where string translation is required, use this macro
#define _(STRING) CncLangGetText(STRING)
#define notr(STRING) (STRING)
#define CNC_LSEP '|'

#define CNC_LANG_MAX_TEXT_LEN (255)
#define CNC_LANG_MAX_LINE_LEN ((CNC_LANG_MAX_TEXT_LEN * CNC_LANG_LAST) + CNC_LANG_LAST)

#ifdef __cplusplus
extern "C" {
#endif

/*
* Name   : CncLangSet
* In     : rc
* Out    : -
* Return : Set language
*/
void EXP2DF __stdcall CncLangSet(CNC_LANG_T language);


/*
* Name   : CncLangGetText
* In     : rc
* Out    : -
* Return : Translated text, translated from english
*/
char EXP2DF * __stdcall CncLangGetText(char *english);

/*
* Name   : CncLangGetText2
* In     : rc
* Out    : -
* Return : Translated text, translated from given LANGUAGE
*/
char EXP2DF * __stdcall CncLangGetText2(char *text, CNC_LANG_T fromLanguage);


/*
* Name   : CncLangGetText2
* In     : rc
* Out    : All Translated texts, NULL if there are none
* Return : 0 if ok, -1 if failed (not existing)
*/
int EXP2DF __stdcall CncLangGetAllTexts(char *english, //input
						     char **german,
							 char **dutch,
							 char **italian,
							 char **french,
							 char **spanish,
							 char **portugese,
                             char **turkish,
							 char **japanese,
							 char **greek,
							 char **hun,
							 char **cze,
							 char **hkr,
							 char **cn,
							 char **new1
							 );
/*
* Name   : CncLangAddText
* In     : rc
* Out    : Translated text
* Return : 0 if ok, -1 if failed, -2 if exists
*/
int EXP2DF __stdcall CncLangAddText(char *english, 
						  char *german,
						  char *dutch,
						  char *italian,
						  char *french,
						  char *spanish,
						  char *portugese,
                          char *turkish,
						  char *japanese,
						  char *greek,
						  char *hun,
						  char *cze,
						  char *hkr,
						  char *cn,
						  char *new1
						  );
/*
* Name   : CncLangImport
* In     : rc
* Out    : -
* Return : -1 if error, 0 if ok
*/
int EXP2DF __stdcall CncLangImport(char *fileName);

/*
* Name   : CncLangExport, CncLangExport2 for new format
* In     : rc
* Out    : -
* Return : -1 if error, 0 if ok
*/
int EXP2DF __stdcall CncLangExport(char *fileName);

#ifdef __cplusplus
}
#endif


#endif