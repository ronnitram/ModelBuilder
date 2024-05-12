#include "APIEnvir.h"
#include "ACAPinc.h"
#include "ACAPI_Environment.h"
#include "APICommon.h"
#include "PropertyList.hpp"
#include <set>
#include <map>

#include "ResourceIds.hpp"
#include "DGModule.hpp"
#include	"GSUtilsDefs.h"
#include <cmath>
#include <iostream>
#include <string>

static const GSResID AddOnInfoID			= ID_ADDON_INFO;
	static const Int32 AddOnNameID			= 1;
	static const Int32 AddOnDescriptionID	= 2;

static const short AddOnMenuID				= ID_ADDON_MENU;
	static const Int32 AddOnCommandID		= 1;

class ExampleDialog :	public DG::ModalDialog,
						public DG::PanelObserver,
						public DG::ButtonItemObserver,
						public DG::CompoundItemObserver
{
public:
	enum DialogResourceIds
	{
		ExampleDialogResourceId = ID_ADDON_DLG,
		OKButtonId = 1,
		CancelButtonId = 2,
		SeparatorId = 3
	};

	ExampleDialog () :
		DG::ModalDialog (ACAPI_GetOwnResModule (), ExampleDialogResourceId, ACAPI_GetOwnResModule ()),
		okButton (GetReference (), OKButtonId),
		cancelButton (GetReference (), CancelButtonId),
		separator (GetReference (), SeparatorId)
	{
		AttachToAllItems (*this);
		Attach (*this);
	}

	~ExampleDialog ()
	{
		Detach (*this);
		DetachFromAllItems (*this);
	}

private:
	virtual void PanelResized (const DG::PanelResizeEvent& ev) override
	{
		BeginMoveResizeItems ();
		okButton.Move (ev.GetHorizontalChange (), ev.GetVerticalChange ());
		cancelButton.Move (ev.GetHorizontalChange (), ev.GetVerticalChange ());
		separator.MoveAndResize (0, ev.GetVerticalChange (), ev.GetHorizontalChange (), 0);
		EndMoveResizeItems ();
	}

	virtual void ButtonClicked (const DG::ButtonClickEvent& ev) override
	{
		if (ev.GetSource () == &okButton) {
			PostCloseRequest (DG::ModalDialog::Accept);
		} else if (ev.GetSource () == &cancelButton) {
			PostCloseRequest (DG::ModalDialog::Cancel);
		}
	}

	DG::Button		okButton;
	DG::Button		cancelButton;
	DG::Separator	separator;
};

static API_Tranmat CreateTransformation(const API_Coord3D& origin, const double rotAngle)
{
	API_Tranmat transformation = {};

	const double	co = cos(rotAngle);
	const double	si = sin(rotAngle);
	// we need an orthogonal transformation
	// origin
	transformation.tmx[3] = origin.x;
	transformation.tmx[7] = origin.y;
	transformation.tmx[11] = origin.z;
	// X axis
	transformation.tmx[0] = co;
	transformation.tmx[4] = si;
	transformation.tmx[8] = 0.0;
	// Y axis
	transformation.tmx[1] = -si;
	transformation.tmx[5] = co;
	transformation.tmx[9] = 0.0;
	// we need a valid Z axis as well
	transformation.tmx[2] = 0.0;
	transformation.tmx[6] = 0.0;
	transformation.tmx[10] = 1.0;

	return transformation;
}

// -----------------------------------------------------------------------------
//  List all the attributes
// -----------------------------------------------------------------------------

static void		DisposeAttribute(API_Attribute* attrib)
{
	switch (attrib->header.typeID) {
	case API_MaterialID:			delete attrib->material.texture.fileLoc;											break;
	default:						/* nothing has to be disposed */													break;
	}
}

const char* AttrID_To_Name(API_AttrTypeID typeID)
{
	static char attrNames[][32] = {
		"Zombie",

		"Pen",
		"Layer",
		"Linetype",
		"Fill type",
		"Composite Wall",
		"Surface",
		"Layer Combination",
		"Zone Category",
		"Font",
		"Profile",
		"Pen table",
		"Dimension style",
		"Model View options",
		"MEP System",
		"Operation Profile",
		"Building Material",
	};

	static_assert (API_LastAttributeID == API_BuildingMaterialID, "Do not forget to update AttrID_To_Name function after new attribute type was introduced!");

	if (typeID < API_ZombieAttrID || typeID > API_LastAttributeID)
		return "Unknown attribute type";

	return attrNames[typeID];
}		// AttrID_To_Name

static void		Do_ListAttributes(void)
{
	API_AttrTypeID	typeID;
	GSErrCode		err = NoError;

	DBPrintf("# List attributes:");
	DBPrintf("#   - scan all attribute types and show the number of each type");
	DBPrintf("#   - deleted instances will be called \"DEL\"");

	for (typeID = API_FirstAttributeID; typeID <= API_LastAttributeID; ((Int32&)typeID)++) {
		GS::Array<API_Attribute> attributes;
		err = ACAPI_Attribute_GetAttributesByType(typeID, attributes);
		if (err != NoError) {
			//WriteReport_Err("Error in ACAPI_Attribute_GetAttributesByType", err);
			continue;
		}

		DBPrintf("%s: %d\n", AttrID_To_Name(typeID), attributes.GetSize());


		for (API_Attribute& attrib : attributes) {
			char guidStr[64];
			APIGuid2GSGuid(attrib.header.guid).ConvertToString(guidStr);
			DBPrintf("  [%3s]   {%s}  \"%s\"\n", attrib.header.index.ToUniString().ToCStr().Get(), guidStr, attrib.header.name);

			DisposeAttribute(&attrib);
		}
	}


	return;
}		// Do_ListAttributes

std::string CvtAnsi(const GS::UniString& src)
{
	return src.ToCStr(0, MaxUSize, CC_System).Get(); // Системная кодировка
}

//std::string UniStringToString(const GS::UniString& str) {
//	std::string newString(str.ToCStr());
//	return newString;
//}

//void PrintString(const char* format, const std::string& str) {
//	char	msgStr[256] = {};
//	sprintf(msgStr, format, str);
//	//DBPrintf(hotlinkNodeInfoStr.ToCStr().Get());
//}

