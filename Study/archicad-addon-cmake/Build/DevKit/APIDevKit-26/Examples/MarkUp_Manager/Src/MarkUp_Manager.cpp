// *****************************************************************************
// Description:		Source code for the MarkUp Manager Add-On
// *****************************************************************************

// ---------------------------------- Includes ---------------------------------

#include 	"APIEnvir.h"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"

#include 	"MarkUp_Test.hpp"


// ============================================================================
// Callback functions
//
//
// ============================================================================


GSErrCode __ACENV_CALL MenuCommandHandler (const API_MenuParams *menuParams)
{
	return ACAPI_CallUndoableCommand ("MarkUp API Function", [&] () {
		switch (menuParams->menuItemRef.itemIndex) {
			case  1: MarkUpTest::CreateMarkUp ();						break;
			case  2: MarkUpTest::DeleteMarkUp (true);					break;
			case  3: MarkUpTest::DeleteMarkUp (false);					break;
			case  4: break;
			case  5: MarkUpTest::AddSelectionToMarkUp (APIMarkUpComponent_Creation);		break;
			case  6: MarkUpTest::AddSelectionToMarkUp (APIMarkUpComponent_Highlight);		break;
			case  7: MarkUpTest::AddSelectionToMarkUp (APIMarkUpComponent_Deletion);		break;
			case  8: MarkUpTest::AddSelectionToMarkUpAsModification (CreateNewElementForModification);	break;
			case  9: MarkUpTest::AddSelectionToMarkUpAsModification (SetSelectedPairsAsModification);	break;
			case  10: MarkUpTest::RemoveSelectionFromMarkUp ();			break;
			case 11: break;
			case 12: MarkUpTest::AddCommentToMarkUp ();					break;
			case 13: MarkUpTest::DumpMarkUp ();							break;
			case 14: MarkUpTest::DumpMarkUpForSelection ();				break;
			case 15: MarkUpTest::ZoomToElementsOfMarkUp ();				break;
			case 16: break;
			case 17: MarkUpTest::ImportFromBCFFileSilentModeFalse ();	break;
			case 18: MarkUpTest::ImportFromBCFFileSilentModeTrue ();	break;
			case 19: MarkUpTest::ExportToBCF ();						break;
		}

		return NoError;
	});
}


// ============================================================================
// Required functions
//
//
// ============================================================================


//------------------------------------------------------
// Dependency definitions
//------------------------------------------------------
API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (&envir->addOnInfo.name, 32000, 1, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, 32000, 2, ACAPI_GetOwnResModule ());

	return APIAddon_Normal;
}		/* CheckEnvironment */


//------------------------------------------------------
// Interface definitions
//------------------------------------------------------
GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	ACAPI_Register_Menu (32500, 32510, MenuCode_UserDef, MenuFlag_SeparatorBefore);
	return NoError;
}		/* RegisterInterface */


//------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
//------------------------------------------------------
GSErrCode	__ACENV_CALL Initialize	(void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (32500, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("MarkUp_Manager::Initialize () ACAPI_Install_MenuHandler (32500) failed\n");

	return err;
}		/* Initialize */


//------------------------------------------------------
// Called when the Add-On is going to be unloaded
//------------------------------------------------------
GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}		/* FreeData */
