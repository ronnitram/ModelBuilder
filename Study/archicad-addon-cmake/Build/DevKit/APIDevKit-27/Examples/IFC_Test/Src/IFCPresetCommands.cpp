#include "APIEnvir.h"
#include "ACAPinc.h"		// also includes APIdefs.h

#include "IFCPresetCommands.hpp"

#include "Owner.hpp"
#include "DGModule.hpp"
#include "MDCLParams.hpp"
#include "Pair.hpp"
#include "Queue.hpp"
#include "StringConversion.hpp"
#include "FixArray.hpp"

#include "DG.h"

void CreatePreset ()
{
	GS::Array<GS::Pair<GS::String, GS::UniString>> presetsToCreate = {
		{ "TypeMappingImportPreset",  "Test Type Mapping Import Preset Name" },
		{ "TypeMappingExportPreset",  "Test Type Mapping Export Preset Name - IFC2X3" },
		{ "TypeMappingExportPreset",  "Test Type Mapping Export Preset Name" },
		{ "PropertyMappingExportPreset",  "Test Property Mapping Export Preset Name" },
		{ "PropertyMappingImportPreset",  "Test Property Mapping Import Preset Name" },
	};

	for (const auto& presetToCreate : presetsToCreate) {
		GS::String presetType = presetToCreate.first;
		GS::UniString presetName = presetToCreate.second;

		GSAPI::MDCLParams params;
		params.SetString ("presetType", presetType.ToCStr ());
		params.SetPtr ("presetName", &presetName);

		GSErrCode err = ACAPI_AddOnAddOnCommunication_Call (&IfcMdid, CREATEPRESET, 1L, params.GetParams (), nullptr, false);
		if (err == NoError) {
			DG::InformationAlert (presetName + " (" + presetType + ") preset was successfully created!", GS::UniString (), "OK");
		} else {
			DG::WarningAlert (presetName + " (" + presetType + ") preset could not be created (err : " + GS::ValueToUniString (err) + ")!", GS::UniString (), "OK");
			GS::UniString report = "IFCPresetCommands_Test CreatePreset failed (err : " + GS::ValueToUniString (err) + ")!\n";
			ACAPI_WriteReport (report, false/*withDial*/);
		}
	}
}


void HasPreset ()
{
	GS::String presetType = "TypeMappingImportPreset";
	GS::UniString presetName = "Test Type Mapping Import Preset Name";

	GSAPI::MDCLParams params;
	params.SetString ("presetType", presetType.ToCStr ());
	params.SetPtr ("presetName", &presetName);

	bool presetFound = false;
	GSErrCode err = ACAPI_AddOnAddOnCommunication_Call (&IfcMdid, HASPRESET, 1L, params.GetParams (), reinterpret_cast<GSPtr> (&presetFound), false);
	if (presetFound && err == NoError) {
		DG::InformationAlert (presetName + " was found!", GS::UniString (), "OK");
	} else {
		DG::WarningAlert (presetName + " was not found (err : " + GS::ValueToUniString (err) + ")!", GS::UniString (), "OK");
		if (err != NoError) {
			GS::UniString report = "IFCPresetCommands_Test HasPreset failed (err : " + GS::ValueToUniString (err) + ")!\n";
			ACAPI_WriteReport (report, false/*withDial*/);
		}
	}
}


void ClearPreset ()
{
	GS::String presetType = "TypeMappingExportPreset";
	GS::UniString presetName = "Test Type Mapping Export Preset Name";

	GSAPI::MDCLParams params;
	params.SetString ("presetType", presetType.ToCStr ());
	params.SetPtr ("presetName", &presetName);

	GSErrCode err = ACAPI_AddOnAddOnCommunication_Call (&IfcMdid, CLEARPRESET, 1L, params.GetParams (), nullptr, false);
	if (err == NoError) {
		DG::InformationAlert (presetName + " export type mapping preset was successfully cleared!", GS::UniString (), "OK");
	} else {
		DG::WarningAlert (presetName + " export type mapping preset could not be cleared (err : " + GS::ValueToUniString (err) + ")!", GS::UniString (), "OK");
		GS::UniString report = "IFCPresetCommands_Test ClearPreset failed (err : " + GS::ValueToUniString (err) + ")!\n";
		ACAPI_WriteReport (report, false/*withDial*/);
	}
}


