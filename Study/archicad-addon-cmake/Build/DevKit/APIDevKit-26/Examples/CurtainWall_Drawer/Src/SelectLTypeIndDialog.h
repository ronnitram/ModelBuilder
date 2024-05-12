// *****************************************************************************
// Description:		CurtainWall_Drawer Test AddOn SelectLTypeIndDialog class header
//					for managing the line type index dialog
// *****************************************************************************

#ifndef SELECTLTYPEINDDIALOG_HPP
#define SELECTLTYPEINDDIALOG_HPP

#pragma once

#include	"DGModule.hpp"

class SelectLTypeIndDialog : public DG::ModalDialog,
							 public DG::ButtonItemObserver,
							 public DG::CompoundItemObserver,
							 public DG::PanelObserver
{
protected:
	enum Controls {
		OKButtonID = 1,
		CancelButtonID = 2,
		BasicLeftTextID = 3,
		HiddenLeftTextID = 4,
		BasicUserControlID = 5,
		HiddenUserControlID = 6
	};

	DG::Button			okButton;
	DG::Button			cancelButton;
	DG::LeftText		basicLine;
	DG::LeftText		hiddenLine;
	DG::UserControl		basicLineControl;
	DG::UserControl	    hiddenLineControl;

	virtual void ButtonClicked (const DG::ButtonClickEvent& ev) override;
	virtual	void PanelCloseRequested (const DG::PanelCloseRequestEvent& ev, bool* accepted) override;
public:
	SelectLTypeIndDialog ();
	~SelectLTypeIndDialog ();
};

#endif