/*
 * Ultimate Unprecacher
 * Copyright (c) 2016 Alik Aslanyan <cplusplus256@gmail.com>
 *
 *
 *
 *    This program is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permission to
 *    link the code of this program with the Half-Life Game Engine ("HL
 *    Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL Engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */

#include <extdll.h>			// always

#include <meta_api.h>		// of course

#include "sdk_util_custom.h"		// UTIL_LogPrintf, etc

#include "cstring_utils.h"

#include <algorithm>

#include "config_file.h"

#if defined _MSC_VER
#pragma comment(linker, "/EXPORT:GiveFnptrsToDll=_GiveFnptrsToDll@8,@1")
#endif // _WIN32

#ifdef _WIN32
extern "C" __declspec(dllexport) void __stdcall GiveFnptrsToDll(enginefuncs_t* pengfuncsFromEngine,globalvars_t *pGlobals);
#elif defined __linux__
extern "C" void GiveFnptrsToDll(enginefuncs_t* pengfuncsFromEngine,globalvars_t *pGlobals);
#endif

#include "global_variables.h"
unprecache_list* GlobalVariables::g_ulUnprecacheList;
char* GlobalVariables::g_szDLLDirPath;
// Must provide at least one of these..
static META_FUNCTIONS gMetaFunctionTable = {
	NULL,			// pfnGetEntityAPI				HL SDK; called before game DLL
	NULL,			// pfnGetEntityAPI_Post			META; called after game DLL
	GetEntityAPI2,	// pfnGetEntityAPI2				HL SDK2; called before game DLL
    NULL,			// pfnGetEntityAPI2_Post		META; called after game DLL
	NULL,			// pfnGetNewDLLFunctions		HL SDK2; called before game DLL
    NULL,			// pfnGetNewDLLFunctions_Post	META; called after game DLL
	GetEngineFunctions,	// pfnGetEngineFunctions	META; called before HL engine
    NULL,			// pfnGetEngineFunctions_Post	META; called after HL engine
};

// Description of plugin
plugin_info_t Plugin_info = {
    META_INTERFACE_VERSION,
    "Ultimate Unprecacher",
    "0.2 Alpha",
    "2016/05/18",
    "Alik Aslanyan <cplusplus256@gmail.com>",
    "http://www.metamod.org/",
    "",
    PT_ANYTIME,
    PT_ANYTIME,
};

meta_globals_t *gpMetaGlobals;
gamedll_funcs_t *gpGamedllFuncs;
mutil_funcs_t *gpMetaUtilFuncs;

C_DLLEXPORT int Meta_Query(const char * /*ifvers */, plugin_info_t **pPlugInfo,
		mutil_funcs_t *pMetaUtilFuncs) 
{
	// Give metamod our plugin_info struct
	*pPlugInfo=&Plugin_info;
	// Get metamod utility function table.
	gpMetaUtilFuncs=pMetaUtilFuncs;
	return(TRUE);
}

C_DLLEXPORT int Meta_Attach(PLUG_LOADTIME /* now */, 
		META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals, 
		gamedll_funcs_t *pGamedllFuncs) 
{
	if(!pMGlobals) {
        UTIL_LogError("[Error] Meta_Attach called with null pMGlobals");
		return(FALSE);
	}
	gpMetaGlobals=pMGlobals;
	if(!pFunctionTable) {
        UTIL_LogError("[Error] Meta_Attach called with null pFunctionTable");
		return(FALSE);
	}
	memcpy(pFunctionTable, &gMetaFunctionTable, sizeof(META_FUNCTIONS));
	gpGamedllFuncs=pGamedllFuncs;
    const char* szDLLPath = GET_PLUGIN_PATH(PLID);
    GlobalVariables::g_szDLLDirPath = get_path((char*)szDLLPath);
    unsigned int iDllDirPathLen = strlen(GlobalVariables::g_szDLLDirPath);
    char* szPathToIni = new char[iDllDirPathLen + 29 + 1];
    sprintf(szPathToIni,"%sconfig/unprecache_list.ini", GlobalVariables::g_szDLLDirPath);
    GlobalVariables::g_ulUnprecacheList = new unprecache_list(szPathToIni);

    delete[] szPathToIni;

    char* szPathToCfg = new char[iDllDirPathLen + 35 + 1];
    sprintf(szPathToCfg,"%sconfig/unprecacher.cfg", GlobalVariables::g_szDLLDirPath);
    config_file::LoadCfg(szPathToCfg);
    delete[] szPathToCfg;


    return(TRUE);
}

C_DLLEXPORT int Meta_Detach(PLUG_LOADTIME,
        PL_UNLOAD_REASON)
{
    delete GlobalVariables::g_ulUnprecacheList;
    delete[] GlobalVariables::g_szDLLDirPath;
    //delete g_ulNotDeleteList;
	return(TRUE);
}


