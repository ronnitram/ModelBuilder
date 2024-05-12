// *****************************************************************************
// Description:		Source code for the Automate Functions Add-On
// *****************************************************************************

#define	_AUTOMATE_FUNCTIONS_TRANSL_

// --- Includes ----------------------------------------------------------------

#include	"APIEnvir.h"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"

#include	"DGModule.hpp"

// =============================================================================
//
// Main functions
//
// =============================================================================

// -----------------------------------------------------------------------------
// Execute a New & Reset command
// -----------------------------------------------------------------------------

static void		Do_New (void)
{
	API_NewProjectPars	npp = {};
	npp.newAndReset = true;
	npp.enableSaveAlert = false;

	const GSErrCode err = ACAPI_Automate (APIDo_NewProjectID, &npp);
	if (err != NoError) {
		ACAPI_WriteReport ("Error in APIDo_NewPlanID: %s", true, ErrID_To_Name (err));
	}
}		// Do_New

static IO::Location GetSpecialFolderLocation (API_SpecFolderID id)
{
	IO::Location folder;
	ACAPI_Environment (APIEnv_GetSpecFolderID, &id, &folder);
	return folder;
}


// -----------------------------------------------------------------------------
// Open a project file
//	 - the file is hardcoded
// -----------------------------------------------------------------------------

static void		Do_Open (void)
{
	API_FileOpenPars	fop = {};
	fop.fileTypeID = APIFType_PlanFile;
	fop.useStoredLib = true;

	IO::Location plaFile (GetSpecialFolderLocation (API_ApplicationFolderID), IO::Name ("QuickTest.pla"));
	fop.file = &plaFile;

	const GSErrCode err = ACAPI_Automate (APIDo_OpenID, &fop);
	if (err != NoError) {
		ACAPI_WriteReport ("Error in APIDo_OpenID: %s", true, ErrID_To_Name (err));
	}
}		// Do_Open


// -----------------------------------------------------------------------------
// Close the current project
// -----------------------------------------------------------------------------

static void		Do_Close (void)
{
	const GSErrCode err = ACAPI_Automate (APIDo_CloseID, (void *) (Int32) 1234);
	if (err != NoError) {
		ACAPI_WriteReport ("Error in APIDo_CloseID: %s", true, ErrID_To_Name (err));
	}
}		// Do_Close


// -----------------------------------------------------------------------------
// Save the current project
// -----------------------------------------------------------------------------

static void		Do_Save_Plan (void)
{
	const GSErrCode err = ACAPI_Automate (APIDo_SaveID);
	if (err != NoError) {
		ACAPI_WriteReport ("Error in APIDo_SaveID: %s", true, ErrID_To_Name (err));
	}
}		// Do_Save_Plan


// -----------------------------------------------------------------------------
// Save the current plan into a picture
// -----------------------------------------------------------------------------

static void		Do_Save_TiffFile (void)
{
	API_FileSavePars		fsp = {};
	fsp.fileTypeID = APIFType_TIFFFile;

	IO::Location outputTIFFile (GetSpecialFolderLocation (API_UserDocumentsFolderID), IO::Name ("PictTest.tif"));
	fsp.file = &outputTIFFile;

	API_SavePars_Picture pars_pict	 = {};
	pars_pict.colorDepth			 = APIColorDepth_256C;
	pars_pict.dithered				 = false;
	pars_pict.view2D				 = true;
	pars_pict.crop					 = true;
	pars_pict.keepSelectionHighlight = true;

	const GSErrCode err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);
	if (err != NoError) {
		ACAPI_WriteReport ("Error in APIDo_SaveID (TIFF): %s", true, ErrID_To_Name (err));
	}
}		// Do_Save_TiffFile


// -----------------------------------------------------------------------------
// Save the current plan to an IFC file
// -----------------------------------------------------------------------------