void AddNewTypeMappingForImport ()
{
	GS::UniString presetName = "Test Type Mapping Import Preset Name";

	GS::Array<API_ClassificationItem> firstSystemRootItems;
	{
		GS::Array<API_ClassificationSystem> classificationSystems;
		GSErrCode err = ACAPI_Classification_GetClassificationSystems (classificationSystems);
		if (err != NoError || classificationSystems.GetSize () < 1) {
			DG::WarningAlert ("No classification systems found for " + presetName + " import type mapping preset.", GS::UniString (), "OK");
			return; //error or no system found
		}

		err = ACAPI_Classification_GetClassificationSystemRootItems (classificationSystems[0].guid, firstSystemRootItems);
		if (err != NoError || firstSystemRootItems.GetSize () < 1) {
			DG::WarningAlert ("No classification items found for " + presetName + " import type mapping preset.", GS::UniString (), "OK");
			return; //error or no item found
		}
	}

	const GS::Array<GS::FixArray<GS::String, 3>> importIfcMappingsToAdd = {
		/* ifcType		ifcPredefinedType	ifcUserDefinedType*/
		{ "IfcWall",	"",					"" },
		{ "IfcWall",	"SHEAR",			"" },
		{ "IfcWall",	"USERDEFINED",		"CustomWall" },
		{ "IfcRailing",	"BALUSTRADE",		"" },
		{ "IfcRoof",	"",					"" }
	};

	GS::Array<GSHandle>					newImportTypeMappingParamHandles;
	GS::Array<Owner<GSAPI::MDCLParams>>	newImportTypeMappingParams; //register MDCLParams allocated on heap, to delete them at the end of scope
	for (const GS::FixArray<GS::String, 3>& mapping : importIfcMappingsToAdd) {
		GSAPI::MDCLParams* params = new GSAPI::MDCLParams ();
		params->SetString ("ifcType", mapping[0].ToCStr ());
		params->SetString ("ifcPredefinedType", mapping[1].ToCStr ());
		params->SetString ("ifcUserDefinedType", mapping[2].ToCStr ());
		params->SetString ("classificationItemGuid", APIGuidToString (firstSystemRootItems[rand () % firstSystemRootItems.GetSize ()].guid).ToCStr ().Get ());
		newImportTypeMappingParamHandles.Push (params->GetParams ());
		newImportTypeMappingParams.PushNew (params);
	}

	GSAPI::MDCLParams params;
	params.SetPtr ("presetName", &presetName);
	params.SetPtr ("newMappings", &newImportTypeMappingParamHandles);

	GSErrCode err = ACAPI_AddOnAddOnCommunication_Call (&IfcMdid, ADDNEWTYPEMAPPINGFORIMPORT, 1L, params.GetParams (), nullptr, false);
	if (err == NoError) {
		DG::InformationAlert (presetName + " import type mapping preset was successfully modified!", GS::UniString (), "OK");
	} else {
		DG::WarningAlert (presetName + " import type mapping preset could not be modified (err : " + GS::ValueToUniString (err) + ")!", GS::UniString (), "OK");
		GS::UniString report = "IFCPresetCommands_Test AddNewTypeMappingForImport failed (err : " + GS::ValueToUniString (err) + ")!\n";
		ACAPI_WriteReport (report, false/*withDial*/);
	}
}


