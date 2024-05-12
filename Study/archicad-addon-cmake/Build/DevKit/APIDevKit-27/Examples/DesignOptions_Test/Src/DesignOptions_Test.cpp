#include "Resources.hpp"

#include <queue>

// from GS
#include "Overloaded.hpp"

// from DG
#include "DG.h"
                       
// from API
#include "APIEnvir.h"
#include "ACAPinc.h"
#include "APICommon.h"

// from ACAPI
#include "ACAPI/Result.hpp"
#include "ACAPI_Navigator.h"
#include "ACAPI/View.hpp"
#include "ACAPI/DesignOptionManager.hpp"

namespace {

using namespace ACAPI::v2;

ACAPI::Result<std::vector<API_NavigatorItem>> GetLeafNavigatorItems (API_NavigatorMapID mapID)
{
	Int32 i = 0;
	API_NavigatorSet set = {};
	set.mapId = mapID;

	if (ACAPI_Navigator_GetNavigatorSet (&set, &i) != NoError) {
		return { ACAPI::Error (APIERR_GENERAL, "Failed to retrieve the list of items in the navigator."), ACAPI_GetToken () };
	}

	API_NavigatorItem rootItem = {};
	rootItem.guid = set.rootGuid;
	rootItem.mapId = mapID;

	std::queue<API_NavigatorItem> itemsToProcess;
	itemsToProcess.emplace (rootItem);

	std::vector<API_NavigatorItem> result;

	while (!itemsToProcess.empty ()) {
		GS::Array<API_NavigatorItem> childItems;
		API_NavigatorItem parentItem = itemsToProcess.front ();
		itemsToProcess.pop ();
		GSErrCode err = ACAPI_Navigator_GetNavigatorChildrenItems (&parentItem, &childItems);
		if (err != NoError) {
			continue;
		}

		for (const auto& childItem : childItems) {
			if (childItem.itemType == API_FolderNavItem || childItem.itemType == API_UndefinedNavItem) {
				itemsToProcess.emplace (childItem);
			} else {
				result.emplace_back (childItem);
			}
		}
	}

	return ACAPI::Ok (result);
}

void DumpDesignOptionCombinations ()
{
	const ACAPI::Result<std::vector<API_NavigatorItem>> mapItems = GetLeafNavigatorItems (API_PublicViewMap);
	if (mapItems.IsErr ()) {
		WriteReport ("Failed to get navigator map items.");
		return;
	}

	// ! [Get-View-Design-Option-Combination-Example]

	ACAPI::Result<ACAPI::DesignOptionManager> manager = CreateDesignOptionManager ();
	if (manager.IsErr ()) {
		WriteReport ("Failed to get Design Option Manager.");
		return;
	}

	for (const API_NavigatorItem& item : mapItems.Unwrap ()) {
		const ACAPI::Result<ACAPI::View> view = ACAPI::FindViewByGuid (item.guid);
		if (view.IsErr ()) {
			continue;
		}

		const ACAPI::Result<ACAPI::DesignOptionCombinationViewSettings> settings =
			manager->GetDesignOptionCombinationSettingsOf (view.Unwrap ());
		if (settings.IsErr ()) {
			continue;
		}

		const ACAPI::Result<ACAPI::DesignOptionCombinationViewSettings::Status> status = settings->GetStatus ();
		if (status.IsErr ()) {
			continue;
		}

		const GS::UniString itemId (item.uiId);
		const GS::UniString itemIdPrefix = !itemId.IsEmpty () ? itemId + " " : GS::EmptyUniString;
		const GS::UniString itemName (item.uName);
		const GS::UniString fullName = itemIdPrefix + itemName;
		std::visit (GS::Overloaded {
			[&fullName] (const ACAPI::DesignOptionCombinationViewSettings::MainModelOnly&) {
				WriteReport ("%s: Main Model Only.", fullName.ToCStr ().Get ());
			},
			[&fullName] (const ACAPI::DesignOptionCombinationViewSettings::Standard& standard) {
				WriteReport ("%s: %s.", fullName.ToCStr ().Get (), standard.combination.GetName ().ToCStr ().Get ());
			},
			[&fullName] (const ACAPI::DesignOptionCombinationViewSettings::Custom& /*custom*/) {
				WriteReport ("%s: Custom.", fullName.ToCStr ().Get ());
			},
			[&fullName] (const ACAPI::DesignOptionCombinationViewSettings::Missing&) {
				WriteReport ("%s: Missing.", fullName.ToCStr ().Get ());
			},
		}, status.Unwrap ());
	}

	// ! [Get-View-Design-Option-Combination-Example]
}

void SetAllDesignOptionCombinationsToMainModelOnly ()
{
	// ! [Set-View-Design-Option-Combination-Example]

	const ACAPI::Result<std::vector<API_NavigatorItem>> mapItems = GetLeafNavigatorItems (API_PublicViewMap);
	if (mapItems.IsErr ()) {
		WriteReport ("Failed to get navigator map items.");
		return;
	}

	ACAPI::Result<ACAPI::DesignOptionManager> manager = CreateDesignOptionManager ();
	if (manager.IsErr ()) {
		WriteReport ("Failed to get Design Option Manager.");
		return;
	}

	for (const API_NavigatorItem& item : mapItems.Unwrap ()) {
		const ACAPI::Result<ACAPI::View> view = ACAPI::FindViewByGuid (item.guid);
		if (view.IsErr ()) {
			continue;
		}

		ACAPI::Result<ACAPI::DesignOptionCombinationViewSettings> settings =
			manager->GetDesignOptionCombinationSettingsOf (view.Unwrap ());
		if (settings.IsErr ()) {
			continue;
		}

		settings->Modify ([] (ACAPI::DesignOptionCombinationViewSettings::Modifier& modifier) {
			modifier.SetMainModelOnly ();
		});
	}

	// ! [Set-View-Design-Option-Combination-Example]
}

void DumpAvailableDesignOptionCombinations ()
{
	// ! [Get-Design-Option-Combinations-Example]

	ACAPI::Result<ACAPI::DesignOptionManager> manager = CreateDesignOptionManager ();
	if (manager.IsErr ()) {
		WriteReport ("Failed to get Design Option Manager.");
		return;
	}

	const auto combinations = manager->GetAvailableDesignOptionCombinations ();
	if (combinations.IsErr ()) {
		WriteReport ("Failed to get available design option combinations.");
		return;
	}

	for (const auto& combination : combinations.Unwrap ()) {
		WriteReport ("%s (%s)", combination.GetName ().ToCStr ().Get (), combination.GetGuid ().ToUniString ().ToCStr ().Get ());
	}

	// ! [Get-Design-Option-Combinations-Example]
}

}