static void		Do_Save_IfcFile (void)
{
	API_IFCTranslatorIdentifier firstTranslator;
	GS::Array<API_IFCTranslatorIdentifier> ifcExportTranslators;
	GSErrCode err = ACAPI_IFC_GetIFCExportTranslatorsList (ifcExportTranslators);
	if (err != NoError) {
		WriteReport ("Can't get IFC export translator");
		return;
	}

	if (DBVERIFY (!ifcExportTranslators.IsEmpty ())) {
		firstTranslator = ifcExportTranslators.GetFirst ();
	}

	API_SavePars_Ifc	pars_ifc = {};
	pars_ifc.subType = API_IFC;
	pars_ifc.translatorIdentifier = firstTranslator;
	pars_ifc.elementsToIfcExport = API_VisibleElementsOnAllStories;
	pars_ifc.elementsSet = nullptr;

	API_FileSavePars	fsp = {};
	fsp.fileTypeID = APIFType_IfcFile;
	IO::Location outputIFCFile (GetSpecialFolderLocation (API_UserDocumentsFolderID), IO::Name ("IFCTest.ifc"));
	fsp.file = &outputIFCFile;
	err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_ifc);
	if (err != NoError) {
		ACAPI_WriteReport ("Error in APIDo_SaveID (IFC): %s", true, ErrID_To_Name (err));
	}
}		// Do_Save_IfcFile


// -----------------------------------------------------------------------------
// Save the current plan to an Pdf file
// -----------------------------------------------------------------------------

static void		Do_Save_PdfFile (void)
{
	API_FileSavePars	fsp = {};
	fsp.fileTypeID			= APIFType_PdfFile;
	IO::Location outputFile (GetSpecialFolderLocation (API_UserDocumentsFolderID), IO::Name ("PDFTest.pdf"));
	fsp.file = &outputFile;

	API_SavePars_Pdf pdfPars	= {};
	pdfPars.sizeX				= 1000;
	pdfPars.sizeY				= 1000;

	const GSErrCode err = ACAPI_Automate (APIDo_SaveID, &fsp, &pdfPars);
	if (err != NoError) {
		ACAPI_WriteReport ("Error in APIDo_SaveID (PDF): %s", true, ErrID_To_Name (err));
	}
}		// Do_Save_PdfFile


// -----------------------------------------------------------------------------
// Save the current plan to a SAF file
// -----------------------------------------------------------------------------

static void		Do_Save_SafFile (void) 
{
	API_FileSavePars	fsp = {};
	fsp.fileTypeID = APIFType_SafFile;
	IO::Location outputFile (GetSpecialFolderLocation (API_UserDocumentsFolderID), IO::Name ("SAFTest.xlsx"));
	fsp.file = &outputFile;

	API_SavePars_Saf safPars = {};
	safPars.elementsToSAFExport = API_SAFEntireProject;

	const GSErrCode err = ACAPI_Automate (APIDo_SaveID, &fsp, &safPars);
	if (err != NoError) {
		ACAPI_WriteReport ("Error in APIDo_SaveID (SAF): %s", true, ErrID_To_Name (err));
	}
}		// Do_Save_SafFile


// -----------------------------------------------------------------------------
// Load libraries
// -----------------------------------------------------------------------------

static void		Do_LoadLibraries (void)
{

	GSErrCode err = ACAPI_Automate (APIDo_ReloadLibrariesID);
	if (err != NoError) {
		ACAPI_WriteReport ("Error in APIDo_ReloadLibrariesID: %s", true, ErrID_To_Name (err));
	}


	API_LibraryInfo libInfo;
	IO::Location outputLoc (GetSpecialFolderLocation (API_ApplicationFolderID), IO::Name ("Library Examples"));
	libInfo.location = outputLoc;
	GS::Array<API_LibraryInfo>	lip = {libInfo};
	err = ACAPI_Automate (APIDo_LoadLibrariesID, &lip);
	if (err != NoError) {
		ACAPI_WriteReport ("Error in APIDo_LoadLibrariesID: %s", true, ErrID_To_Name (err));
	}
}		// Do_LoadLibraries


// -----------------------------------------------------------------------------
// Print the current view (window and zoom)
// -----------------------------------------------------------------------------