static GS::UniString	GetLocationDetailsOfHotlinkNode(const API_HotlinkNode& hotlinkNode)
{
	switch (hotlinkNode.sourceType) {
	case APIHotlink_LocalFile:
		return GS::UniString::Printf("%T (Local File)",
			hotlinkNode.sourceLocation->ToDisplayText().ToPrintf());
	case APIHotlink_TWProject:
	case APIHotlink_TWFS: {
		GS::UniString serverUrl;
		GS::UniString projectName;
		GS::UniString userName;
		GSErrCode err = ACAPI_Teamwork_GetTeamworkProjectDetails(*hotlinkNode.serverSourceLocation,
						&serverUrl,
						&projectName,
						&userName);
		if (err == NoError) 
		{
			return GS::UniString::Printf("%T/%T (Teamwork Project, Server Name: %T, Project Name: %T, User Name: %T)",
				serverUrl.ToPrintf(),
				projectName.ToPrintf(),
				serverUrl.ToPrintf(),
				projectName.ToPrintf(),
				userName.ToPrintf());
		}
		else
		{
			return "Unknown";
		}
		
	}
	default:
		return "Unknown";
	}
}

static GS::Optional<API_Guid>	CreateHotlinkNode(const IO::Location& sourceFileLoc)
{
	API_HotlinkNode hotlinkNode = {};
	hotlinkNode.type = APIHotlink_Module;
	hotlinkNode.sourceLocation = new IO::Location(sourceFileLoc);

	GSErrCode err = ACAPI_Hotlink_CreateHotlinkNode(&hotlinkNode);

	if (err != NoError) {
		return GS::NoValue;
	}

	GS::UniString hotlinkName(hotlinkNode.name);
	DBPrintf("HotlinkNode Type: %d\n", hotlinkNode.type);
	DBPrintf("HotlinkNode Guid: %s\n", APIGuidToString(hotlinkNode.guid).ToCStr().Get());
	DBPrintf("HotlinkNode Name: %s\n", hotlinkName.ToCStr().Get());
	DBPrintf("HotlinkNode Source Type: %d\n", hotlinkNode.sourceType);
	DBPrintf("HotlinkNode Source Location: %s\n", GetLocationDetailsOfHotlinkNode(hotlinkNode).ToCStr().Get());
	return hotlinkNode.guid;
}


std::string GetHotlinkNodeSrcLocation(const API_Guid& hotlinkNodeGuid, bool debug) {
	API_HotlinkNode hotlinkNode = {};
	hotlinkNode.guid = hotlinkNodeGuid;
	bool enableUnplaced = true;
	GSErrCode error = ACAPI_Hotlink_GetHotlinkNode(&hotlinkNode, &enableUnplaced);
	GS::UniString	hotlinkSrcLoc;
	if (error == NoError) {
		hotlinkSrcLoc = GS::UniString::Printf("%T",hotlinkNode.sourceLocation->ToDisplayText().ToPrintf());
		if (debug) 
		{
			GS::UniString hotlinkName(hotlinkNode.name);
			DBPrintf("HotlinkNode Type: %d\n", hotlinkNode.type);
			DBPrintf("HotlinkNode Guid: %s\n", APIGuidToString(hotlinkNode.guid).ToCStr().Get());
			DBPrintf("HotlinkNode Name: %s\n", hotlinkName.ToCStr().Get());
			DBPrintf("HotlinkNode Source Type: %d\n", hotlinkNode.sourceType);
			DBPrintf("HotlinkNode Source Location: %s\n", hotlinkSrcLoc.ToCStr().Get());
		}
	}
	std::string hotlinkSrcLocStr(hotlinkSrcLoc.ToCStr());
	return hotlinkSrcLocStr;
}

void GetHotlinkNode(const API_Guid& hotlinkNodeGuid)
{
	API_HotlinkNode hotlinkNode = {};
	hotlinkNode.guid = hotlinkNodeGuid;
	bool enableUnplaced = true;
	GSErrCode error = ACAPI_Hotlink_GetHotlinkNode(&hotlinkNode, &enableUnplaced);
	if (error == NoError) {

		GS::UniString hotlinkNodeInfoStr = GS::UniString::Printf("{%T} - ", APIGuidToString(hotlinkNodeGuid).ToPrintf());
		GS::UniString hotlinkName(hotlinkNode.name);
		/*hotlinkNodeInfoStr += GS::UniString::Printf("Name: %T, Source Location: %T",
			hotlinkName, GetLocationDetailsOfHotlinkNode(hotlinkNode));*/
		hotlinkNodeInfoStr += GS::UniString::Printf("Type: %s, Source Location: %T",
			hotlinkNode.type == APIHotlink_Module ? "Hotlink Module" : "XRef",
			GetLocationDetailsOfHotlinkNode(hotlinkNode).ToPrintf());
		//GS::UniString hotlinkNodeInfoStr = GetLocationDetailsOfHotlinkNode(hotlinkNode);
		ACAPI_WriteReport(hotlinkName, true);
		std::string myStdStr(hotlinkNodeInfoStr.ToCStr());
		std::cout << myStdStr;
		//char	msgStr[] = {};
		//sprintf(msgStr, "%s", hotlinkNodeInfoStr.ToCStr().Get());
		DBPrintf(hotlinkNodeInfoStr.ToCStr().Get());
		DBPrintf("\n");

		//return hotlinkNode;

	}
	else {
		//return GS::NoValue;
	}
}
//static GS::UniString	GetStringDetailsOfHotlinkNode(const API_Guid& hotlinkNodeGuid)
//{
//	GS::UniString hotlinkNodeInfo = GS::UniString::Printf("{%T} - ", APIGuidToString(hotlinkNodeGuid).ToPrintf());
//
//	API_HotlinkNode hotlinkNode = {};
//	hotlinkNode.guid = hotlinkNodeGuid;
//	bool enableUnplaced = true;
//
//	GSErrCode err = ACAPI_Hotlink_GetHotlinkNode(&hotlinkNode, &enableUnplaced);
//	if (err == NoError) {
//		const GS::UniString updateTime = TIGetTimeString(hotlinkNode.updateTime, TI_LONG_DATE_FORMAT | TI_SHORT_TIME_FORMAT);
//
//		hotlinkNodeInfo += GS::UniString::Printf("Type: %s, Source Location: %T, Update Time: %T",
//			hotlinkNode.type == APIHotlink_Module ? "Hotlink Module" : "XRef",
//			GetLocationDetailsOfHotlinkNode(hotlinkNode).ToPrintf(),
//			updateTime.ToPrintf());
//	}
//	else {
//		hotlinkNodeInfo += "Failed to gather details";
//	}
//
//	return hotlinkNodeInfo;
//}