// -----------------------------------------------------------------------------
// MenuCommandHandler
//		called to perform the user-asked command
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL MenuCommandHandler (const API_MenuParams* menuParams)
{
	switch (menuParams->menuItemRef.menuResID) {
		case DESIGNOPTIONS_TEST_MENU_STRINGS: {
			switch (menuParams->menuItemRef.itemIndex) {
				case DumpViewMapDesignOptionCombinationsID:
					DumpDesignOptionCombinations ();
					return NoError;
				case SetAllDesignOptionCombinationsToMainModelOnlyID:
					SetAllDesignOptionCombinationsToMainModelOnly ();
					return NoError;
				case DumpAvailableDesignOptionCombinationsID:
					DumpAvailableDesignOptionCombinations ();
					return NoError;
				default:
					DBBREAK ();
					return Error;
			}
			break;
		}
	}

	return NoError;
}

// =============================================================================
//
// Required functions
//
// =============================================================================

// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------

API_AddonType __ACDLL_CALL CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (&envir->addOnInfo.name,        DESIGNOPTIONS_TEST_ADDON_NAME, 1, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, DESIGNOPTIONS_TEST_ADDON_NAME, 2, ACAPI_GetOwnResModule ());

	return APIAddon_Normal;
}


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode __ACDLL_CALL RegisterInterface ()
{
	GSErrCode err = ACAPI_MenuItem_RegisterMenu (DESIGNOPTIONS_TEST_MENU_STRINGS, DESIGNOPTIONS_TEST_MENU_PROMPT_STRINGS, MenuCode_UserDef, MenuFlag_Default);
	DBASSERT (err == NoError);
	return err;
}


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL Initialize ()
{
	GSErrCode err = ACAPI_MenuItem_InstallMenuHandler (DESIGNOPTIONS_TEST_MENU_STRINGS, MenuCommandHandler);
	DBASSERT (err == NoError);
	ACAPI_KeepInMemory (true);
	return err;
}


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL FreeData ()
{
	return NoError;
}