static void		Do_Print (void)
{
	API_PrintPars	pi = {};
	pi.grid			= true;
	pi.printArea	= PrintArea_CurrentView;
	pi.fixText		= false;
	pi.scale		= 20;

	const GSErrCode err = ACAPI_Automate (APIDo_PrintID, &pi);
	if (err != NoError) {
		ACAPI_WriteReport ("Error in APIDo_PrintID: %s", true, ErrID_To_Name (err));
	}
}		// Do_Print


// -----------------------------------------------------------------------------
// Show Selection/All In 3D
// -----------------------------------------------------------------------------

static void Do_Show3D (bool onlySelection)
{
	const API_AutomateID code = onlySelection ? APIDo_ShowSelectionIn3DID : APIDo_ShowAllIn3DID;

	const GSErrCode err = ACAPI_Automate (code);
	if (err != NoError) {
		ACAPI_WriteReport ("Error in %s: %s", true,
		                   (onlySelection ? "APIDo_ShowSelectionIn3DID" : "APIDo_ShowAllIn3DID"),
		                   ErrID_To_Name (err));
	}
}		// Do_Show3D


// -----------------------------------------------------------------------------
// Zoom In/Out
// -----------------------------------------------------------------------------

static void Do_Zoom (bool zoomIn)
{
	API_WindowInfo	currWindowInfo = {};
	GSErrCode err = ACAPI_Database (APIDb_GetCurrentWindowID, &currWindowInfo);
	if (err != NoError) {
		return;
	}

	API_Box box = {};
	API_Rect rect = {};

	if (currWindowInfo.typeID == APIWind_3DModelID) {
		API_3DWindowInfo api3DWindowInfo = {};
		ACAPI_Environment (APIEnv_Get3DWindowSetsID, &api3DWindowInfo);
		rect.right = api3DWindowInfo.hSize;
		rect.bottom = api3DWindowInfo.vSize;
		const short inset = (zoomIn ? 50 : -50);
		rect.left += inset;
		rect.top += inset;
		rect.right -= inset;
		rect.bottom -= inset;
		err = ACAPI_Automate (APIDo_ZoomID, nullptr, &rect);

	} else {
		API_Tranmat tr = {};
		ACAPI_Database (APIDb_GetZoomID, &box, &tr);
		const double ratio = (zoomIn ? 0.2 : -0.2);
		const double dx = ratio * (box.xMax - box.xMin);
		const double dy = ratio * (box.yMax - box.yMin);
		box.xMin += dx;
		box.yMin += dy;
		box.xMax -= dx;
		box.yMax -= dy;

		// rotate grid by 30 degrees
		if (zoomIn) {
			tr.tmx[0] = tr.tmx[5] = sqrt(3.0) / 2.0;
			tr.tmx[1] = -0.5;
			tr.tmx[4] = 0.5;
			err = ACAPI_Database (APIDb_SetZoomID, &box, &tr);
		} else {
			err = ACAPI_Database (APIDb_SetZoomID, &box);
		}
	}

	ACAPI_WriteReport ("Automate Functions - APIDo_ZoomID box(%.4f,%.4f,%.4f,%.4f), rect(%d,%d,%d,%d)%s", err != NoError,
						box.xMin, box.yMin, box.xMax, box.yMax,
						rect.left, rect.bottom, rect.right, rect.top,
						(err != NoError) ? " - FAILED" : "");
}		// Do_Zoom


// -----------------------------------------------------------------------------
// Fit In Window
// -----------------------------------------------------------------------------

static void Do_FitInWindow (void)
{
	ACAPI_Automate (APIDo_ZoomID);
}		// Do_FitInWindow

static void Do_ZoomToSelected (void)
{
	ACAPI_Automate (APIDo_ZoomToSelectedID);
}


// -----------------------------------------------------------------------------
// Change the front window
// 	 - Section: switch to the plan and scan for a selected cutplane
// 	 - Section/3D windows will be rebuilt automatically if necessary
// -----------------------------------------------------------------------------