//static void DownloadBimCloudFiles(void) {
//	std::string_view bimCloudUrl{ "http://bimcloud.surbanajurong.com:21000" };
//	std
//}
static GSErrCode MenuCommandHandler (const API_MenuParams *menuParams)
{
	switch (menuParams->menuItemRef.menuResID) {
		case AddOnMenuID:
			switch (menuParams->menuItemRef.itemIndex) {
				case AddOnCommandID:
				{
					Do_ListAttributes();
					//DownloadBimCloudFiles();
					/*ExampleDialog dialog;
					dialog.Invoke ();*/

					//GSErrCode err = ACAPI_CallUndoableCommand("Create GDL Element", [&]() -> GSErrCode {
					//	API_LibPart		m_libpart = {};
					//	//BNZeroMemory(&m_libpart, sizeof(API_LibPart));
					//	m_libpart.typeID = APILib_ObjectID;
					//	const GS::UniString fileName{ "3R-1.gsm" };
					//	GS::ucscpy(m_libpart.file_UName, fileName.ToUStr());
					//	GSErrCode m_err = ACAPI_LibraryPart_Search(&m_libpart, false, true);
					//	Int32 m_ParamsCount = 0;
					//	double a, b;
					//	API_AddParType** params = NULL;
					//	m_err = ACAPI_LibraryPart_GetParams(m_libpart.index, &a, &b, &m_ParamsCount, &params);
					//	API_Element	m_element = {};
					//	//BNZeroMemory(&m_element, sizeof(API_Element));
					//	// GDL Object type
					//	m_element.header.type.typeID = API_ObjectID;
					//	m_element.header.floorInd = short(1);
					//	// Library index
					//	m_element.object.libInd = GS::Int32(334);
					//	m_element.object.head.floorInd = short(1);
					//	m_element.object.pos.x = double(0);
					//	m_element.object.pos.y = double(0);
					//	API_ElementMemo m_memo = {};
					//	//BNZeroMemory(&m_memo, sizeof(API_ElementMemo));
					//	m_memo.params = params;
					//	return ACAPI_Element_Create(&m_element, &m_memo);
					//});
					//if (err != NoError) {
					//	return err;
					//}

					// Get story info in the current environment
					API_StoryInfo storyInfo;
					BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
					GSErrCode err = ACAPI_ProjectSetting_GetStorySettings(&storyInfo, NULL);
					if (err != NoError) {
						return err;
					}

					//// Get Project preference working unit
					//API_WorkingUnitPrefs	unitPrefs;
					//err = ACAPI_ProjectSetting_GetPreferences(&unitPrefs, APIPrefs_WorkingUnitsID);
					//if (err != NoError)
					//	return err;

					//API_Coord coorOffsetPrefs;
					//err = ACAPI_ProjectSetting_GetOffset(&coorOffsetPrefs);
					//if (err != NoError)
					//	return err;

					// Get all object elements
					GS::Array<API_Guid> elemGuidList;
					err = ACAPI_Element_GetElemList(API_ObjectID, &elemGuidList);
					DBPrintf("Number of found element type: %ld\n", elemGuidList.GetSize());
					if (err != NoError)
						return err;


					//// Check each object element if it was created using a library to identify if it is a GDL object
					API_Element  element;
					API_ElementMemo  elementMemo;
					GSErrCode elemErr;
					//GS::Array <API_Element> elemList;
					std::vector<API_Element> elemList;
					std::vector<API_ElementMemo> elemMemoList;
					std::set<Int32> libIdList;
					API_Coord3D sampleGDLCoord = {};
					for (GS::Array<API_Guid>::ConstIterator it = elemGuidList.Enumerate(); it != nullptr; ++it) 
					{
						BNZeroMemory(&element, sizeof(API_Element));
						element.header.guid = *it;
						elemErr = ACAPI_Element_Get(&element);
						if (elemErr == NoError) 
						{
							elemList.push_back(element);
							// List all unique library index
							libIdList.insert(element.object.libInd);
							DBPrintf("libIndex: %ld\n", element.object.libInd);
						}

						elementMemo = {};
						elemErr = ACAPI_Element_GetMemo(element.header.guid, &elementMemo);
						if (elemErr == NoError)
						{
							elemMemoList.push_back(elementMemo);
							// print gdl object parameters
							//if (element.object.libInd >= 333) {
							//	Int32 memoParamCtr = BMGetHandleSize((GSHandle)elementMemo.params) / sizeof(API_AddParType);
							//	for (Int32 i = 0; i < memoParamCtr; ++i) {
							//		API_AddParType tmpParam = (*elementMemo.params)[i];
							//		if ((*elementMemo.params)[i].typeMod == API_ParArray)
							//			continue;


							//		std::string name((*elementMemo.params)[i].name);
							//		double length = 0;
							//		if ((*elementMemo.params)[i].typeID == APIParT_Length) {
							//			length = (*elementMemo.params)[i].value.real * pow(10, (int)unitPrefs.lengthUnit);
							//		}
							//	}
							//}

						}

						//if (element.object.libInd == 333) {
						//	sampleGDLCoord.x = element.hotspot.pos.x;
						//	sampleGDLCoord.y = element.hotspot.pos.y;
						//	sampleGDLCoord.z = element.hotspot.height;
						//}

					}
					//DBPrintf("pause\n");

					// 
					std::set<Int32>::iterator setItr;
					API_LibPart		m_libpart;
					std::vector<API_LibPart> libPartList;
					for (setItr = libIdList.begin(); setItr != libIdList.end(); ++setItr)
					{
						BNZeroMemory(&m_libpart, sizeof(API_LibPart));
						// Library part index seems to have some offset when used in the element object libInd
						// Library part index (Zero Based) while Object LibInd (One Based)
						m_libpart.index = (*setItr) - 1;
						GSErrCode libErr = ACAPI_LibraryPart_Search(&m_libpart, false, true);
						if (libErr == NoError) {
							libPartList.push_back(m_libpart);
						}
					}
					DBPrintf("pause\n");

					// Try searching each gdl library part in the sourcefile directory
					//IO::Location parentLocation("C:\\FREELANCE\\Archicad\\SJT-GDL\\Resources\\");
					std::map<Int32, API_Guid> gdlLibTo2DHLNodeGuidMap;
					std::map<Int32, API_Guid> gdlLibTo3DHLNodeGuidMap;
					for (unsigned int i = 0; i < libPartList.size(); i++)
					{

						std::string libPartFileName{ ((GS::UniString)libPartList[i].file_UName).ToCStr() };
						size_t strInd = libPartFileName.find(".gsm");

						// skip non-gsm file
						if (strInd == std::string::npos) continue;

						// Create 2D hotlink
						std::string hotlinkFile = "C:\\FREELANCE\\Archicad\\SJT-GDL\\Resources\\2D MOD\\" + libPartFileName.substr(0, strInd) + ".mod";
						//std::string hotlinkTestFile = "C:\\FREELANCE\\Archicad\\SJT-GDL\\Resources\\3R-1.mod";
						//GS::UniString hlUstringFile(hotlinkFile.c_str());
						IO::Location sourceFileLoc(GS::UniString(hotlinkFile.c_str()));
						//sourceFileLoc.AppendToLocal(IO::Name(GS::UniString(hotlinkFile.c_str())));
						//WriteReport("Source File: %s \n", sourceFileLoc.ToDisplayText().ToPrintf());

						// Try creating a hotlink node
						GS::Optional<API_Guid> hotlinkNodeGuid = CreateHotlinkNode(sourceFileLoc);
						if (!hotlinkNodeGuid.IsEmpty())
						{
							// add to map
							gdlLibTo2DHLNodeGuidMap.insert(std::make_pair(libPartList[i].index, hotlinkNodeGuid.Get()));
						}

						// Create 3D hotlink
						hotlinkFile = "C:\\FREELANCE\\Archicad\\SJT-GDL\\Resources\\3D MOD\\" + libPartFileName.substr(0, strInd) + ".mod";
						IO::Location sourceFileLoc3D(GS::UniString(hotlinkFile.c_str()));

						// Try creating a hotlink node
						hotlinkNodeGuid = CreateHotlinkNode(sourceFileLoc3D);
						if (!hotlinkNodeGuid.IsEmpty())
						{
							// add to map
							gdlLibTo3DHLNodeGuidMap.insert(std::make_pair(libPartList[i].index, hotlinkNodeGuid.Get()));
						}

					}
					DBPrintf("pause\n");


					// Create hotlink in the same story/floor with the gdl objects
					if (false)
					{
						for (unsigned int i = 0; i < elemList.size(); i++)
						{
							// Check if object lib index is in the hotlink node map
							Int32 key = elemList[i].object.libInd;
							if (gdlLibTo2DHLNodeGuidMap.find(key) == gdlLibTo2DHLNodeGuidMap.end()) continue;

							API_Element hotlinkElement = {};
							hotlinkElement.header.type = API_HotlinkID;
							hotlinkElement.header.layer = ACAPI_CreateAttributeIndex(46); // Master Layer -> [46] Model Unit - Module
							hotlinkElement.hotlink.type = APIHotlink_Module;
							hotlinkElement.header.floorInd = elemList[i].header.floorInd; // put to story/floor 

							hotlinkElement.hotlink.transformation = CreateTransformation({ elemList[i].object.pos.x, elemList[i].object.pos.y, 0.0 }, 0);
							API_Tranmat transformation = {};
							hotlinkElement.hotlink.floorDifference = elemList[i].header.floorInd - storyInfo.lastStory; // floor ind - total floor (3 - 4)

							hotlinkElement.hotlink.ignoreTopFloorLinks = true;
							hotlinkElement.hotlink.relinkWallOpenings = true;
							hotlinkElement.hotlink.adjustLevelDiffs = true;


							hotlinkElement.hotlink.hotlinkNodeGuid = gdlLibTo2DHLNodeGuidMap[key];

							err = ACAPI_CallUndoableCommand("Element Test - Create Hotlink Node and Place an Instance", [&]() -> GSErrCode {
								return ACAPI_Element_Create(&hotlinkElement, nullptr);
								});
							//err = ACAPI_Element_Create(&hotlinkElement, nullptr);
							if (err != NoError) return err;
						}
					}

					//Do_ListAttributes();
					//// Get all hotlink elements
					//GS::Array<API_Guid> elemHLGuidList;
					//err = ACAPI_Element_GetElemList(API_HotlinkID, &elemHLGuidList);
					//DBPrintf("Number of found element type: %ld", elemHLGuidList.GetSize());
					//if (err != NoError)
					//	return err;



					///**** EXAMPLE IN API 27 to create hotlink element (Working required transformation and layer) *****/
					//API_HotlinkNode hotlinkNode{};
					////IO::Location srcLocation (GS::UniString(hotlinkSrcLocation.c_str()));
					//IO::Location srcLocation("C:\\FREELANCE\\Archicad\\SJT-GDL\\Resources\\3R-1.mod");
					//hotlinkNode.type = APIHotlink_Module;
					//hotlinkNode.sourceLocation = new IO::Location("C:\\FREELANCE\\Archicad\\SJT-GDL\\Resources\\3R-1-upward-projOrigin.mod");;

					//// Try creating a hotlink node
					//err = ACAPI_Hotlink_CreateHotlinkNode(&hotlinkNode);
					//if (err != NoError) return err;

					//API_Element hotlinkElement = {};
					//hotlinkElement.header.type = API_HotlinkID;
					//hotlinkElement.header.layer = ACAPI_CreateAttributeIndex(46); // Master Layer -> [46] Model Unit - Module
					//hotlinkElement.hotlink.type = APIHotlink_Module;
					//hotlinkElement.header.floorInd = 3; // put to story/floor 3

					//hotlinkElement.hotlink.transformation = CreateTransformation({ 10.0, 0.0, 0.0 }, 0); 
					////hotlinkElement.hotlink.transformation = CreateTransformation(sampleGDLCoord, 0);
					//API_Tranmat transformation = {};
					//hotlinkElement.hotlink.floorDifference = -1; // floor ind - total floor (3 - 4)

					//////API_Tranmat tmpTransformation = CreateTransformation({ 10.0, 4.0, 0.0 }, 3.141592653589793 / 6.0);

					//hotlinkElement.hotlink.ignoreTopFloorLinks = true;
					//hotlinkElement.hotlink.relinkWallOpenings = true;
					//hotlinkElement.hotlink.adjustLevelDiffs = true;
					////element.hotlink.floorDifference = 0;

					//hotlinkElement.hotlink.hotlinkNodeGuid = hotlinkNode.guid;

					//err = ACAPI_CallUndoableCommand("Element Test - Create Hotlink Node and Place an Instance", [&]() -> GSErrCode {
					//	return ACAPI_Element_Create(&hotlinkElement, nullptr);
					//});
					//if (err != NoError) return err;
					////DBPrintf("pause");
					///**** ENDEXAMPLE IN API 27 to create hotlink element *****/

					IO::Location appFolderLocation;
					API_SpecFolderID specID = API_EmbeddedProjectLibraryHotlinkFolderID;
					err = ACAPI_ProjectSettings_GetSpecFolder(&specID, &appFolderLocation);
					if (err == NoError) {
						IO::Path path;
						appFolderLocation.ToPath(&path);
						const char* pCharPath = path.operator const char* ();
						ACAPI_WriteReport(pCharPath, true);
					}

					if (true) 
					{
						//// Get all hotlink elements
						GS::Array<API_Guid> elemHLGuidList;
						err = ACAPI_Element_GetElemList(API_HotlinkID, &elemHLGuidList);
						DBPrintf("Number of found element type: %ld", elemHLGuidList.GetSize());
						if (err != NoError)
							return err;

						//// Check each object element if it was created using a library to identify if it is a GDL object
						API_Element  elementHL;
						GSErrCode elemHLErr;
						std::vector<API_Element> elemHLList;
						std::vector<GS::UniString> elemHLGuidStrList;
						std::vector<API_ElementMemo> elemHLMemoList;
						std::vector<GS::Optional<API_HotlinkNode>> hotlinkNodeList;
						std::vector<GS::UniString> hotlinkNodeInfoStrList;
						for (GS::Array<API_Guid>::ConstIterator it = elemHLGuidList.Enumerate(); it != nullptr; ++it) {
							BNZeroMemory(&elementHL, sizeof(API_Element));
							elementHL.header.guid = *it;
							elemHLErr = ACAPI_Element_Get(&elementHL);
							if (elemHLErr == NoError) {
								elemHLList.push_back(elementHL);

								GS::UniString guidStr = APIGuidToString(elementHL.header.guid);
								elemHLGuidStrList.push_back(guidStr);
							}

							//API_HotlinkNode hotlinkNode = {};
							//hotlinkNode.guid = elementHL.hotlink.hotlinkNodeGuid;
							//bool enableUnplaced = true;
							//elemHLErr = ACAPI_Hotlink_GetHotlinkNode(&hotlinkNode, &enableUnplaced);
							//if (elemHLErr == NoError) {
							//	
							//	hotlinkNodeList.push_back(hotlinkNode);
							//	GS::UniString hotlinkNodeInfoStr = GetLocationDetailsOfHotlinkNode(hotlinkNode);
							//	//ACAPI_WriteReport(hotlinkNodeInfoStr, true);
							//	std::string myStdStr(hotlinkNodeInfoStr.ToCStr());
							//	std::cout << myStdStr;
							//	//char	msgStr[] = {};
							//	//sprintf(msgStr, "%s", hotlinkNodeInfoStr.ToCStr().Get());
							//	DBPrintf(hotlinkNodeInfoStr.ToCStr().Get());
							//	hotlinkNodeInfoStrList.push_back(hotlinkNodeInfoStr);
							//	
							//}
							GetHotlinkNode(elementHL.hotlink.hotlinkNodeGuid);
							//GS::Optional<API_HotlinkNode> currHotlinkNode = GetHotlinkNode(elementHL.hotlink.hotlinkNodeGuid);
							//if (!currHotlinkNode.IsEmpty())
							//{
							//	// add to map
							//	hotlinkNodeList.push_back(currHotlinkNode);
							//}
							/*API_ElementMemo elementHLMemo = {};
							elemHLErr = ACAPI_Element_GetDefaults(&elementHL, &elementHLMemo);
							if (elemHLErr == NoError)
							{
								elemHLMemoList.push_back(elementHLMemo);
							}*/
						}
						DBPrintf("pause");
					}

					
					

					


					// Get selected gdl object element's GUID
					//API_SelectionInfo    selectionInfo;
					//GS::Array<API_Neig>  selNeigs;
					//err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, true);
					//for (Int32 i = 0; i < selectionInfo.sel_nElem; i++) {
					//	API_Element  element;
					//	BNZeroMemory(&element, sizeof(API_Element));
					//	element.header.guid = selNeigs[i].guid;
					//	err = ACAPI_Element_Get(&element);
					//	if (err != NoError)
					//		break;
					//}

				}
				break;
				case 2:
				{
					//// Get Project preference working unit
					API_WorkingUnitPrefs	unitPrefs;
					GSErrCode err = ACAPI_ProjectSetting_GetPreferences(&unitPrefs, APIPrefs_WorkingUnitsID);
					if (err != NoError)
						return err;

					// Get story info in the current environment
					API_StoryInfo storyInfo;
					BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
					err = ACAPI_ProjectSetting_GetStorySettings(&storyInfo, NULL);
					if (err != NoError) {
						return err;
					}

					// Get all object elements
					GS::Array<API_Guid> elemGuidList;
					err = ACAPI_Element_GetElemList(API_ObjectID, &elemGuidList);
					DBPrintf("Number of found element type: %ld\n", elemGuidList.GetSize());
					if (err != NoError)
						return err;

					//// Check each object element if it was created using a library to identify if it is a GDL object
					API_Element  element;
					GSErrCode elemErr;
					API_LibPart	m_libpart;
					std::vector<API_Element> elemList;
					std::set<Int32> libIdList;
					std::map<Int32, API_LibPart> libraryPartList; // (id - library part)
					DBPrintf("Find Element and get library part\n");
					for (GS::Array<API_Guid>::ConstIterator it = elemGuidList.Enumerate(); it != nullptr; ++it)
					{
						BNZeroMemory(&element, sizeof(API_Element));
						element.header.guid = *it;
						elemErr = ACAPI_Element_Get(&element);
						if (elemErr == NoError)
						{
							elemList.push_back(element);

							// Check and get library part base on index
							if (libraryPartList.find(element.object.libInd) == libraryPartList.end()) {
								BNZeroMemory(&m_libpart, sizeof(API_LibPart));
								// Library part index seems to have some offset when used in the element object libInd
								// Library part index (Zero Based) while Object LibInd (One Based)
								m_libpart.index = element.object.libInd - 1;
								GSErrCode libErr = ACAPI_LibraryPart_Search(&m_libpart, false, true);
								if (libErr == NoError) {
									// add to map
									libraryPartList.insert(std::make_pair(element.object.libInd, m_libpart));
									GS::UniString libFileName(m_libpart.file_UName);
									/*WriteReport("Element Library Index: %d", element.object.libInd);
									WriteReport("--------------------");
									WriteReport("FileName: %s", libFileName.ToCStr().Get());
									WriteReport("Location: %s", m_libpart.location->ToDisplayText().ToCStr().Get());*/
									DBPrintf("--------------------\n");
									DBPrintf("Element Library Index: %d\n", element.object.libInd);
									DBPrintf("FileName: %s\n", libFileName.ToCStr().Get());
									DBPrintf("Location: %s\n", m_libpart.location->ToDisplayText().ToCStr().Get());
									DBPrintf("--------------------\n");

								}
							}
						}
					}
					DBPrintf("pause\n\n");



					// Try searching each gdl library part in the sourcefile directory
					DBPrintf("\n\nCreate Hotlink Node\n");
					IO::Location parentLocation("C:\\FREELANCE\\Archicad\\SJT-GDL\\Resources\\");
					std::map<Int32, API_Guid> libIndTo2DHotlinknode;
					for (auto libPart : libraryPartList) {
						std::string libPartFileName{ ((GS::UniString)libPart.second.file_UName).ToCStr() };
						size_t strInd = libPartFileName.find(".gsm");

						// skip non-gsm file
						if (strInd == std::string::npos) continue;

						// Create 2D hotlink
						std::string hotlinkFile = "C:\\FREELANCE\\Archicad\\SJT-GDL\\Resources\\2D MOD\\" + libPartFileName.substr(0, strInd) + ".mod";
						IO::Location sourceFileLoc(GS::UniString(hotlinkFile.c_str()));

						// Try creating a hotlink node
						DBPrintf("--------------------\n");
						DBPrintf("Element Library Index: %d\n", libPart.first);
						GS::Optional<API_Guid> hotlinkNodeGuid = CreateHotlinkNode(sourceFileLoc);
						if (!hotlinkNodeGuid.IsEmpty())
						{
							// add to map
							libIndTo2DHotlinknode.insert(std::make_pair(libPart.first, hotlinkNodeGuid.Get()));
						}
						DBPrintf("--------------------\n");

					}
					DBPrintf("pause\n\n");


					// Create hotlink in the same story/floor with the gdl objects
					DBPrintf("\n\nDelete GDL and Create Hotlink Module\n");
					if (true)
					{
						for (unsigned int i = 0; i < elemList.size(); i++)
						{
							// Check if object lib index is in the hotlink node map
							Int32 key = elemList[i].object.libInd;
							if (libIndTo2DHotlinknode.find(key) == libIndTo2DHotlinknode.end()) continue;
							DBPrintf("--------------------\n");
							DBPrintf("GDL Guid: %d\n", APIGuidToString(elemList[i].header.guid).ToCStr().Get());
							DBPrintf("GDL Floor Index: %d\n", elemList[i].object.head.floorInd);
							DBPrintf("GDL Library Index: %d\n", elemList[i].object.libInd);
							DBPrintf("GDL Has Memo: %d\n", elemList[i].object.head.hasMemo);

							// Get GDL element memo parameters
							if (elemList[i].object.head.hasMemo) {
								API_ElementMemo  elementMemo = {};
								err = ACAPI_Element_GetMemo(elemList[i].header.guid, &elementMemo);
								if (err == NoError)
								{
									DBPrintf("    --------- MEMO PARAMS START ---------\n");
									Int32 memoParamCtr = BMGetHandleSize((GSHandle)elementMemo.params) / sizeof(API_AddParType);
									for (Int32 i = 0; i < memoParamCtr; ++i) {
										API_AddParType tmpParam = (*elementMemo.params)[i];
										if ((*elementMemo.params)[i].typeMod == API_ParArray)
											continue;

										std::string paramName((*elementMemo.params)[i].name);
										if ((*elementMemo.params)[i].typeID == APIParT_Length) {
											double length = 0;
											length = (*elementMemo.params)[i].value.real * pow(10, (int)unitPrefs.lengthUnit);
											//DBPrintf("Memo Param Value: %d\n", length);
										}
										else if ((*elementMemo.params)[i].typeID == APIParT_CString) {
											GS::UniString valueStr((*elementMemo.params)[i].value.uStr);
											DBPrintf("    Memo Param Name: %s\n", GS::UniString(paramName.c_str()).ToCStr().Get());
											DBPrintf("    Memo Param Type: %d\n", (*elementMemo.params)[i].typeID);
											DBPrintf("    Memo Param Value: %s\n", valueStr.ToCStr().Get());
											DBPrintf("\n");
										}
									}
									DBPrintf("    --------- MEMO PARAMS END ---------\n");
								}
							}


							API_Element hotlinkElement = {};
							hotlinkElement.header.type = API_HotlinkID;
							hotlinkElement.header.layer = ACAPI_CreateAttributeIndex(46); // Master Layer -> [46] Model Unit - Module
							hotlinkElement.hotlink.type = APIHotlink_Module;
							hotlinkElement.header.floorInd = elemList[i].header.floorInd; // put to story/floor 

							hotlinkElement.hotlink.transformation = CreateTransformation({ elemList[i].object.pos.x, elemList[i].object.pos.y, 0.0 }, 0);
							hotlinkElement.hotlink.floorDifference = elemList[i].header.floorInd - storyInfo.lastStory; // floor ind - total floor (3 - 4)

							// Top Linked elements - Keep height as in Story Structure of Hotlink Source
							hotlinkElement.hotlink.ignoreTopFloorLinks = true;
							// Additional Offset - Relink all Doors/Windows to Wall Base
							hotlinkElement.hotlink.relinkWallOpenings = true;
							// Elements Elevation - Adjust Elevation to Story Structure of Host Project
							hotlinkElement.hotlink.adjustLevelDiffs = false;

							hotlinkElement.hotlink.hotlinkNodeGuid = libIndTo2DHotlinknode[key];

							err = ACAPI_CallUndoableCommand("Element Test - Create Hotlink Node and Place an Instance", [&]() -> GSErrCode {
								return ACAPI_Element_Create(&hotlinkElement, nullptr);
								});
							if (err != NoError) return err;

							// Delete GDL Element
							//err = ACAPI_Element_Delete({elemList[i].header.guid});
							err = ACAPI_CallUndoableCommand("Element Test - Delete GDL Element", [&]() -> GSErrCode {
								return ACAPI_Element_Delete({ elemList[i].header.guid });
								});
							//if (err != NoError) return err;
							DBPrintf("--------------------\n");
						}

						
					}

				}
				break;
				case 3:
				{
					//// Get Project preference working unit
					API_WorkingUnitPrefs	unitPrefs;
					GSErrCode err = ACAPI_ProjectSetting_GetPreferences(&unitPrefs, APIPrefs_WorkingUnitsID);
					if (err != NoError)
						return err;

					// Get story info in the current environment
					API_StoryInfo storyInfo;
					BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
					err = ACAPI_ProjectSetting_GetStorySettings(&storyInfo, NULL);
					if (err != NoError) {
						return err;
					}

					// Get all object elements
					GS::Array<API_Guid> elemGuidList;
					err = ACAPI_Element_GetElemList(API_HotlinkID, &elemGuidList);
					DBPrintf("Number of found element type: %ld\n", elemGuidList.GetSize());
					if (err != NoError)
						return err;

					//// Check each hotlink element convert 2D to 3D hotlink
					API_Element  element;
					GSErrCode elemErr;
					std::vector<API_Element> elemList;
					std::map<Int32, API_LibPart> libraryPartList; // (id - library part)
					std::map<std::string, API_Guid> src2DLocTo3DHotlinkNode;
					
					DBPrintf("Find Element and get hotlink node\n");
					std::string parent2DFolderName{ "\\2D MOD\\" };
					for (GS::Array<API_Guid>::ConstIterator it = elemGuidList.Enumerate(); it != nullptr; ++it)
					{
						BNZeroMemory(&element, sizeof(API_Element));
						element.header.guid = *it;
						elemErr = ACAPI_Element_Get(&element);
						if (elemErr == NoError)
						{
							elemList.push_back(element);

							//if (element.header.floorInd > 0) continue; // Temp for testing

							std::string hotlinkNodeSrcLoc = GetHotlinkNodeSrcLocation(element.hotlink.hotlinkNodeGuid, false);
							size_t strInd = hotlinkNodeSrcLoc.find(parent2DFolderName);

							// skip if 2d parent folder not found
							if (strInd == std::string::npos) continue;

							// Check if 3d hotlink node already created based on 2d hotlink source loc
							API_Guid hotlinkNode3DGuid = {};
							if (src2DLocTo3DHotlinkNode.find(hotlinkNodeSrcLoc) == src2DLocTo3DHotlinkNode.end()) 
							{
								// Create 3D hotlink node
								std::string hotlinkNode3DSrcLoc{ hotlinkNodeSrcLoc };
								hotlinkNode3DSrcLoc.replace(strInd, parent2DFolderName.length(), "\\3D MOD\\");
								IO::Location sourceFileLoc(GS::UniString(hotlinkNode3DSrcLoc.c_str()));

								// Try creating a hotlink node
								GS::Optional<API_Guid> hotlinkNodeGuid = CreateHotlinkNode(sourceFileLoc);
								if (!hotlinkNodeGuid.IsEmpty())
								{
									// add to map
									src2DLocTo3DHotlinkNode.insert(std::make_pair(hotlinkNodeSrcLoc, hotlinkNodeGuid.Get()));
									hotlinkNode3DGuid = hotlinkNodeGuid.Get();
								}
							}
							else {
								hotlinkNode3DGuid = src2DLocTo3DHotlinkNode[hotlinkNodeSrcLoc];
							}

							// Create hotlink element (copy some important parameters from 2d HLM to 3D HLM
							API_Element hotlinkElement = {};
							hotlinkElement.header.type = element.header.type;
							hotlinkElement.header.layer = element.header.layer;
							hotlinkElement.hotlink.type = element.hotlink.type;
							hotlinkElement.header.floorInd = element.header.floorInd;

							hotlinkElement.hotlink.transformation = element.hotlink.transformation;
							// 3D floor diff has an additional offset of 1 upon comparing 2d and 3d hlm in the same floor
							hotlinkElement.hotlink.floorDifference = element.hotlink.floorDifference + 1;

							// Top Linked elements - Keep height as in Story Structure of Hotlink Source
							hotlinkElement.hotlink.ignoreTopFloorLinks = element.hotlink.ignoreTopFloorLinks;
							// Additional Offset - Relink all Doors/Windows to Wall Base
							hotlinkElement.hotlink.relinkWallOpenings = element.hotlink.relinkWallOpenings;
							// Elements Elevation - Adjust Elevation to Story Structure of Host Project
							hotlinkElement.hotlink.adjustLevelDiffs = element.hotlink.adjustLevelDiffs;

							hotlinkElement.hotlink.hotlinkNodeGuid = hotlinkNode3DGuid;

							//// Delete 2D HLM Element before creating 3D HLM element. If they overlap, the 2D HLM group returns null/empty list
							//DBPrintf("Delete 2D HLM\n");
							//GS::Array<API_Guid> deleteGuids = {};
							//DBPrintf("2D Hotlink Group Guid: %s\n", APIGuidToString(element.hotlink.hotlinkGroupGuid).ToCStr().Get());
							//err = ACAPI_Grouping_GetGroupedElems(element.hotlink.hotlinkGroupGuid, &deleteGuids);
							//if (err != NoError) return err;
							//err = ACAPI_CallUndoableCommand("Element Test - Delete 2D HLM Elements", [&]() -> GSErrCode {
							//	return ACAPI_Element_Delete(deleteGuids);
							//	});
							//if (err != NoError) return err;

							DBPrintf("Create 3D HLM\n");
							DBPrintf("HotlinkNode Guid: %s\n", APIGuidToString(hotlinkElement.hotlink.hotlinkNodeGuid).ToCStr().Get());
							DBPrintf("Floor Index:%d\n", hotlinkElement.header.floorInd);
							DBPrintf("Floor Diff:%d\n", hotlinkElement.hotlink.floorDifference);
							GSErrCode err = ACAPI_CallUndoableCommand("Element Test - Create Hotlink Node and Place an Instance", [&]() -> GSErrCode {
								return ACAPI_Element_Create(&hotlinkElement, nullptr);
							});
							if (err != NoError) return err;

							
					//		deleteGuids.Push(element.header.guid);
					//		
					//		//// Delete 2D HLM Element
					//		//err = ACAPI_CallUndoableCommand("Element Test - Delete 2D HLM Element", [&]() -> GSErrCode {
					//		//	return ACAPI_Hotlink_DeleteHotlinkNode(const_cast<API_Guid*> (&element.hotlink.hotlinkNodeGuid));
					//		//	});
					//		//if (err != NoError) return err;
					//		//DBPrintf("--------------------\n");

						}
					}
					////// Delete 2D HLM Element
					//err = ACAPI_CallUndoableCommand("Element Test - Delete 2D HLM Element", [&]() -> GSErrCode {
					//	return ACAPI_Element_Delete(elemGuidList);
					//});
				/*	if (err != NoError) return err;*/
					DBPrintf("pause\n\n");


				}
				break;
				case 4:
				{
					//// Get Project preference working unit
					API_WorkingUnitPrefs	unitPrefs;
					GSErrCode err = ACAPI_ProjectSetting_GetPreferences(&unitPrefs, APIPrefs_WorkingUnitsID);
					if (err != NoError)
						return err;

					// Get story info in the current environment
					API_StoryInfo storyInfo;
					BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
					err = ACAPI_ProjectSetting_GetStorySettings(&storyInfo, NULL);
					if (err != NoError) {
						return err;
					}

					// Get all object elements
					GS::Array<API_Guid> elemGuidList;
					err = ACAPI_Element_GetElemList(API_HotlinkID, &elemGuidList);
					DBPrintf("Number of found element type: %ld\n", elemGuidList.GetSize());
					if (err != NoError)
						return err;

					//// Check each hotlink element convert 2D to 3D hotlink
					API_Element  element;
					GSErrCode elemErr;
					std::vector<API_Element> elemList;
					std::map<Int32, API_LibPart> libraryPartList; // (id - library part)
					std::map<std::string, API_Guid> src2DLocTo3DHotlinkNode;
					GS::Array<API_Guid> deleteGuids = {};
					DBPrintf("Find Element and get hotlink node\n");
					std::string parent2DFolderName{ "\\2D MOD\\" };
					for (GS::Array<API_Guid>::ConstIterator it = elemGuidList.Enumerate(); it != nullptr; ++it)
					{
						BNZeroMemory(&element, sizeof(API_Element));
						element.header.guid = *it;
						elemErr = ACAPI_Element_Get(&element);
						if (elemErr == NoError)
						{
							elemList.push_back(element);
							DBPrintf("--------------------------\n");
							DBPrintf("Display Current HLM Info\n");
							DBPrintf("HotlinkNode Guid: %s\n", APIGuidToString(element.hotlink.hotlinkNodeGuid).ToCStr().Get());
							DBPrintf("Floor Index:%d\n", element.header.floorInd);
							DBPrintf("Floor Diff:%d\n", element.hotlink.floorDifference);
							std::string hotlinkNodeSrcLoc = GetHotlinkNodeSrcLocation(element.hotlink.hotlinkNodeGuid, true);
							DBPrintf("--------------------------\n");
						}
					}
					DBPrintf("pause\n\n");

				}
				break;
				case 5:
				{
					//// Get Project preference working unit
					API_WorkingUnitPrefs	unitPrefs;
					GSErrCode err = ACAPI_ProjectSetting_GetPreferences(&unitPrefs, APIPrefs_WorkingUnitsID);
					if (err != NoError)
						return err;

					// Get story info in the current environment
					API_StoryInfo storyInfo;
					BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
					err = ACAPI_ProjectSetting_GetStorySettings(&storyInfo, NULL);
					if (err != NoError) {
						return err;
					}

					// Get all object elements
					GS::Array<API_Guid> elemGuidList;
					err = ACAPI_Element_GetElemList(API_HotlinkID, &elemGuidList);
					DBPrintf("Number of found element type: %ld\n", elemGuidList.GetSize());
					if (err != NoError)
						return err;

					//// Check each hotlink element convert 2D to 3D hotlink
					API_Element  element;
					GSErrCode elemErr;
					std::vector<API_Element> elemList;
					std::map<Int32, API_LibPart> libraryPartList; // (id - library part)
					std::map<std::string, API_Guid> src2DLocTo3DHotlinkNode;

					DBPrintf("Find Element and get hotlink node\n");
					std::string parent2DFolderName{ "\\2D MOD\\" };
					for (GS::Array<API_Guid>::ConstIterator it = elemGuidList.Enumerate(); it != nullptr; ++it)
					{
						BNZeroMemory(&element, sizeof(API_Element));
						element.header.guid = *it;
						elemErr = ACAPI_Element_Get(&element);
						if (elemErr == NoError)
						{
							elemList.push_back(element);

							DBPrintf("2D Hotlink Group Guid: %s\n", APIGuidToString(element.hotlink.hotlinkGroupGuid).ToCStr().Get());
							GS::Array<API_Guid> deleteGuids = {};
							err = ACAPI_Grouping_GetGroupedElems(element.hotlink.hotlinkGroupGuid, &deleteGuids);
							if (err != NoError) return err;
							err = ACAPI_CallUndoableCommand("Element Test - Delete 2D HLM Elements", [&]() -> GSErrCode {
								return ACAPI_Element_Delete(deleteGuids);
								});
							if (err != NoError) return err;

						}
					}

					DBPrintf("pause\n\n");
				}
			}
			break;
	}
	return NoError;
}


