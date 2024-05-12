// *****************************************************************************
// Header file for the AttributeList Example Dialog in DG Test Add-On
// *****************************************************************************

#ifndef ATTRIBUTELISTDIALOG_H
#define ATTRIBUTELISTDIALOG_H

#pragma once

#include	"DGModule.hpp"
#include	"UCModule.hpp"
#include	"APIAttributeList.hpp"


#define ATTRIBUTE_LIST_DIALOG_RESID			32590

// --- AttributeListDialog -----------------------------------------------------

class AttributeListDialog: public DG::ModalDialog,
						   public DG::PanelObserver,
						   public DG::ListBoxObserver,
						   public DG::ButtonItemObserver,
						   public VBAL::APIAttributeListPenItemObserver,
						   public VBAL::APIAttributeListFillItemObserver,
						   public VBAL::APIAttributeListCheckItemObserver,
						   public VBAL::APIAttributeListMaterialItemObserver,
						   public VBAL::APIAttributeListLineTypeItemObserver,
						   public VBAL::APIAttributeListTextItemObserver,
						   public VBAL::APIAttributeListRealItemObserver,
						   public VBAL::APIAttributeListButtonItemObserver,
						   public DG::CompoundItemObserver
{
private:

	enum {
		OKButtonId			= 1,
		AttributeListId		= 2
	};

	DG::Button								okButton;
	VBAL::APIAttributeList				attributeList;

	// Groups
	VBAL::APIAttributeListGroupItem		firstGroup;
	VBAL::APIAttributeListGroupItem		secondGroup;

	// Items
	VBAL::APIAttributeListPenItem		penItem;
	VBAL::APIAttributeListFillItem		fillItem;
	VBAL::APIAttributeListCheckItem		checkItem;
	VBAL::APIAttributeListSelectionItem	selectionItem;
	VBAL::APIAttributeListMaterialItem	materialItem;
	VBAL::APIAttributeListLineTypeItem	lineTypeItem;
	VBAL::APIAttributeListTextItem		textItem;
	VBAL::APIAttributeListAngleItem		angleItem;
	VBAL::APIAttributeListButtonItem		buttonItem;
	VBAL::APIAttributeListSeparatorItem	separatorItem;

	virtual void	PanelResized (const DG::PanelResizeEvent& ev) override;

	virtual void	ListBoxSelectionChanged (const DG::ListBoxSelectionEvent& ev) override;

	virtual void	ButtonClicked (const DG::ButtonClickEvent& ev) override;

	virtual void	PenChanged (const VBAL::APIAttributeListPenItem& source) override;
	virtual void	FillChanged (const VBAL::APIAttributeListFillItem& source) override;
	virtual void	CheckChanged (const VBAL::APIAttributeListCheckItem& source) override;
	virtual void	SelectionChanged (const VBAL::APIAttributeListSelectionItem& source) override;
	virtual void	MaterialChanged (const VBAL::APIAttributeListMaterialItem& source) override;
	virtual void	LineTypeChanged (const VBAL::APIAttributeListLineTypeItem& source) override;
	virtual void	TextChanged (const VBAL::APIAttributeListTextItem& source) override;
	virtual void	ValueChanged (const VBAL::APIAttributeListRealItem& source) override;
	virtual void	ButtonClicked (const VBAL::APIAttributeListButtonItem& source) override;

public:

	AttributeListDialog ();
	~AttributeListDialog ();
};

#endif // ATTRIBUTELISTDIALOG_H