static API_WindowInfo GetWindow (const API_WindowTypeID typeID, const GS::HashSet<API_NeigID>& neigsIDs, const GS::UniString& windowName)
{
	API_WindowInfo		windInfo = {};
						windInfo.typeID = typeID;

	API_SelectionInfo	selectionInfo;
	GS::Array<API_Neig>	selNeigs;
	GSErrCode err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, false);
	if (err == APIERR_NOSEL || selectionInfo.typeID == API_SelEmpty) {
		ACAPI_WriteReport ("Nothing is selected", true);
		return {};
	}

	if (err != NoError) {
		ACAPI_WriteReport ("Error in ACAPI_Selection_Get: %s", true, ErrID_To_Name (err));
		return {};
	}

	for (Int32 i = 0; i < selectionInfo.sel_nElem; i++) {
		if (neigsIDs.Contains (selNeigs[i].neigID)) {
			API_Element	element = {};
			element.header.guid = selNeigs[i].guid;
			err = ACAPI_Element_Get (&element);
			if (err == NoError) {
				windInfo.databaseUnId = element.cutPlane.segment.databaseID;
				break;
			}
		}
	}
	if (windInfo.databaseUnId.elemSetId == APINULLGuid) {
		ACAPI_WriteReport ("# No " + windowName + " selected - select one and try again", true);
		return {};
	}
	return windInfo;
}

static API_WindowInfo GetSectionWindow ()
{
	API_DatabaseInfo	origDB = {};
	ACAPI_Database (APIDb_GetCurrentDatabaseID, &origDB);

	API_DatabaseInfo	planDB = {};
	planDB.typeID = APIWind_FloorPlanID;
	const GSErrCode err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &planDB);
	if (err != NoError) {
		return {};
	}

	const API_WindowInfo windInfo = GetWindow (APIWind_SectionID, {APINeig_CutPlane, APINeig_CutPlOn}, "section");

	ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &origDB);
	return windInfo;
}


static API_WindowInfo GetDetailWindow ()
{
	return GetWindow (APIWind_DetailID, {APINeig_Detail, APINeig_DetailPoly, APINeig_DetailPolyOn, APINeig_DetailMarker}, "detail");
}

static API_WindowInfo GetWorksheetWindow ()
{
	return GetWindow (APIWind_WorksheetID, {APINeig_Worksheet, APINeig_WorksheetPoly, APINeig_WorksheetPolyOn, APINeig_WorksheetMarker}, "worksheet");
}

static void Do_SwitchToWindow (const API_WindowTypeID typeID)
{
	API_WindowInfo windInfo = {};
	switch (typeID) {
		case APIWind_SectionID:
			windInfo = GetSectionWindow ();
			break;
		case APIWind_DetailID:
			windInfo = GetDetailWindow ();
			break;
		case APIWind_WorksheetID:
			windInfo = GetWorksheetWindow ();
			break;
		default:
			windInfo.typeID = typeID;
			break;
	}
	ACAPI_Automate (APIDo_ChangeWindowID, &windInfo);
}		// Do_SwitchToWindow


////////////////////////////////////////////////////////////////////////////////
// To show db-list

class ListDbDialog :	public DG::ModalDialog,
						public DG::ButtonItemObserver
{
public:
	ListDbDialog (API_WindowTypeID typeID, API_DatabaseID code);
	~ListDbDialog ();

// Results after running modal dlg
public:
	// nullptr=None
	API_DatabaseUnId* GetSelection ()
	{
		if (m_nListSelection < 0)
			return nullptr;

		return m_pDatabases + m_nListSelection;
	}

	bool GetChangeDb ()
	{
		return m_bChangeDb;
	}

protected:
	DG::Dialog& GetReference ()
	{
		return *this;
	}

protected:
	virtual void	ButtonClicked (const DG::ButtonClickEvent& ev) override;

private:
	enum {
		OkButtonId = 1,
		CancelButtonId = 2,
		DbListId = 4,
		DbCheckId = 5
	};

	DG::Button				m_ok;
	DG::Button				m_cancel;
	DG::SingleSelListBox	m_list;
	DG::CheckBox			m_check;
	API_DatabaseUnId*		m_pDatabases;

private:
	int						m_nListSelection;
	bool					m_bChangeDb;
};