void AddNewTypeMappingForExport (const GS::UniString& ifcSchema /* = "" */)
{
	GS::UniString	presetName = "Test Type Mapping Export Preset Name";
	GS::UniString	ifcSchemaName = ifcSchema.ToUpperCase ();

	if (!ifcSchemaName.IsEmpty ()) {
		presetName += " - " + ifcSchemaName;
	}

	GS::Queue<API_ClassificationItem> firstSystemItems;
	{
		GS::Array<API_ClassificationSystem> classificationSystems;
		GSErrCode err = ACAPI_Classification_GetClassificationSystems (classificationSystems);
		if (err != NoError || classificationSystems.GetSize () < 1) {
			DG::WarningAlert ("No classification systems found for " + presetName + " import type mapping preset.", GS::UniString (), "OK");
			return; //error or no system found
		}

		GS::Array<API_ClassificationItem> itemsToProcess;
		ACAPI_Classification_GetClassificationSystemRootItems (classificationSystems[0].guid, itemsToProcess);
		while (!itemsToProcess.IsEmpty ()) {
			API_ClassificationItem item = itemsToProcess.Pop ();
			firstSystemItems.Push (item);
			GS::Array<API_ClassificationItem> children;
			ACAPI_Classification_GetClassificationItemChildren (item.guid, children);
			itemsToProcess.Append (children);
		}
	}

	const GS::Array<GS::FixArray<GS::String, 3>> exportIfcMappingsToAdd = {
		/* ifcType		ifcPredefinedType	ifcUserDefinedType*/
		{ "IfcWall",	"",					"" },
		{ "IfcWall",	"SHEAR",			"" },
		{ "IfcWall",	"USERDEFINED",		"CustomWall" },
		{ "IfcRailing",	"BALUSTRADE",		"" },
		{ "IfcRoof",	"",					"" }
	};

	GS::Array<GSHandle>					newExportTypeMappingParamHandles;
	GS::Array<Owner<GSAPI::MDCLParams>>	newExportTypeMappingParams; //register MDCLParams allocated on heap, to delete them at the end of scope
	for (const API_ClassificationItem& item : firstSystemItems) {
		if (rand () % 2 == 0) {
			continue; //map every second item
		}
		const GS::FixArray<GS::String, 3>& randomTypeMapping = exportIfcMappingsToAdd[rand () % exportIfcMappingsToAdd.GetSize ()];
		GSAPI::MDCLParams* params = new GSAPI::MDCLParams ();
		params->SetString ("classificationItemGuid", APIGuidToString (item.guid).ToCStr ().Get ());
		params->SetString ("ifcType", randomTypeMapping[0].ToCStr ());
		params->SetString ("ifcPredefinedType", randomTypeMapping[1].ToCStr ());
		params->SetString ("ifcUserDefinedType", randomTypeMapping[2].ToCStr ());
		newExportTypeMappingParamHandles.Push (params->GetParams ());
		newExportTypeMappingParams.PushNew (params);
	}

	GSAPI::MDCLParams params;
	params.SetPtr ("presetName", &presetName);
	params.SetPtr ("ifcSchema", &ifcSchemaName);
	params.SetPtr ("newMappings", &newExportTypeMappingParamHandles);

	GSErrCode err = ACAPI_AddOnAddOnCommunication_Call (&IfcMdid, ADDNEWTYPEMAPPINGFOREXPORT, 1L, params.GetParams (), nullptr, false);
	if (err == NoError) {
		DG::InformationAlert (presetName + " export property mapping preset was successfully modified!", GS::UniString (), "OK");
	} else {
		DG::WarningAlert (presetName + " export property mapping preset could not be modified (err : " + GS::ValueToUniString (err) + ")!", GS::UniString (), "OK");
		GS::UniString report = "IFCPresetCommands_Test AddNewTypeMappingForExport failed (err : " + GS::ValueToUniString (err) + ")!\n";
		ACAPI_WriteReport (report, false/*withDial*/);
	}
}


