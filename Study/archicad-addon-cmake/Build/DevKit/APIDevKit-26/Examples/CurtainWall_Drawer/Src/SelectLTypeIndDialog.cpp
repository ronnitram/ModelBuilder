// *****************************************************************************
// Description:		CurtainWall_Drawer Test AddOn SelectLTypeIndDialog class for
//					managing the line type index dialog
// *****************************************************************************

#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"SelectLTypeIndDialog.h"
#include	"LineTypeIndexes.h"

SelectLTypeIndDialog::SelectLTypeIndDialog () :
	DG::ModalDialog (ACAPI_GetOwnResModule (), 32500, ACAPI_GetOwnResModule ()),
	okButton (GetReference (), OKButtonID),
	cancelButton (GetReference (), CancelButtonID),
	basicLine (GetReference (), BasicLeftTextID),
	hiddenLine (GetReference (), HiddenLeftTextID),
	basicLineControl (GetReference (), BasicUserControlID),
	hiddenLineControl (GetReference (), HiddenUserControlID)
{
	API_UCCallbackType	ucb = {};
	ucb.type = APIUserControlType_SymbolLine;
	ucb.dialogID = 32500;
	ucb.itemID = BasicUserControlID;
	ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, nullptr);
	ucb.itemID = HiddenUserControlID;
	ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, nullptr);

	basicLineControl.SetValue (LineTypeIndexes::GetInstance ().GetBasicLineType ());
	hiddenLineControl.SetValue (LineTypeIndexes::GetInstance ().GetHiddenLineType ());

	AttachToAllItems (*this);
	Attach (*this);
}

SelectLTypeIndDialog::~SelectLTypeIndDialog ()
{
}

void SelectLTypeIndDialog::ButtonClicked (const DG::ButtonClickEvent& ev)
{
	if (ev.GetSource () == &cancelButton)
		PostCloseRequest (Cancel);
	else if (ev.GetSource () == &okButton)
		PostCloseRequest (Accept);
}

void SelectLTypeIndDialog::PanelCloseRequested (const DG::PanelCloseRequestEvent& ev, bool* /*accepted*/)
{
	if (ev.IsAccepted ()) {
		LineTypeIndexes::SetLineTypes ((short)basicLineControl.GetValue (), (short)hiddenLineControl.GetValue ());
	}
}
