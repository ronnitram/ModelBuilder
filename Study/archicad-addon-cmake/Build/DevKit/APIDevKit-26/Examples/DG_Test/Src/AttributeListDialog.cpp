// *****************************************************************************
// Source code for the AttributeList Example Dialog in DG Test Add-On
// *****************************************************************************

#include	"AttributeListDialog.hpp"

#include	"APIEnvir.h"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"

//---------------------------- Class AttributeListDialog -----------------------

AttributeListDialog::AttributeListDialog ():
	DG::ModalDialog		(ACAPI_GetOwnResModule (), ATTRIBUTE_LIST_DIALOG_RESID, ACAPI_GetOwnResModule ()),

	okButton			(GetReference (), OKButtonId),
	attributeList		(GetReference (), AttributeListId)
{
	Attach (*this);
	AttachToAllItems (*this);
	penItem.Attach (*this);
	fillItem.Attach (*this);
	checkItem.Attach (*this);
	selectionItem.Attach (*this);
	materialItem.Attach (*this);
	lineTypeItem.Attach (*this);
	textItem.Attach (*this);
	angleItem.Attach (*this);
	buttonItem.Attach (*this);

	attributeList.DisableUpdate ();

	// Init attribute list items
	firstGroup.SetName ("CUT ELEMENTS - FIRST GROUP");

	{
		selectionItem.SetName ("Fill Cut Surfaces with:");

		VBAL::APIAttributeListSelectionItem::Item item;

		item.text = "Uniform Pen Color";
		item.icon = DG::Icon (ACAPI_GetOwnResModule (), 27101);
		selectionItem.AddItem (item);

		item.text = "Own Material Color";
		item.icon = DG::Icon (ACAPI_GetOwnResModule (), 27102);
		selectionItem.AddItem (item);

		item.text = "Own Shaded Material Color";
		item.icon = DG::Icon (ACAPI_GetOwnResModule (), 27103);
		selectionItem.AddItem (item);

		firstGroup.AddItem (selectionItem);
	}

	checkItem.SetName ("Capture Material From 3D Cutting Planes Settings");
	checkItem.SetChecked (true);
	firstGroup.AddItem (checkItem);

	materialItem.SetName ("Uniform Material");
	materialItem.SetMaterial (56 /* Brick - Red */);
	firstGroup.AddItem (materialItem);

	penItem.SetName ("Cut Line Pen");
	penItem.SetIcon (DG::Icon (ACAPI_GetOwnResModule (), 27104));
	penItem.SetPen (24);
	firstGroup.AddItem (penItem);

	fillItem.SetName ("Cut Fill Pen");
	fillItem.SetIcon (DG::Icon (ACAPI_GetOwnResModule (), 27105));
	fillItem.SetFill (56 /* Block - Stack Bond */);
	firstGroup.AddItem (fillItem);

	secondGroup.SetName ("BOUNDARY CONTOURS - SECOND GROUP");

	lineTypeItem.SetName ("Boundary Line Type");
	lineTypeItem.SetIcon (DG::Icon (ACAPI_GetOwnResModule (), 27106));
	lineTypeItem.SetLineType (9 /* Zigzag */);
	secondGroup.AddItem (lineTypeItem);

	// Adding groups to the AttributeList
	attributeList.AddListItem (firstGroup);
	attributeList.AddListItem (secondGroup);

	attributeList.AddListItem (separatorItem);

	textItem.SetName ("Text Selection Item");
	// Custom will be the last item on the TextItem
	textItem.SetText ("Custom text");
	// The string items will be before the Custom
	textItem.AddStringItem ("First text");
	textItem.AddStringItem ("Second text");
	textItem.AddStringItem ("Third text");
	attributeList.AddListItem (textItem);

	angleItem.SetName ("Sun Azimuth");
	angleItem.SetIcon (DG::Icon (ACAPI_GetOwnResModule (), 27107));
	angleItem.SetValue (60);
	attributeList.AddListItem (angleItem);

	buttonItem.SetName ("Button Item");
	buttonItem.SetButtonText ("Click here!");
	attributeList.AddListItem (buttonItem);

	// Setting attributes
	selectionItem.SelectItem (1);
	materialItem.SetVisibility (selectionItem.GetSelected () == 1);
	materialItem.SetSubType (VBAL::APIAttributeListMaterialItem::ACMaterial);
	penItem.SetSubType (VBAL::APIAttributeListPenItem::GMPen);
	fillItem.SetSubType (VBAL::APIAttributeListFillItem::ACPolyFill);
	lineTypeItem.SetSubType (VBAL::APIAttributeListLineTypeItem::ACSymbolLine);

	// Update
	attributeList.EnableUpdate ();
	attributeList.FullRefreshList ();
}