ListDbDialog::ListDbDialog (API_WindowTypeID typeID, API_DatabaseID code)
  : ModalDialog (ACAPI_GetOwnResModule (), 32505, InvalidResModule),
	m_ok	(GetReference (), OkButtonId),
	m_cancel(GetReference (), CancelButtonId),
	m_list	(GetReference (), DbListId),
	m_check	(GetReference (), DbCheckId),
	m_pDatabases (nullptr),
	m_nListSelection (-1),
	m_bChangeDb (false)
{
	GSErrCode err = ACAPI_Database (code, (void *) &m_pDatabases);
	if (DBERROR(err))
		return;
	const USize nDBs = BMpGetSize ((GS::GSPtr) m_pDatabases) / Sizeof32(*m_pDatabases);
	if (err == NoError && m_pDatabases != nullptr && nDBs != 0) {
		for (UIndex index = 0; index < nDBs; ++index) {
			API_DatabaseInfo	info = {};
			info.typeID = typeID;
			info.databaseUnId = m_pDatabases[index];
			err = ACAPI_Database (APIDb_GetDatabaseInfoID, (void *) &info);
			if (DBERROR(err))
				continue;

			m_list.AppendItem ();
			m_list.SetTabItemText (DG::ListBox::BottomItem, 1, GS::UniString (info.title));
			m_list.SetItemValue (DG::ListBox::BottomItem, index);
		}
	}

	if (m_list.GetItemCount ())
		m_list.SelectItem (1);

	m_list.EnableSeparatorLines (true);

	m_ok.Attach (*this);
	m_cancel.Attach (*this);
}


ListDbDialog::~ListDbDialog ()
{
	BMpFree ((GS::GSPtr) m_pDatabases);

	m_ok.Detach (*this);
	m_cancel.Detach (*this);
}


void ListDbDialog::ButtonClicked (const DG::ButtonClickEvent& ev)
{
	if (ev.GetSource () == &m_ok) {
		// Save results
		const short sel = m_list.GetSelectedItem ();
		if (sel >= 1)
			m_nListSelection = TruncateTo32Bit (m_list.GetItemValue (sel));

		m_bChangeDb = m_check.IsChecked ();

		PostCloseRequest (Accept);
	} else if (ev.GetSource () == &m_cancel)
		PostCloseRequest (Cancel);
}


// -----------------------------------------------------------------------------
// Shows list of databases and changes the front window.
// Works with sections, elevations, interior elevations, details, layouts and
//  master layouts
// -----------------------------------------------------------------------------

static void Do_EnumerateAndSwitchToWindow (API_DatabaseTypeID typeID)
{
	API_DatabaseID		code;

	switch (typeID) {
	case APIWind_SectionID:
		code = APIDb_GetSectionDatabasesID;
		break;

	case APIWind_ElevationID:
		code = APIDb_GetElevationDatabasesID;
		break;

	case APIWind_InteriorElevationID:
		code = APIDb_GetInteriorElevationDatabasesID;
		break;

	case APIWind_DetailID:
		code = APIDb_GetDetailDatabasesID;
		break;

	case APIWind_WorksheetID:
		code = APIDb_GetWorksheetDatabasesID;
		break;

	case APIWind_LayoutID:
		code = APIDb_GetLayoutDatabasesID;
		break;

	case APIWind_MasterLayoutID:
		code = APIDb_GetMasterLayoutDatabasesID;
		break;

	default:
		DBBREAK ();
		return;
	}

	ListDbDialog dlg (typeID, code);
	if (!dlg.Invoke ()) {
		return;
	}

	const API_DatabaseUnId*	pDbUnId = dlg.GetSelection ();
	if (pDbUnId == nullptr) {
		return;
	}

	API_DatabaseInfo	prevDbInfo;
	ACAPI_Database (APIDb_GetCurrentDatabaseID, &prevDbInfo);

	// Activate window or just db in background
	if (!dlg.GetChangeDb ()) {
		API_WindowInfo	windInfo = {};
		windInfo.typeID = typeID;
		windInfo.databaseUnId = *pDbUnId;

		const GSErrCode err = ACAPI_Automate (APIDo_ChangeWindowID, &windInfo);
		if (DBERROR (err != NoError)) {
			return;
		}
	} else {
		API_DatabaseInfo	dbInfo = {};
		dbInfo.typeID = typeID;
		dbInfo.databaseUnId = *pDbUnId;

		const GSErrCode err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &dbInfo);
		if (DBERROR (err != NoError)) {
			return;
		}
	}



	// Dump number of labels
	GS::Array<API_Guid> labelList;
	ACAPI_Element_GetElemList (API_LabelID, &labelList);
	ACAPI_WriteReport ("Number of labels in active db=%d", false, labelList.GetSize ());

	// Switch back if we changed here
	if (dlg.GetChangeDb ()) {
		ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &prevDbInfo);
	}
}

