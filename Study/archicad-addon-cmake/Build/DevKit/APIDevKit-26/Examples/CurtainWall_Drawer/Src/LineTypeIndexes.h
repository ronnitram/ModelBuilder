// *****************************************************************************
// Description:		CurtainWall_Drawer Test AddOn LineTypeIndexes class header
//					for managing the indexes of Line type elements
// *****************************************************************************

#ifndef LINETYPEINDEXES_HPP
#define LINETYPEINDEXES_HPP

#pragma once

class LineTypeIndexes
{
	short basicLineType;
	short hiddenLineType;

	void GetPreferences ();

	static LineTypeIndexes& GetNotConstInstance ();

	LineTypeIndexes ();

public:
	static const LineTypeIndexes& GetInstance ();

	short GetBasicLineType () const { return basicLineType; }
	short GetHiddenLineType () const { return hiddenLineType; }

	static void SetLineTypes (short basicLineType, short hiddenLineType);
};

#endif