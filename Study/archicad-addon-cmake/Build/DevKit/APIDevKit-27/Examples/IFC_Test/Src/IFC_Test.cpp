#include "APIEnvir.h"
#include "ACAPinc.h"		// also includes APIdefs.h

#include "Resources.hpp"

#include "IFCDifference.hpp"
#include "IFCPresetCommands.hpp"


// -----------------------------------------------------------------------------
// MenuCommandHandler
//		called to perform the user-asked command
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL MenuCommandHandler (const API_MenuParams *menuParams)
{
	switch (menuParams->menuItemRef.menuResID) {
		case IFCPRESETCOMMANDS_TEST_MENU_STRINGS:
			{
				switch (menuParams->menuItemRef.itemIndex) {
					case 1: CreatePreset (); break;
					case 2: HasPreset (); break;
					case 3: ClearPreset (); break;
					case 4: AddNewTypeMappingForImport (); break;
					case 5: AddNewTypeMappingForExport ("ifc2x3"); break;
					case 6: AddNewTypeMappingForExport (); break;
					case 7: AddNewPropertyMappingForExport (); break;
					case 8: AddNewPropertyMappingForImport (); break;
					default: DBBREAK_STR ("Unhandled menu item!"); break;
				}
				break;
			}
		case IFCDIFFERENCE_TEST_MENU_STRINGS:
			{
				switch (menuParams->menuItemRef.itemIndex) {
					case 1: InvokeExportSettingsDialog ();		break;
					case 2: StoreCurrentIFCDifferenceState ();	break;
					case 3: ShowIFCDifference ();				break;
				}
				break;
			}
		default:
			DBBREAK_STR ("Unhandled menu item!"); break;
	}

	ACAPI_KeepInMemory (true);

	return NoError;
}		// MenuCommandHandler


// =============================================================================
//
// Required functions
//
// =============================================================================

// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------

API_AddonType	__ACDLL_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	if (envir->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	RSGetIndString (&envir->addOnInfo.name, IFC_TEST_ADDON_NAME, 1, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, IFC_TEST_ADDON_NAME, 2, ACAPI_GetOwnResModule ());

	if (ACAPI_AddOnIntegration_RegisterRequiredService (&IfcMdid, CREATEPRESET, 1L) != NoError)
		return APIAddon_DontRegister;
	if (ACAPI_AddOnIntegration_RegisterRequiredService (&IfcMdid, HASPRESET, 1L) != NoError)
		return APIAddon_DontRegister;
	if (ACAPI_AddOnIntegration_RegisterRequiredService (&IfcMdid, CLEARPRESET, 1L) != NoError)
		return APIAddon_DontRegister;
	if (ACAPI_AddOnIntegration_RegisterRequiredService (&IfcMdid, ADDNEWTYPEMAPPINGFORIMPORT, 1L) != NoError)
		return APIAddon_DontRegister;
	if (ACAPI_AddOnIntegration_RegisterRequiredService (&IfcMdid, ADDNEWTYPEMAPPINGFOREXPORT, 1L) != NoError)
		return APIAddon_DontRegister;
	if (ACAPI_AddOnIntegration_RegisterRequiredService (&IfcMdid, ADDNEWPROPERTYMAPPINGFOREXPORT, 1L) != NoError)
		return APIAddon_DontRegister;

	return APIAddon_Normal;
}		// CheckEnvironment


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode	__ACDLL_CALL	RegisterInterface (void)
{
	GSErrCode err = ACAPI_MenuItem_RegisterMenu (IFCPRESETCOMMANDS_TEST_MENU_STRINGS, IFCPRESETCOMMANDS_TEST_MENU_PROMPT_STRINGS, MenuCode_UserDef, MenuFlag_InsertIntoSame);
	err |= ACAPI_MenuItem_RegisterMenu (IFCDIFFERENCE_TEST_MENU_STRINGS, IFCDIFFERENCE_TEST_MENU_PROMPT_STRINGS, MenuCode_UserDef, MenuFlag_InsertIntoSame);

	DBASSERT (err == NoError);

	return err;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err	= ACAPI_MenuItem_InstallMenuHandler (IFCPRESETCOMMANDS_TEST_MENU_STRINGS, MenuCommandHandler);
	err			   |= ACAPI_MenuItem_InstallMenuHandler (IFCDIFFERENCE_TEST_MENU_STRINGS, MenuCommandHandler);

	DBASSERT (err == NoError);

	InitIFCDifference ();

	return err;
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	FreeIFCDifference ();

	return NoError;
}		// FreeData
