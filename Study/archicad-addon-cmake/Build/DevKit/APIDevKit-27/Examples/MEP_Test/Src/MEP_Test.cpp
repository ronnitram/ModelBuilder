#include "MEP_Test.hpp"
#include "Resources.hpp"

#include "QueryElementsAndDefaults.hpp"
#include "PlaceElements.hpp"
#include "ModifyElements.hpp"
#include "Preferences.hpp"
#include "Utility.hpp"

// API
#include "ACAPinc.h"


// -----------------------------------------------------------------------------
// MenuCommandHandler
//		called to perform the user-asked command
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL MenuCommandHandler (const API_MenuParams *menuParams)
{
	switch (menuParams->menuItemRef.menuResID) {
		case MEP_TEST_MENU_STRINGS:
		{
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:
					return MEPExample::PlaceMEPElements ();
				case 2:
					return MEPExample::CopyPlaceSelectedTerminals ();
				case 4:
					return MEPExample::PlaceTwoContinuousRoutes ();
				case 5:
					return MEPExample::PlaceTwoRoutesInTShape ();
				case 6:
					return MEPExample::PlaceThreeRoutesInTShape ();
				case 7:
					return MEPExample::PlaceConnectedElements ();
				case 9:
					return MEPExample::QuerySelectedElementDetails ();
				case 10:
					return MEPExample::QueryDefaultDetails ();
				case 12:
					return MEPExample::ModifySelectedBranches ();
				case 13:
					return MEPExample::ModifySelectedRoutingElements ();
				case 14:
					return MEPExample::ModifySelectedFlexibleSegments ();
				case 15:
					return MEPExample::ShiftAndReorientSelectedMEPElements ();
				case 16:
					return MEPExample::ModifySelectedElemGeneralParameters ();
				case 17: 
					return MEPExample::ModifySelectedPipeFittingGDLParameters ();
				case 18: 
					return MEPExample::ModifySelectedRoutingElemGDLParameters ();
				case 19: 
					return MEPExample::ModifyPipeFittingDefaultParameters ();
				case 20: 
					return MEPExample::ModifyRoutingElemDefaultParameters ();
				case 22: 
					return MEPExample::DeleteSelectedElements ();
				case 24:
					return MEPExample::QueryMEPPreferences ();
				case 25:
					return MEPExample::ModifyMEPPreferences ();
				case 26:
					return MEPExample::ModifyLibraryPartOfSelectedPipeTerminal ();
				case 27:
					return MEPExample::ModifyLibraryPartOfDefaultPipeTerminal ();
				case 28:
					return MEPExample::ModifyLibraryPartOfSelectedCableCarrierRoutesFirstRigidSegment ();
				case 29:
					return MEPExample::ModifyLibraryPartOfSelectedCableCarrierRoutesDefaultRigidSegment ();
				case 30:
					return MEPExample::ModifyLibraryPartOfDefaultCableCarrierRoutesRigidSegment ();
				case 32:
					return MEPExample::CreateElementSetFromSelectedElements ();
				case 33:
					return MEPExample::CreateElementLinksBetweenSelectedElements ();
			}
			break;
		}
	}

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
	RSGetIndString (&envir->addOnInfo.name,        MEP_TEST_ADDON_NAME, 1, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, MEP_TEST_ADDON_NAME, 2, ACAPI_GetOwnResModule ());

	return APIAddon_Normal;
}		// CheckEnvironment


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode	__ACDLL_CALL	RegisterInterface (void)
{
	GSErrCode err = ACAPI_MenuItem_RegisterMenu (MEP_TEST_MENU_STRINGS, MEP_TEST_MENU_PROMPT_STRINGS, MenuCode_UserDef, MenuFlag_Default);

	DBASSERT (err == NoError);

	return err;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err = ACAPI_MenuItem_InstallMenuHandler (MEP_TEST_MENU_STRINGS, MenuCommandHandler);

	DBASSERT (err == NoError);

	ACAPI_KeepInMemory (true);

	return err;
}


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}
