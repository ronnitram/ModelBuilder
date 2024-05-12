// *****************************************************************************
// Description:		CurtainWall_Drawer Test AddOn LineTypeIndexes class for managing
//					the indexes of Line type elements
// *****************************************************************************

#include	"LineTypeIndexes.h"
#include "APIEnvir.h"
#include "ACAPinc.h"

void LineTypeIndexes::GetPreferences ()
{
	Int32       version;
	GSSize      nBytes;
	GS::FixArray<short, 2> basicHiddenType = {};

	ACAPI_GetPreferences (&version, &nBytes, nullptr);
	if (version == 0) {
		basicHiddenType[0] = 1;
		basicHiddenType[1] = 21;
	} else {
		ACAPI_GetPreferences (&version, &nBytes, &basicHiddenType);
	}

	basicLineType = basicHiddenType[0];
	hiddenLineType = basicHiddenType[1];
}

LineTypeIndexes::LineTypeIndexes ()
{
	GetPreferences ();
}

LineTypeIndexes& LineTypeIndexes::GetNotConstInstance ()
{
	static LineTypeIndexes indexes;
	return indexes;
}

const LineTypeIndexes& LineTypeIndexes::GetInstance ()
{
	return GetNotConstInstance ();
}

void LineTypeIndexes::SetLineTypes (short basic, short hidden)
{
	GS::FixArray<short, 2> basicHiddenType = { basic, hidden };

	ACAPI_SetPreferences (1, sizeof (basicHiddenType), &basicHiddenType);

	GetNotConstInstance ().basicLineType = basic;
	GetNotConstInstance ().hiddenLineType = hidden;
}