// To show db-list
////////////////////////////////////////////////////////////////////////////////



// -----------------------------------------------------------------------------
// Menu command handler
// -----------------------------------------------------------------------------

static GSErrCode __ACENV_CALL	MenuCommandProc (const API_MenuParams* menuParams)
{
	DBPrintf ("Automate Functions Add-On User Menu called, ResID: %d, Item No.: %d\n",
				menuParams->menuItemRef.menuResID, menuParams->menuItemRef.itemIndex);

	switch (menuParams->menuItemRef.itemIndex) {
		case  1:	Do_New ();									break;
		case  2:	Do_Open ();									break;
		case  3:	Do_Close ();								break;
		case  4:	Do_Save_Plan ();							break;
		case  5:	Do_Save_TiffFile ();						break;
		case  6:	Do_Save_IfcFile ();							break;
		case  7:	Do_Save_PdfFile ();							break;
		case  8:	Do_Save_SafFile ();							break;
		case  9:	Do_LoadLibraries ();						break;
		case  10:	Do_Print ();								break;
		// -----
		case 12:	Do_Zoom (true);								break;
		case 13:	Do_Zoom (false);							break;
		case 14:	Do_ZoomToSelected();						break;
		case 15:	Do_FitInWindow ();							break;
		// ---
		case 17:	Do_Show3D (true);							break;
		case 18:	Do_Show3D (false);							break;
		// -----
		case 20:	Do_SwitchToWindow (APIWind_FloorPlanID);	break;
		case 21:	Do_SwitchToWindow (APIWind_SectionID);		break;
		case 22:	Do_SwitchToWindow (APIWind_DetailID);		break;
		case 23:	Do_SwitchToWindow (APIWind_WorksheetID);	break;
		case 24:	Do_SwitchToWindow (APIWind_3DModelID);		break;
		// -----
		case 26:	Do_EnumerateAndSwitchToWindow (APIWind_SectionID);				break;
		case 27:	Do_EnumerateAndSwitchToWindow (APIWind_ElevationID);			break;
		case 28:	Do_EnumerateAndSwitchToWindow (APIWind_InteriorElevationID);	break;
		case 29:	Do_EnumerateAndSwitchToWindow (APIWind_DetailID);				break;
		case 30:	Do_EnumerateAndSwitchToWindow (APIWind_MasterLayoutID);			break;
		case 31:	Do_EnumerateAndSwitchToWindow (APIWind_LayoutID);				break;
	}

	return NoError;
}


// =============================================================================
//
// Required functions
//
// =============================================================================
#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Called when the Add-On is going to be registered
// -----------------------------------------------------------------------------

API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (&envir->addOnInfo.name, 32000, 1, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, 32000, 2, ACAPI_GetOwnResModule ());

	return APIAddon_Normal;
}


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	RegisterInterface (void)
{
	// Register menus
	GSErrCode err = ACAPI_Register_Menu (32500, 32501, MenuCode_UserDef, MenuFlag_SeparatorBefore); //or MenuFlag_Default

	return err;
}


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	Initialize (void)
{
	ACAPI_Install_MenuHandler (32500, MenuCommandProc);

	return NoError;
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}		// FreeData
