#include "APIEnvir.h"
#include "ACAPinc.h"		// also includes APIdefs.h

#include "IFCDifference.hpp"
#include "Resources.hpp"

#include "DGModule.hpp"
#include "GSRoot.hpp"
#include "StringConversion.hpp"

#include "APICommon.h"

static API_IFCTranslatorIdentifier		ifcTranslator;
static API_IFCDifferenceGeneratorState	storedDiffGeneratorState;
static API_Guid							previousViewGuid;
static API_Guid							viewGuid;


// -----------------------------------------------------------------------------
//	Utility functions
// -----------------------------------------------------------------------------


static GS::UniString APIGuidToIFCGuid (const API_Guid& apiGuid)
{
	GS::UniString ifcGuid;
	ACAPI_IFC_APIGuidToIFCGuid (apiGuid, ifcGuid);

	return ifcGuid;
}


static void SelectDifferencedElements (API_IFCDifference &result)
{
	GS::Array<API_Neig> selNeigs;
	for (auto& elem_guid : result.newIFCEntities) {
		if (elem_guid.elementID != APINULLGuid) {
			API_Neig neig (elem_guid.elementID);
			selNeigs.Push (neig);
		}
	}
	for (auto& elem_guid : result.modifiedIFCEntities) {
		if (elem_guid.elementID != APINULLGuid) {
			API_Neig neig (elem_guid.elementID);
			selNeigs.Push (neig);
		}
	}
	ACAPI_Selection_Select (selNeigs, true);
}


static GS::UniString GetElementType (const API_Guid& guid)
{
	if (guid == APINULLGuid)
		return GS::UniString ("");

	API_Elem_Head elemHead = {};
	BNZeroMemory (&elemHead, sizeof (API_Elem_Head));
	elemHead.guid = guid;

	if (ACAPI_Element_GetHeader (&elemHead) != NoError)
		return GS::UniString ("");

	GS::UniString result (" - ");
	result.Append (ElemID_To_Name (elemHead.type));
	return result;
}


static GS::UniString GetIFCType (const API_Guid& guid)
{
	if (guid == APINULLGuid)
		return GS::UniString ("");

	GS::UniString ifcType;
	GS::UniString typeObjectIFCType;
	GSErrCode err = ACAPI_Element_GetIFCType (guid, &ifcType, &typeObjectIFCType);
	if (err != NoError)
		return GS::UniString ("");

	GS::UniString result (" - ");
	result.Append (ifcType);

	return result;
}


static void WriteReportAboutIFCDifferencedElements (API_IFCDifference &result)
{
	GS::UniString report = "API_IFCDifference:";
	report += "\n\t\tIs Full sync needed: " + (!result.isValid ? GS::UniString ("true") : GS::UniString ("false")) + "\n";
	report += "\n\t\tNew Elements:\n";
	if (result.isValid) {
		for (auto& elem : result.newIFCEntities) {
			report += "\t\t\t" + APIGuidToIFCGuid (elem.ifcGlobalId) + GetIFCType (elem.elementID) + " " + APIGuidToString (elem.elementID) + GetElementType (elem.elementID) + "\n";
		}
	} else {
		report += "\t\t\tBecause It is Full Sync needed, elements difference not calculated.\n";
	}
	report += "\n\t\tModified Elements:\n";
	if (result.isValid) {
		for (auto& elem : result.modifiedIFCEntities) {
			report += "\t\t\t" + APIGuidToIFCGuid (elem.ifcGlobalId) + GetIFCType (elem.elementID) + " " + APIGuidToString (elem.elementID) + GetElementType (elem.elementID) + "\n";
		}
	} else {
		report += "\t\t\tBecause It is Full Sync needed, elements difference not calculated.\n";
	}
	report += "\n\t\tDeleted Elements:\n";
	if (result.isValid) {
		for (auto& elem : result.deletedIFCEntities) {
			report += "\t\t\t" + APIGuidToIFCGuid (elem.ifcGlobalId) + " " + APIGuidToString (elem.elementID) + "\n";
		}
	} else {
		report += "\t\t\tBecause It is Full Sync needed, elements difference not calculated.\n";
	}
	ACAPI_WriteReport (report, false);
}


// -----------------------------------------------------------------------------
//	Create and IFCDifference
// -----------------------------------------------------------------------------


void ShowIFCDifference ()
{
	if (storedDiffGeneratorState.environmentChecksum == APINULLGuid) {
		DG::WarningAlert ("Please store an IFCSnapshot before this step.", GS::UniString (), "OK");
		return;
	}

	API_IFCDifferenceGeneratorState currentIFCDiffGeneratorState;
	GSErrCode err = ACAPI_IFC_GetIFCDifferenceState (viewGuid, ifcTranslator, currentIFCDiffGeneratorState);

	if (DBVERIFY (err == NoError)) {
		API_IFCDifference result;
		if (DBVERIFY (ACAPI_IFC_GetIFCDifference (storedDiffGeneratorState, currentIFCDiffGeneratorState, result) == NoError)) {
			DG::InformationAlert ("IFCDifference succesfully created\nSee Session Report for more details.", GS::UniString (), "OK");
			WriteReportAboutIFCDifferencedElements (result);
			SelectDifferencedElements (result);
		}
	} else {
		DG::WarningAlert ("Couldn't create IFCDifference.", GS::UniString (), "OK");
		GS::UniString report = "IFCDifference_Test: Creating IFCDifference failed... (err : " + GS::ValueToUniString (err) + ")!\n";
		ACAPI_WriteReport (report, false);
	}
}


// -----------------------------------------------------------------------------
//	Create and Store current IFCSnapshot
// -----------------------------------------------------------------------------


void StoreCurrentIFCDifferenceState ()
{
	if (viewGuid == APINULLGuid || ifcTranslator.innerReference == nullptr) {
		InvokeExportSettingsDialog ();
		if (viewGuid == APINULLGuid || ifcTranslator.innerReference == nullptr) {
			DG::WarningAlert ("Please select a valid View and IFC Translator before this step.", GS::UniString (), "OK");
			return;
		}
	}

	GSErrCode err = ACAPI_IFC_GetIFCDifferenceState (viewGuid, ifcTranslator, storedDiffGeneratorState);
	if (DBVERIFY (err == NoError)) {
		previousViewGuid = viewGuid;
		DG::InformationAlert ("Current IFCSnapshot succesfully created.", GS::UniString (), "OK");
	} else {
		DG::WarningAlert ("Couldn't create IFCSnapshot.", GS::UniString (), "OK");
		GS::UniString report = "IFCDifference_Test: Creating Current IFCSnapshot failed... (err : " + GS::ValueToUniString (err) + ")!\n";
		ACAPI_WriteReport (report, false);
	}
}


// -----------------------------------------------------------------------------
//	Set ViewGuid and IFC Translator
// -----------------------------------------------------------------------------


void InvokeExportSettingsDialog ()
{
	API_IFCTranslatorIdentifier tmpIFCTranslator = ifcTranslator;

	GSErrCode err = ACAPI_IFC_InvokeIFCDifferenceExportSettingsDlg (viewGuid, tmpIFCTranslator);
	if (err == NoError) {
		BMhKill (&ifcTranslator.innerReference);
		ifcTranslator = tmpIFCTranslator;
	}
}


void InitIFCDifference ()
{
	ifcTranslator.innerReference							= nullptr;
	viewGuid												= APINULLGuid;
}


void FreeIFCDifference ()
{
	BMhKill (&ifcTranslator.innerReference);
}