API_AddonType CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (&envir->addOnInfo.name, AddOnInfoID, AddOnNameID, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, AddOnInfoID, AddOnDescriptionID, ACAPI_GetOwnResModule ());

	return APIAddon_Normal;
}

GSErrCode RegisterInterface (void)
{
#ifdef ServerMainVers_2700
	//return ACAPI_MenuItem_RegisterMenu (AddOnMenuID, 0, MenuCode_Tools, MenuFlag_Default);
	GSErrCode err = NoError;
	err = ACAPI_MenuItem_RegisterMenu(32500, 0, MenuCode_Tools, MenuFlag_SeparatorBefore);
	if (err == NoError)
		err = ACAPI_MenuItem_RegisterMenu(32501, 0, MenuCode_Tools, MenuFlag_Default);
	if (err == NoError)
		err = ACAPI_MenuItem_RegisterMenu(32502, 0, MenuCode_Tools, MenuFlag_Default);
	if (err == NoError)
		err = ACAPI_MenuItem_RegisterMenu(32503, 0, MenuCode_Tools, MenuFlag_Default);
	if (err == NoError)
		err = ACAPI_MenuItem_RegisterMenu(32504, 0, MenuCode_Tools, MenuFlag_Default);
	if (err == NoError)
		err = ACAPI_MenuItem_RegisterMenu(32505, 0, MenuCode_Tools, MenuFlag_SeparatorAfter);

	return err;
#else
	return ACAPI_Register_Menu (AddOnMenuID, 0, MenuCode_Tools, MenuFlag_Default);
#endif
}

GSErrCode Initialize (void)
{
#ifdef ServerMainVers_2700
	//return ACAPI_MenuItem_InstallMenuHandler (AddOnMenuID, MenuCommandHandler);
	GSErrCode err = NoError;
	err = ACAPI_MenuItem_InstallMenuHandler(32500, MenuCommandHandler);
	if (err == NoError)
		err = ACAPI_MenuItem_InstallMenuHandler(32501, MenuCommandHandler);
	if (err == NoError)
		err = ACAPI_MenuItem_InstallMenuHandler(32502, MenuCommandHandler);
	if (err == NoError)
		err = ACAPI_MenuItem_InstallMenuHandler(32503, MenuCommandHandler);
	if (err == NoError)
		err = ACAPI_MenuItem_InstallMenuHandler(32504, MenuCommandHandler);
	if (err == NoError)
		err = ACAPI_MenuItem_InstallMenuHandler(32505, MenuCommandHandler);
	
	return err;
#else
	return ACAPI_Install_MenuHandler (AddOnMenuID, MenuCommandHandler);
#endif
}

GSErrCode FreeData (void)
{
	return NoError;
}
