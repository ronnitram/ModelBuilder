// *****************************************************************************
// Source code for the Navigator Test Add-On
// *****************************************************************************

#define _API_NAVIGATOR_TRANSL_

// ------------------------------ Includes -------------------------------------

#include "APIEnvir.h"
#include "ACAPinc.h"

#include "NavigatorUtility.hpp"
#include "NavigatorWindowHandling.hpp"
#include "Resources.hpp"


//------------------------------------------------------
//------------------------------------------------------
static GSErrCode __ACENV_CALL    NotificationHandler (API_NotifyEventID notifID, Int32 /*param*/)
{
	GSErrCode err = NoError;

	switch (notifID) {
	case APINotify_AllInputFinished:
		err = NavigatorUtility::RegisterNavigatorRootGroup ();
		break;
	case APINotify_ReceiveChanges:
		NavigatorWindowHandling::RegenerateContentIfAppropriate ();
		break;
	}

	return err;
}


//------------------------------------------------------
//------------------------------------------------------
GSErrCode __ACENV_CALL MenuCommandHandler (const API_MenuParams* menuParams)
{
	GSErrCode err = NoError;

	switch (menuParams->menuItemRef.menuResID) {
		case NAVIGATOR_TEST_MENU_STRINGS:
			switch (menuParams->menuItemRef.itemIndex) {
				case NavigatorTestOpenNavigatorId:
					NavigatorUtility::OpenAPINavigator ();
					break;
				case NavigatorTestCreateChildGroupId:
					NavigatorUtility::CreateNavigatorChildGroup ();
					break;
				case NavigatorTestCloneProjectMapToViewMap: {
					err = NavigatorUtility::CloneProjectMapToViewMap ();
					DBASSERT (err == NoError);
				}	break;
			}
	}

	return err;
}


// ============================================================================
// Required functions
//
//
// ============================================================================


//------------------------------------------------------
// Dependency definitions
//------------------------------------------------------
API_AddonType __ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (&envir->addOnInfo.name, NAVIGATOR_TEST_ADDON_NAME, 1, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, NAVIGATOR_TEST_ADDON_NAME, 2, ACAPI_GetOwnResModule ());

	return APIAddon_Preload;
}		/* CheckEnvironment */


//------------------------------------------------------
// Interface definitions
//------------------------------------------------------
GSErrCode __ACENV_CALL	RegisterInterface (void)
{
	GSErrCode	err = NoError;

	// Register menus
	err = ACAPI_Register_Menu (NAVIGATOR_TEST_MENU_STRINGS, 0, MenuCode_UserDef, MenuFlag_InsertIntoSame);
	if (err != NoError)
		ACAPI_WriteReport ("ACAPI_Install_NavigatorAddOnViewPointDataMergeHandler returned error", false);

	if (err == NoError) {
		err = ACAPI_Register_NavigatorAddOnViewPointDataHandler ();
		if (err != NoError)
			ACAPI_WriteReport ("ACAPI_Install_NavigatorAddOnViewPointDataMergeHandler returned error", false);
	}

	return err;
}		/* RegisterInterface */


//------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
//------------------------------------------------------
GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err = NoError;

	err = ACAPI_Install_NavigatorAddOnViewPointDataMergeHandler (NavigatorUtility::APINavigatorAddOnViewPointDataMergeHandler);
	if (err != NoError)
		ACAPI_WriteReport ("ACAPI_Install_NavigatorAddOnViewPointDataMergeHandler returned error", false);

	if (err == NoError) {
		err = ACAPI_Install_NavigatorAddOnViewPointDataSaveOldFormatHandler (NavigatorUtility::APINavigatorAddOnViewPointDataSaveOldFormatHandler);
		if (err != NoError)
			ACAPI_WriteReport ("ACAPI_Install_NavigatorAddOnViewPointDataSaveOldFormatHandler returned error", false);
	}

	if (err == NoError) {
		err = ACAPI_Install_NavigatorAddOnViewPointDataConvertNewFormatHandler (NavigatorUtility::APINavigatorAddOnViewPointDataConvertNewFormatHandler);
		if (err != NoError)
			ACAPI_WriteReport ("ACAPI_Install_NavigatorAddOnViewPointDataConvertNewFormatHandler returned error", false);
	}

	if (err == NoError) {
		err = ACAPI_Install_MenuHandler (NAVIGATOR_TEST_MENU_STRINGS, MenuCommandHandler);
		if (err != NoError)
			ACAPI_WriteReport ("ACAPI_Install_MenuHandler returned error", false);
	}

	if (err == NoError) {
		err = ACAPI_Navigator (APINavigator_RegisterCallbackInterfaceID, &NavigatorUtility::getNavigatorCallbackInterface ());
		if (err != NoError)
			ACAPI_WriteReport ("APINavigator_RegisterCallbackInterfaceID returned error", false);
	}

	if (err == NoError) {
		err = ACAPI_Notify_CatchProjectEvent (APINotify_AllInputFinished | APINotify_ReceiveChanges, NotificationHandler);
		if (err != NoError)
			ACAPI_WriteReport ("ACAPI_Notify_CatchProjectEvent returned error", false);
	}

	return err;
}		/* Initialize */


//------------------------------------------------------
// Called when the Add-On is going to be unloaded
//------------------------------------------------------
GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}		/* FreeData */