void AddNewPropertyMappingForExport ()
{
	GS::UniString presetName = "Test Property Mapping Export Preset Name";

	GS::Array<API_PropertyGroup> propertyGroups;
	GSErrCode err = ACAPI_Property_GetPropertyGroups (propertyGroups);
	UIndex firstCustomPropertyGroupIndex = propertyGroups.FindFirst ([] (const API_PropertyGroup& group) {
		return (group.groupType == API_PropertyCustomGroupType);
	});
	if (err != NoError || firstCustomPropertyGroupIndex == MaxUIndex) {
		DG::WarningAlert ("No property groups found for " + presetName + " export property mapping preset.", GS::UniString (), "OK");
		return; //error or no system found
	}

	GS::Array<API_PropertyDefinition> firstGroupsPropertyDefinitions;
	err = ACAPI_Property_GetPropertyDefinitions (propertyGroups[firstCustomPropertyGroupIndex].guid, firstGroupsPropertyDefinitions);
	if (err != NoError || firstGroupsPropertyDefinitions.GetSize () < 1) {
		DG::WarningAlert ("No property definitions found for " + presetName + " export property mapping preset.", GS::UniString (), "OK");
		return; //error or no item found
	}

	GS::Array<GSHandle>					newPropertyMappingParamHandles;
	GS::Array<Owner<GSAPI::MDCLParams>>	newPropertyMappingParams; //register MDCLParams allocated on heap, to delete them at the end of scope
	GS::Array<Owner<API_IFCAttribute>>	ifcAttributes; //register API_IFCAttribute allocated on heap, to delete them at the end of scope
	GS::Array<Owner<API_IFCProperty>>	ifcProperties; //register API_IFCProperty allocated on heap, to delete them at the end of scope

															   //map attribute
	const GS::Array<GS::String> ifcAttributeNames = { "Description", "ObjectType" };
	for (const GS::String& attributeName : ifcAttributeNames) {
		GSAPI::MDCLParams* params = new GSAPI::MDCLParams ();
		params->SetString ("ifcType", "IfcWall");
		params->SetString ("schemeItemToMapType", "Attribute");
		params->SetString ("acPropertyGuid", APIGuidToString (firstGroupsPropertyDefinitions[0].guid).ToCStr ().Get ());
		API_IFCAttribute* ifcAttribute = new API_IFCAttribute ();
		ifcAttribute->attributeName = attributeName;
		params->SetPtr ("ifcAttribute", ifcAttribute);
		newPropertyMappingParamHandles.Push (params->GetParams ());
		newPropertyMappingParams.PushNew (params);
		ifcAttributes.PushNew (ifcAttribute);
	}

	//map properties
	const GS::Array<GS::String> ifcTypes = { "IfcColumn", "IfcCovering", "IfcDoor", "IfcMember", "IfcRoof", "IfcSlab", "IfcStair", "IfcWall", "IfcWallStandardCase", "IfcWindow" };
	for (UIndex i = 1; i < 100; ++i) {
		GSAPI::MDCLParams* params = new GSAPI::MDCLParams ();
		params->SetString ("ifcType", ifcTypes[rand () % ifcTypes.GetSize ()].ToCStr ());
		params->SetString ("schemeItemToMapType", "Property");
		params->SetString ("acPropertyGuid", APIGuidToString (firstGroupsPropertyDefinitions[0].guid).ToCStr ().Get ());
		API_IFCProperty* ifcProperty = new API_IFCProperty ();
		ifcProperty->head.propertySetName = "CustomPset_Test";
		ifcProperty->head.propertyName = "TestIFCProperty " + GS::ValueToUniString (i);
		ifcProperty->head.propertyType = API_IFCPropertySingleValueType;
		ifcProperty->singleValue.nominalValue.value.intValue = 0;
		ifcProperty->singleValue.nominalValue.valueType = "IfcInteger";
		params->SetPtr ("ifcProperty", ifcProperty);
		newPropertyMappingParamHandles.Push (params->GetParams ());
		newPropertyMappingParams.PushNew (params);
		ifcProperties.PushNew (ifcProperty);
	}

	GSAPI::MDCLParams params;
	params.SetPtr ("presetName", &presetName);
	params.SetPtr ("newMappings", &newPropertyMappingParamHandles);

	err = ACAPI_AddOnAddOnCommunication_Call (&IfcMdid, ADDNEWPROPERTYMAPPINGFOREXPORT, 1L, params.GetParams (), nullptr, false);
	if (err == NoError) {
		DG::InformationAlert (presetName + " export property mapping preset was successfully modified!", GS::UniString (), "OK");
	} else {
		DG::WarningAlert (presetName + " export property mapping preset could not be modified (err : " + GS::ValueToUniString (err) + ")!", GS::UniString (), "OK");
		GS::UniString report = "IFCPresetCommands_Test AddNewPropertyMappingForExport failed (err : " + GS::ValueToUniString (err) + ")!\n";
		ACAPI_WriteReport (report, false/*withDial*/);
	}
}