AttributeListDialog::~AttributeListDialog ()
{
	attributeList.DisableUpdate ();

	Detach (*this);
	DetachFromAllItems (*this);
	penItem.Detach (*this);
	fillItem.Detach (*this);
	checkItem.Detach (*this);
	selectionItem.Detach (*this);
	materialItem.Detach (*this);
	lineTypeItem.Detach (*this);
	textItem.Detach (*this);
	angleItem.Detach (*this);
	buttonItem.Detach (*this);
}


void AttributeListDialog::PanelResized (const DG::PanelResizeEvent& ev)
{
	short hGrow = ev.GetHorizontalChange ();
	short vGrow = ev.GetVerticalChange ();
	if (hGrow != 0 || vGrow != 0) {
		BeginMoveResizeItems ();
		okButton.MoveAndResize (0, vGrow, hGrow, 0);
		attributeList.Resize (hGrow, vGrow);
		EndMoveResizeItems ();
	}
}


void AttributeListDialog::ListBoxSelectionChanged (const DG::ListBoxSelectionEvent& /*ev*/)
{
}


void	AttributeListDialog::PenChanged (const VBAL::APIAttributeListPenItem& source)
{
	if (&source == &penItem) {
		WriteReport ("Pen changed to: %d", source.GetPen ());
	}
}


void	AttributeListDialog::FillChanged (const VBAL::APIAttributeListFillItem& source)
{
	if (&source == &fillItem) {
		WriteReport ("Fill changed to: %d", source.GetFill ());
	}
}


void	AttributeListDialog::CheckChanged (const VBAL::APIAttributeListCheckItem& source)
{
	if (&source == &checkItem) {
		WriteReport ("Check changed to: %s", source.IsChecked () ? "true" : "false");
	}
}


void	AttributeListDialog::SelectionChanged (const VBAL::APIAttributeListSelectionItem& source)
{
	if (&source == &selectionItem) {
		WriteReport ("Selection changed to: %d", source.GetSelected ());
	}
}


void	AttributeListDialog::MaterialChanged (const VBAL::APIAttributeListMaterialItem& source)
{
	if (&source == &materialItem) {
		WriteReport ("Material changed to: %d", source.GetMaterial ());
	}
}


void	AttributeListDialog::LineTypeChanged (const VBAL::APIAttributeListLineTypeItem& source)
{
	if (&source == &lineTypeItem) {
		WriteReport ("LineType changed to: %d", source.GetLineType ());
	}
}


void	AttributeListDialog::TextChanged (const VBAL::APIAttributeListTextItem& source)
{
	if (&source == &textItem) {
		WriteReport ("Text changed to: %s", source.GetText ().ToCStr ().Get ());
	}
}


void	AttributeListDialog::ValueChanged (const VBAL::APIAttributeListRealItem& source)
{
	if (&source == &angleItem) {
		WriteReport ("Value changed to: %f", source.GetValue ());
	}
}


void	AttributeListDialog::ButtonClicked (const VBAL::APIAttributeListButtonItem& source)
{
	if (&source == &buttonItem) {
		WriteReport ("Button clicked");
	}
}


void	AttributeListDialog::ButtonClicked (const DG::ButtonClickEvent& ev)
{
	if (ev.GetSource () == &okButton) {
		PostCloseRequest (Accept);
	}
}