void AddNewPropertyMappingForImport ()
{
	GS::UniString presetName = "Test Property Mapping Import Preset Name";

	GS::Array<API_PropertyGroup> propertyGroups;
	ACAPI_Property_GetPropertyGroups (propertyGroups);

	UIndex firstCustomPropertyGroup = propertyGroups.FindFirst ([](const API_PropertyGroup& group) {
		return group.groupType == API_PropertyGroupType::API_PropertyCustomGroupType;
	});

	GS::Array<API_PropertyDefinition> propertyDefinitions;
	ACAPI_Property_GetPropertyDefinitions (propertyGroups[firstCustomPropertyGroup].guid, propertyDefinitions);

	// It's always possible to map values to String AC Properties... availability of other types depends on the given IFC types.
	// So for easy demonstration, we select only String AC Properties to the mapping table.
	GS::Array<API_PropertyDefinition> stringPropertyDefinitions;
	for (const API_PropertyDefinition& prop : propertyDefinitions) {
		if (prop.collectionType == API_PropertySingleCollectionType && prop.valueType == API_PropertyStringValueType && prop.measureType == API_PropertyDefaultMeasureType) {
			stringPropertyDefinitions.Push (prop);
		}
	}

	const GS::Array<GS::Array<GS::String>> importIfcMappingsToAdd = {
		{"Thickness"      , "FirstSet"     , "IfcBoolean"					 , "SingleValue"},
		{"Flameproofness" , "FirstSet"     , "IfcLabel"						 , "SingleValue"},
		{"Bulletproofness", "SecondSet"    , "IfcText"						 , "BoundedValue"},
		{"Waterproofness" , "SecondSet"    , "IfcMoistureDiffusivityMeasure" , "SingleValue"},
		{"Soundproofness" , ""/*Any set!*/ , "IfcCalendarDate"				 , "ReferenceValue"},
		{"Foolproofness"  , ""/*Any set!*/ , ""/*Any type!*/				 , ""/*Any value!*/}
	};

	GS::Array<GSHandle>					newImportTypeMappingParamHandles;
	GS::Array<Owner<GSAPI::MDCLParams>>	newImportTypeMappingParams; //register MDCLParams allocated on heap, to delete them at the end of scope
	for (const GS::Array<GS::String>& mapping : importIfcMappingsToAdd) {
		GSAPI::MDCLParams* params = new GSAPI::MDCLParams ();
		params->SetString ("ifcPropertyName", mapping[0].ToCStr ());
		params->SetString ("ifcPropertySetName", mapping[1].ToCStr ());
		params->SetString ("ifcPropertyValueType", mapping[2].ToCStr ());
		params->SetString ("ifcPropertyType", mapping[3].ToCStr ());

		API_Guid propertyGuid = stringPropertyDefinitions[rand () % stringPropertyDefinitions.GetSize ()].guid;
		params->SetString ("acPropertyGuid", APIGuidToString (propertyGuid).ToCStr ());
		newImportTypeMappingParamHandles.Push (params->GetParams ());
		newImportTypeMappingParams.PushNew (params);
	}

	GSAPI::MDCLParams params;
	params.SetPtr ("presetName", &presetName);
	params.SetPtr ("newMappings", &newImportTypeMappingParamHandles);

	GSErrCode err = ACAPI_AddOnAddOnCommunication_Call (&IfcMdid, ADDNEWPROPERTYMAPPINGFORIMPORT, 1L, params.GetParams (), nullptr, false);
	if (err == NoError) {
		DG::InformationAlert (presetName + " import property mapping preset was successfully modified!", GS::UniString (), "OK");
	} else {
		DG::WarningAlert (presetName + " import property mapping preset could not be modified (err : " + GS::ValueToUniString (err) + ")!", GS::UniString (), "OK");
		GS::UniString report = "IFCPresetCommands_Test AddNewPropertyMappingForImport failed (err : " + GS::ValueToUniString (err) + ")!\n";
		ACAPI_WriteReport (report, false/*withDial*/);
	}
}
