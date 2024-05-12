#include "ModifyElements.hpp"
#include "Reporter.hpp"

// API
#include "ACAPinc.h"

// ACAPI
#include "ACAPI/Result.hpp"

// MEPAPI
#include "ACAPI/MEPAdapter.hpp"

#include "ACAPI/MEPElement.hpp"
#include "ACAPI/MEPModifiableElement.hpp"

#include "ACAPI/MEPPort.hpp"
#include "ACAPI/MEPVentilationPort.hpp"
#include "ACAPI/MEPPipingPort.hpp"

#include "ACAPI/MEPRoutingElement.hpp"
#include "ACAPI/MEPRoutingElementDefault.hpp"
#include "ACAPI/MEPRoutingSegment.hpp"
#include "ACAPI/MEPRoutingSegmentDefault.hpp"
#include "ACAPI/MEPRigidSegmentDefault.hpp"
#include "ACAPI/MEPRoutingNodeDefault.hpp"
#include "ACAPI/MEPBendDefault.hpp"
#include "ACAPI/MEPTransitionDefault.hpp"

#include "ACAPI/MEPBranch.hpp"
#include "ACAPI/MEPTerminal.hpp"
#include "ACAPI/MEPFitting.hpp"
#include "ACAPI/MEPFlexibleSegment.hpp"

// GSUtils
#include "GSUnID.hpp"



using namespace ACAPI::MEP;


namespace MEPExample {


namespace {


GSErrCode ModifyPorts (const UniqueID& id)
{
// ! [Port-Modification-Example]
	ACAPI::Result<Element> mepElem = Element::Get (id);
	if (mepElem.IsErr ()) {
		ACAPI_WriteReport (mepElem.UnwrapErr ().text.c_str (), false);
		return mepElem.UnwrapErr ().kind;
	}

	for (const UniqueID& portID : mepElem->GetPortIDs ()) {
		ACAPI::Result<Port> port = Port::Get (portID);

		ACAPI::Result<void> modifyPortResult = (*port).Modify ([&portID](Port::Modifier& modifier) {
			modifier.SetShape (ConnectorShape::Circular);
			modifier.SetWidth (0.7);

			ACAPI::Result<VentilationPort> ventilationPort = VentilationPort::Get (portID);
			if (ventilationPort.IsOk ()) {
				ventilationPort->Modify ([](VentilationPort::Modifier& modifier) {
					modifier.SetWallThickness (0.004);
				}, "Ventilation port modification.");
			}

			ACAPI::Result<PipingPort> pipingPort = PipingPort::Get (portID);
			if (pipingPort.IsOk ()) {
				pipingPort->Modify ([](PipingPort::Modifier& modifier) {
					modifier.SetWallThickness (0.008);
					modifier.SetOuterDiameter (0.1);
				}, "Piping port modification.");
			}

		}, "Port properties modified.");

		if (modifyPortResult.IsErr ())
			return modifyPortResult.UnwrapErr ().kind;
	}

	return NoError;
// ! [Port-Modification-Example]
}


GSErrCode ModifyBranch (const UniqueID& id)
{
// ! [Branch-Modification-Example]
	ACAPI::Result<Branch> branch = Branch::Get (id);
	if (branch.IsErr ()) {
		ACAPI_WriteReport (branch.UnwrapErr ().text.c_str (), false);
		return branch.UnwrapErr ().kind;
	}

	bool canHaveInsulation = branch->GetDomain () != Domain::CableCarrier;
	ACAPI::Result<void> modifyResult = branch->Modify ([&canHaveInsulation](Branch::Modifier& modifier) -> GSErrCode {
		if (canHaveInsulation) {
			ACAPI::Result<void> result = modifier.SetInsulationThickness (0.1);
			if (result.IsErr ())
				return result.UnwrapErr ().kind;
		}

		return NoError;
	}, "Modify Parameters of Branch.");

	if (modifyResult.IsErr ())
		return modifyResult.UnwrapErr ().kind;

	return NoError;
// ! [Branch-Modification-Example]
}


GSErrCode ModifyRoutingMEPSystem (RoutingElement routingElement)
{
	RoutingElementDefault routingElementDefault = routingElement.PickUpDefault ();

	const API_AttributeIndex currentMEPSystem = routingElementDefault.GetMEPSystem ();

	// Search for a different compatible MEPSystem.
	std::optional<API_AttributeIndex> newMEPSystem;
	ACAPI_Attribute_EnumerateAttributesByType (API_MEPSystemID, [&](const API_Attribute& attribute) {
		if (newMEPSystem.has_value ())
			return;

		const bool isCompatible =
			(attribute.mepSystem.isForDuctwork && routingElementDefault.GetDomain () == Domain::Ventilation) ||
			(attribute.mepSystem.isForPipework && routingElementDefault.GetDomain () == Domain::Piping) ||
			(attribute.mepSystem.isForCabling && routingElementDefault.GetDomain () == Domain::CableCarrier);

		if (isCompatible && attribute.mepSystem.head.index != currentMEPSystem)
			newMEPSystem = attribute.mepSystem.head.index;
		});

	// Fallback to the Undefined MEPSystem if no other found.
	if (!newMEPSystem.has_value () && currentMEPSystem != APIInvalidAttributeIndex)
		newMEPSystem = APIInvalidAttributeIndex;

	if (newMEPSystem.has_value ()) {
		ACAPI::Result<void> elementDefaultModifyResult = routingElementDefault.Modify ([&](RoutingElementDefault::Modifier& modifier) {
			[[maybe_unused]] ACAPI::Result<void> setMEPSystemResult = modifier.SetMEPSystem (*newMEPSystem);
			DBASSERT (setMEPSystemResult.IsOk ());
		});

		if (elementDefaultModifyResult.IsErr ())
			return elementDefaultModifyResult.UnwrapErr ().kind;

		ACAPI::Result<void> injectResult = routingElement.Modify ([&routingElementDefault](RoutingElement::Modifier& modifier) {
			modifier.Inject (routingElementDefault);
		}, "Inject Default into RoutingElement.");

		if (injectResult.IsErr ())
			return injectResult.UnwrapErr ().kind;
	}

	return NoError;
}


GSErrCode ModifyRoutingSegment (RoutingSegment routingSegment)
{
// ! [RoutingSegment-Modification-Example]
	double currentWidth = routingSegment.GetCrossSectionWidth ();
	double currentHeight = routingSegment.GetCrossSectionHeight ();
	ACAPI::Result<void> modifyResult = routingSegment.Modify ([&](RoutingSegment::Modifier& modifier) {
		modifier.SetCrossSectionWidth (0.5 * currentWidth);
		modifier.SetCrossSectionHeight (0.5 * currentHeight);
	}, "Modify the dimensions of Cross Section of Routing Segment.");

	if (modifyResult.IsErr ())
		return modifyResult.UnwrapErr ().kind;

	return NoError;
// ! [RoutingSegment-Modification-Example]
}


GSErrCode ModifyDefaultRoutingSegment (RoutingElement routingElement)
{
// ! [RoutingSegmentDefault-Modification-Example]
	// PickUp RoutingElementDefault from RoutingElement
	RoutingElementDefault routingElementDefault = routingElement.PickUpDefault ();

	// Get RoutingSegmentDefault of RoutingElementDefault and modify it
	RoutingSegmentDefault routingSegmentDefault = routingElementDefault.GetRoutingSegmentDefault ();
	double currentWidth = routingSegmentDefault.GetCrossSectionWidth ();
	double currentHeight = routingSegmentDefault.GetCrossSectionHeight ();

	ACAPI::Result<void> segmentDefaultModifyResult = routingSegmentDefault.Modify ([&](RoutingSegmentDefault::Modifier& modifier) {
		modifier.SetCrossSectionWidth (2.0 * currentWidth);
		modifier.SetCrossSectionHeight (2.0 * currentHeight);
	});

	if (segmentDefaultModifyResult.IsErr ())
		return segmentDefaultModifyResult.UnwrapErr ().kind;

	// Set RoutingSegmentDefault back to RoutingElementDefault
	ACAPI::Result<void> elementDefaultModifyResult = routingElementDefault.Modify ([&](RoutingElementDefault::Modifier& modifier) {
		modifier.SetRoutingSegmentDefault (routingSegmentDefault);
	});

	if (elementDefaultModifyResult.IsErr ())
		return elementDefaultModifyResult.UnwrapErr ().kind;

	// Inject RoutingElementDefault into RoutingElement
	ACAPI::Result<void> injectResult = routingElement.Modify ([&routingElementDefault](RoutingElement::Modifier& modifier) {
		modifier.Inject (routingElementDefault);
	}, "Inject Default into RoutingElement.");

	if (injectResult.IsErr ())
		return injectResult.UnwrapErr ().kind;

	return NoError;
// ! [RoutingSegmentDefault-Modification-Example]
}


GSErrCode ModifyRoutingNode (RoutingNode routingNode)
{
// ! [RoutingNode-Modification-Example]
	const API_Coord3D& currentPosition = routingNode.GetPosition ();
	ACAPI::Result<void> modifyResult = routingNode.Modify ([&](RoutingNode::Modifier& modifier) {
		API_Coord3D position (currentPosition);
		position.z += 5.0;
		modifier.SetPosition (position);
	}, "Elevate a Node of Routing Element.");

	if (modifyResult.IsErr ())
		return modifyResult.UnwrapErr ().kind;

	return NoError;
// ! [RoutingNode-Modification-Example]
}


GSErrCode ModifyDefaultRoutingNode (RoutingElement routingElement)
{
// ! [RoutingNodeDefault-Modification-Example]
	// PickUp RoutingElementDefault from RoutingElement
	RoutingElementDefault routingElementDefault = routingElement.PickUpDefault ();

	// Get RoutingNodeDefault of RoutingElementDefault and modify it
	RoutingNodeDefault routingNodeDefault = routingElementDefault.GetRoutingNodeDefault ();
	ACAPI::Result<void> nodeDefaultModifyResult = routingNodeDefault.Modify ([&](RoutingNodeDefault::Modifier& modifier) {
		modifier.SetPreferredTransitionPlacement (PreferredTransitionPlacement::LargeToSmall);
	});

	if (nodeDefaultModifyResult.IsErr ())
		return nodeDefaultModifyResult.UnwrapErr ().kind;

	// Get BendDefault and modify it
	BendDefault bendDefault = routingNodeDefault.GetBendDefault ();
	ACAPI::Result<void> bendDefaultModifyResult = bendDefault.Modify ([&](BendDefault::Modifier& modifier) {
		modifier.SetFactorRadius (0.6);
	});

	if (bendDefaultModifyResult.IsErr ())
		return bendDefaultModifyResult.UnwrapErr ().kind;

	// Get TransitionDefault and modify it
	TransitionDefault transitionDefault = routingNodeDefault.GetTransitionDefault ();
	ACAPI::Result<void> transitionDefaultModifyResult = transitionDefault.Modify ([&](TransitionDefault::Modifier& modifier) {
		modifier.SetAngle (35.0);
	});

	if (transitionDefaultModifyResult.IsErr ())
		return transitionDefaultModifyResult.UnwrapErr ().kind;

	// Set BendDefault and TransitionDefault back to RoutingNodeDefault
	ACAPI::Result<void> nodeDefaultSetBackResult = routingNodeDefault.Modify ([&](RoutingNodeDefault::Modifier& modifier) {
		modifier.SetBendDefault (bendDefault);
		modifier.SetTransitionDefault (transitionDefault);
	});

	if (nodeDefaultSetBackResult.IsErr ())
		return nodeDefaultSetBackResult.UnwrapErr ().kind;

	// Set RoutingNodeDefault back to RoutingElementDefault
	ACAPI::Result<void> elementDefaultModifyResult = routingElementDefault.Modify ([&](RoutingElementDefault::Modifier& modifier) {
		modifier.SetRoutingNodeDefault (routingNodeDefault);
	});

	if (elementDefaultModifyResult.IsErr ())
		return elementDefaultModifyResult.UnwrapErr ().kind;

	// Inject RoutingElementDefault into RoutingElement
	ACAPI::Result<void> injectResult = routingElement.Modify ([&routingElementDefault](RoutingElement::Modifier& modifier) {
		modifier.Inject (routingElementDefault);
	}, "Inject Default into RoutingElement.");

	if (injectResult.IsErr ())
		return injectResult.UnwrapErr ().kind;

	return NoError;
// ! [RoutingNodeDefault-Modification-Example]
}


GSErrCode ModifyRoutingElement (const UniqueID& id)
{
// ! [RoutingElement-Modification-Example]
	ACAPI::Result<RoutingElement> routingElement = RoutingElement::Get (id);
	if (routingElement.IsErr ()) {
		ACAPI_WriteReport (routingElement.UnwrapErr ().text.c_str (), false);
		return routingElement.UnwrapErr ().kind;
	}

	ERRCHK_NO_ASSERT (ModifyRoutingMEPSystem (*routingElement));

	std::vector<UniqueID> routingNodeIds = routingElement->GetRoutingNodeIds ();
	std::vector<UniqueID> routingSegmentIds = routingElement->GetRoutingSegmentIds ();

	ACAPI::Result<RoutingNode> node = RoutingNode::Get (routingNodeIds[4]);
	if (node.IsErr ())
		return node.UnwrapErr ().kind;

	ERRCHK_NO_ASSERT (ModifyRoutingNode (*node));

	ACAPI::Result<RoutingSegment> segment = RoutingSegment::Get (routingSegmentIds[4]);
	if (segment.IsErr ())
		return segment.UnwrapErr ().kind;

	ERRCHK_NO_ASSERT (ModifyRoutingSegment (*segment));

	ERRCHK_NO_ASSERT (ModifyDefaultRoutingNode (*routingElement));
	ERRCHK_NO_ASSERT (ModifyDefaultRoutingSegment (*routingElement));

	return NoError;
// ! [RoutingElement-Modification-Example]
}


GSErrCode ModifyFlexibleSegment (const UniqueID& id)
{
// ! [FlexibleSegment-Modification-Example]
	ACAPI::Result<FlexibleSegment> flexibleSegment = FlexibleSegment::Get (id);
	if (flexibleSegment.IsErr ()) {
		ACAPI_WriteReport (flexibleSegment.UnwrapErr ().text.c_str (), false);
		return flexibleSegment.UnwrapErr ().kind;
	}

	std::vector<API_Coord3D> controlPoints = flexibleSegment->GetControlPoints ();

	for (API_Coord3D& controlPoints : controlPoints)
		controlPoints.x += 1.0;

	ACAPI::Result<void> modifyResult = flexibleSegment->Modify ([&](FlexibleSegment::Modifier& modifier) {
		modifier.SetControlPoints (controlPoints);
	}, "Modify the control points of Flexible Segment.");

	return NoError;
// ! [FlexibleSegment-Modification-Example]
}


GSErrCode ModifyElement (const UniqueID& id)
{
// ! [ModifiableElement-Modification-Example]
	ACAPI::Result<ModifiableElement> element = ModifiableElement::Get (id);
	if (element.IsErr ()) {
		ACAPI_WriteReport (element.UnwrapErr ().text.c_str (), false);
		return element.UnwrapErr ().kind;
	}

	const API_Coord3D& currentPosition = element->GetAnchorPoint ();
	ACAPI::Result<void> modifyResult = element->Modify ([&currentPosition](ModifiableElement::Modifier& modifier) {
		API_Coord3D position (currentPosition);
		position.x += 5.0;
		modifier.SetAnchorPoint (position);
		Orientation newOrientation = { { 0.0, 1.0, 0.0 }, { 1.0, 0.0, 0.0 } };
		modifier.SetOrientation (newOrientation);
	}, "Change position and orientation of MEP Element.");

	return NoError;
// ! [ModifiableElement-Modification-Example]
}


API_Guid GetRigidSegmentClassIDFromRoutingElemClassID (const API_Guid& routingElemClassID) 	
{
	if (routingElemClassID == ACAPI::MEP::VentilationRoutingID)
		return ACAPI::MEP::VentilationRigidSegmentID;

	if (routingElemClassID == ACAPI::MEP::PipingRoutingID)
		return ACAPI::MEP::PipingRigidSegmentID;

	if (routingElemClassID == ACAPI::MEP::CableCarrierRoutingID)
		return ACAPI::MEP::CableCarrierRigidSegmentID;

	return APINULLGuid;
}


API_Guid GetBendClassIDFromRoutingElemClassID (const API_Guid& routingElemClassID) 	
{
	if (routingElemClassID == ACAPI::MEP::VentilationRoutingID)
		return ACAPI::MEP::VentilationBendID;

	if (routingElemClassID == ACAPI::MEP::PipingRoutingID)
		return ACAPI::MEP::PipingBendID;

	if (routingElemClassID == ACAPI::MEP::CableCarrierRoutingID)
		return ACAPI::MEP::CableCarrierBendID;

	return APINULLGuid;
}


API_Guid GetTransitionClassIDFromRoutingElemClassID (const API_Guid& routingElemClassID) 	
{
	if (routingElemClassID == ACAPI::MEP::VentilationRoutingID)
		return ACAPI::MEP::VentilationTransitionID;

	if (routingElemClassID == ACAPI::MEP::PipingRoutingID)
		return ACAPI::MEP::PipingTransitionID;

	if (routingElemClassID == ACAPI::MEP::CableCarrierRoutingID)
		return ACAPI::MEP::CableCarrierTransitionID;

	return APINULLGuid;
}


void WriteGDLParameters (const API_ElementMemo& memo) 
{
	Reporter reporter;

	Int32 n = BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType);
	reporter.Add ("GDL Parameters:");
	reporter.AddNewLine ();

	for (Int32 i = 0; i < n; ++i) {
		if ((*memo.params)[i].typeMod == API_ParArray)
			continue;

		GS::UniString name ((*memo.params)[i].name);
		if ((*memo.params)[i].typeID == APIParT_CString) {
			GS::UniString stringValue ((*memo.params)[i].value.uStr);
			reporter.Add (std::move (name), std::move (stringValue));
		} else {
			reporter.Add (std::move (name), (*memo.params)[i].value.real);
		}
	}

	reporter.AddNewLine ();
	reporter.Write ();
}


void ChangeConnectionStyleParameterOfSubElemAndLog (API_ElementMemo& memo, const GS::UniString& subElemName, bool isDefault)
{
	Reporter reporter;

	if (isDefault)
		reporter.Add ("Information about the default " + subElemName);
	else
		reporter.Add ("Information about the selected element's " + subElemName);
	reporter.AddNewLine ();
	reporter.Write ();

	WriteGDLParameters (memo);

	Int32 index = 0;
	Int32 n = BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType);
	for (Int32 i = 0; i < n; i++) {
		if (CHCompareASCII ((*memo.params)[i].name, "MEP_StrConnectionData", CS_CaseInsensitive) == 0) {
			index = i;
			break;
		}
	}

	GS::uchar_t** hdl = (GS::uchar_t**) (*(memo.params))[index].value.array;
	Int32 dim1 = (*(memo.params))[index].dim1;
	Int32 dim2 = (*(memo.params))[index].dim2;

	reporter.AddNewLine ();
	reporter.Add ("Parameter MEP_StrConnectionData of " + subElemName + " successfully modified.");
	reporter.Write ();

	GS::UniString tmpUStr = "{179FC42C-A611-4465-9DF4-9926B7C64A8E}"; // Guid of Simple Body connection style

	const Int32 indexOfConnectionStyle = 0;

	Int32 lastPos = 0;
	for (Int32 i = 0; i < dim1; ++i) {
		for (Int32 j = 0; j < dim2; ++j) {
			if (j == indexOfConnectionStyle) {
				GS::ucsncpy (&(*hdl)[lastPos], tmpUStr.ToUStr (), GS::ucslen (tmpUStr.ToUStr ()) + 1);
			}
			lastPos += GS::ucslen32 (&(*hdl)[lastPos]) + 1;
		}
	}
}


GS::UniString GetLibPartNameFromGuid (const API_Guid guid) 	
{
	API_LibPart libPart = {};
	GS::UnID unId (APIGuid2GSGuid (guid), GS::NULLGuid);
	CHCopyC (unId.ToUniString ().ToCStr (), libPart.ownUnID);

	GSErrCode err = ACAPI_LibraryPart_Search (&libPart, false);
	if DBERROR (err != NoError)
		return GS::EmptyUniString;

	return libPart.file_UName;
}


GSErrCode GetLibPartUnIdFromName (const GS::UniString& name, GS::UnID& libPartUnID)
{ 
	API_LibPart libPart = {};
	GS::ucscpy (libPart.file_UName, name.ToUStr ());

	ERRCHK_NO_ASSERT (ACAPI_LibraryPart_Search (&libPart, false));
	
	libPartUnID = GS::UnID (libPart.ownUnID);
	return NoError;
}


} // namespace


GSErrCode ModifySelectedBranches ()
{
	GS::Array<API_Neig>	selNeigs;
	API_SelectionInfo 	selectionInfo = {};

	ERRCHK_NO_ASSERT (ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false));

	for (const API_Neig& neig : selNeigs) {
		API_Element	elem = {};
		elem.header.guid = neig.guid;
		ERRCHK_NO_ASSERT (ACAPI_Element_Get (&elem));

		if (elem.header.type.typeID == API_ExternalElemID && IsBranch (elem.header.type.classID)) {
			ERRCHK_NO_ASSERT (ModifyBranch (Adapter::UniqueID (neig.guid)));
			ERRCHK_NO_ASSERT (ModifyPorts (Adapter::UniqueID (neig.guid)));

			Reporter reporter (2);
			reporter.Add ("Branch modified successfully.");
			reporter.Write ();
		}
	}

	return NoError;
}


GSErrCode ModifySelectedRoutingElements ()
{
	GS::Array<API_Neig>	selNeigs;
	API_SelectionInfo 	selectionInfo = {};

	ERRCHK_NO_ASSERT (ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false));

	for (const API_Neig& neig : selNeigs) {
		API_Element	elem = {};
		elem.header.guid = neig.guid;
		ERRCHK_NO_ASSERT (ACAPI_Element_Get (&elem));

		if (elem.header.type.typeID == API_ExternalElemID && IsRoutingElement (elem.header.type.classID)) {
			ERRCHK_NO_ASSERT (ModifyRoutingElement (Adapter::UniqueID (neig.guid)));

			Reporter reporter (1);
			reporter.Add ("Routing Element modified successfully.");
			reporter.Write ();
		}
	}

	return NoError;
}


GSErrCode ModifySelectedFlexibleSegments ()
{
	GS::Array<API_Neig>	selNeigs;
	API_SelectionInfo 	selectionInfo = {};

	ERRCHK_NO_ASSERT (ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false));

	for (const API_Neig& neig : selNeigs) {
		API_Element	elem = {};
		elem.header.guid = neig.guid;
		ERRCHK_NO_ASSERT (ACAPI_Element_Get (&elem));

		if (elem.header.type.typeID == API_ExternalElemID && IsFlexibleSegment (elem.header.type.classID)) {
			ERRCHK_NO_ASSERT (ModifyFlexibleSegment (Adapter::UniqueID (neig.guid)));

			Reporter reporter (1);
			reporter.Add ("Flexible Segment modified successfully.");
			reporter.Write ();
		}
	}

	return NoError;
}


GSErrCode ShiftAndReorientSelectedMEPElements ()
{
	GS::Array<API_Neig>	selNeigs;
	API_SelectionInfo 	selectionInfo = {};

	ERRCHK_NO_ASSERT (ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false));

	for (const API_Neig& neig : selNeigs) {
		API_Element	elem = {};
		elem.header.guid = neig.guid;
		ERRCHK_NO_ASSERT (ACAPI_Element_Get (&elem));

		if (elem.header.type.typeID == API_ExternalElemID
			&& (IsBranch (elem.header.type.classID)
				|| IsAccessory (elem.header.type.classID)
				|| IsEquipment (elem.header.type.classID)
				|| IsTerminal (elem.header.type.classID)
				|| IsFitting (elem.header.type.classID)
				|| IsFlexibleSegment (elem.header.type.classID)))
		{
			ERRCHK_NO_ASSERT (ModifyElement (Adapter::UniqueID (neig.guid)));
		}
	}

	return NoError;
}


GSErrCode DeleteSelectedElements ()
{
	GS::Array<API_Neig>	selNeigs;
	API_SelectionInfo 	selectionInfo = {};

	ERRCHK_NO_ASSERT (ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false));

	GS::Array<API_Guid> toDelete;

	for (const API_Neig& neig : selNeigs) {
		API_Element	elem = {};
		elem.header.guid = neig.guid;
		ERRCHK_NO_ASSERT (ACAPI_Element_Get (&elem));

		if (elem.header.type.typeID == API_ExternalElemID
			&& (IsBranch (elem.header.type.classID)
				|| IsAccessory (elem.header.type.classID)
				|| IsEquipment (elem.header.type.classID)
				|| IsTerminal (elem.header.type.classID)
				|| IsFitting (elem.header.type.classID)
				|| IsFlexibleSegment (elem.header.type.classID)
				|| IsRoutingElement (elem.header.type.classID)))
		{
			toDelete.Push (elem.header.guid);
		}
	}

	ACAPI_CallUndoableCommand ("Delete selected elements", [&]() -> GSErrCode {
		return ACAPI_Element_Delete (toDelete);
	});

	return NoError;
}


GSErrCode ModifySelectedElemGeneralParameters () 
{
	Reporter reporter;

	GS::Array<API_Neig>	selNeigs;
	API_SelectionInfo 	selectionInfo = {};
	API_Element         mask = {};

	ERRCHK_NO_ASSERT (ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false));

	for (const API_Neig& neig : selNeigs) {
		API_Element	elem = {};
		elem.header.guid = neig.guid;
		ERRCHK_NO_ASSERT (ACAPI_Element_Get (&elem));

		if (elem.header.type.typeID == API_ExternalElemID) {

			ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, floorInd);
			elem.header.floorInd = 2;

			ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
			elem.header.layer = ACAPI_CreateAttributeIndex (10);

			ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, renovationStatus);
			elem.header.renovationStatus = API_ExistingStatus;

			ERRCHK_NO_ASSERT (ACAPI_CallUndoableCommand ("Change General Parameters", [&] () -> GSErrCode {
				return ACAPI_Element_Change (&elem, &mask, nullptr, 0, true);
			}));

			reporter.Add ("Floor index, layer and renovation status of the selected element successfully modified.");
			reporter.Write ();
		}
	}

	return NoError;
}


GSErrCode ModifySelectedPipeFittingGDLParameters ()
{
	Reporter reporter;

	GS::Array<API_Neig>	selNeigs;
	API_SelectionInfo 	selectionInfo = {};

	ERRCHK_NO_ASSERT (ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false));

	for (const API_Neig& neig : selNeigs) {
		API_Element	elem = {};
		elem.header.guid = neig.guid;
		ERRCHK_NO_ASSERT (ACAPI_Element_Get (&elem));

		if (elem.header.type.typeID == API_ExternalElemID && ACAPI::MEP::IsFitting (elem.header.type.classID)) {

			ACAPI::Result<Fitting> fitting = Fitting::Get (Adapter::UniqueID (neig.guid));
			if (fitting.IsErr ()) {
				ACAPI_WriteReport (fitting.UnwrapErr ().text.c_str (), false);
				return fitting.UnwrapErr ().kind;
			}
			if (fitting->GetDomain () != Domain::Piping)
				return NoError;

			API_ElementMemo		memo = {};
			API_Element         mask = {};

			ERRCHK_NO_ASSERT (ACAPI_Element_GetMemo (elem.header.guid, &memo));

			reporter.Add ("Information about the selected element");
			reporter.AddNewLine ();
			reporter.Write ();
			WriteGDLParameters (memo);

			Int32 n = BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType);
			Int32 i;
			for (i = 0; i < n; i++) {
				if (CHCompareASCII ((*memo.params)[i].name, "MEP_StraightLength", CS_CaseInsensitive) == 0)
					break;
			}

			(*memo.params)[i].value.real = 5.0;

			GSErrCode err = ACAPI_CallUndoableCommand ("Change General Parameters", [&] () -> GSErrCode {
				return ACAPI_Element_Change (&elem, &mask, &memo, APIMemoMask_AddPars, true);
			});

			ACAPI_DisposeElemMemoHdls (&memo);

			if (err != NoError)
				return err;

			reporter.Add ("Parameter MEP_StraightLength successfully changed.");
			reporter.Write ();
		}
	}

	return NoError;
}


GSErrCode ModifySelectedRoutingElemGDLParameters ()
{
	Reporter reporter;

	GS::Array<API_Neig>	selNeigs;
	API_SelectionInfo 	selectionInfo = {};

	ERRCHK_NO_ASSERT (ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false));

	for (const API_Neig& neig : selNeigs) {
		API_Element	elem = {};
		elem.header.guid = neig.guid;
		ERRCHK_NO_ASSERT (ACAPI_Element_Get (&elem));

		if (elem.header.type.typeID == API_ExternalElemID && ACAPI::MEP::IsRoutingElement (elem.header.type.classID)) {

			API_Element mask = {};

			API_ElemType mepElemType;
			mepElemType.typeID = API_ExternalElemID;
			mepElemType.variationID = APIVarId_Generic;

			const UInt32 subElemCount = 3;
			API_SubElement* subElemArray = (API_SubElement*) BMpAllClear (3 * sizeof (API_SubElement));

			mepElemType.classID = GetRigidSegmentClassIDFromRoutingElemClassID (elem.header.type.classID);
			subElemArray[0].subElem.header.type = mepElemType;

			mepElemType.classID = GetBendClassIDFromRoutingElemClassID (elem.header.type.classID);
			subElemArray[1].subElem.header.type = mepElemType;

			mepElemType.classID = GetTransitionClassIDFromRoutingElemClassID (elem.header.type.classID);
			subElemArray[2].subElem.header.type = mepElemType;

			ERRCHK_NO_ASSERT (ACAPI_Element_GetMemo_ExternalHierarchical (elem.header.guid, subElemCount, subElemArray));

			reporter.AddNewLine ();
			reporter.Write ();
			ChangeConnectionStyleParameterOfSubElemAndLog (subElemArray[0].memo, "Rigid Segment", false);
			ChangeConnectionStyleParameterOfSubElemAndLog (subElemArray[1].memo, "Bend", false);
			ChangeConnectionStyleParameterOfSubElemAndLog (subElemArray[2].memo, "Transition", false);

			GSErrCode err = ACAPI_CallUndoableCommand ("Change GDL parameters of selected route", [&] () -> GSErrCode {
				return ACAPI_Element_ChangeExt (&elem, &mask, nullptr, 0, subElemCount, subElemArray, true, 0);
			});

			ACAPI_DisposeElemMemoHdls (&subElemArray[0].memo);
			ACAPI_DisposeElemMemoHdls (&subElemArray[1].memo);
			ACAPI_DisposeElemMemoHdls (&subElemArray[2].memo);

			if (err != NoError)
				return err;
		}
	}

	return NoError;
}


GSErrCode ModifyPipeFittingDefaultParameters () 
{
	Reporter reporter;

	API_Element		element = {};
	API_ElementMemo	memo = {};

	API_ElemType mepElemType;
	mepElemType.typeID = API_ExternalElemID;
	mepElemType.classID = ACAPI::MEP::PipingFittingID;
	mepElemType.variationID = APIVarId_Generic;

	element.header.type = mepElemType;

	ERRCHK_NO_ASSERT (ACAPI_Element_GetDefaults (&element, &memo));

	WriteGDLParameters (memo);

	API_Element mask = {};
	ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, floorInd);
	element.header.floorInd = 2;

	Int32 n = BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType);
	Int32 index = 0;
	for (Int32 i = 0; i < n; i++) {
		if (CHCompareASCII ((*memo.params)[i].name, "MEP_StraightLength", CS_CaseInsensitive) == 0) {
			index = i;
			break;
		}
	}
	(*memo.params)[index].value.real = 5.0;

	GSErrCode err = ACAPI_Element_ChangeDefaults (&element, &memo, &mask);

	ACAPI_DisposeElemMemoHdls (&memo);

	if (err != NoError)
		return err;

	reporter.Add ("Floor Index successfully modified.");
	reporter.Add ("Parameter MEP_StraightLength successfully modified.");
	reporter.Write ();
	
	return NoError;
}


GSErrCode ModifyRoutingElemDefaultParameters () 
{
	Reporter reporter;

	API_Element element = {};

	API_ElemType mepElemType;
	mepElemType.typeID = API_ExternalElemID;
	mepElemType.classID = ACAPI::MEP::VentilationRoutingID;
	mepElemType.variationID = APIVarId_Generic;

	element.header.type = mepElemType;

	const UInt32 subElemCount = 3;
	API_SubElement* subElemArray = (API_SubElement*) BMpAllClear (subElemCount * sizeof (API_SubElement));

	mepElemType.classID = ACAPI::MEP::VentilationRigidSegmentID;
	subElemArray[0].subElem.header.type = mepElemType;

	mepElemType.classID = ACAPI::MEP::VentilationBendID;
	subElemArray[1].subElem.header.type = mepElemType;

	mepElemType.classID = ACAPI::MEP::VentilationTransitionID;
	subElemArray[2].subElem.header.type = mepElemType;

	ERRCHK_NO_ASSERT (ACAPI_Element_GetDefaultsExt (&element, nullptr, subElemCount, subElemArray));

	ChangeConnectionStyleParameterOfSubElemAndLog (subElemArray[0].memo, "Rigid Segment", true);
	ChangeConnectionStyleParameterOfSubElemAndLog (subElemArray[1].memo, "Bend", true);
	ChangeConnectionStyleParameterOfSubElemAndLog (subElemArray[2].memo, "Transition", true);

	API_Element mask = {};
	ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, floorInd);
	element.header.floorInd = 2;

	GSErrCode err = ACAPI_Element_ChangeDefaultsExt (&element, nullptr, &mask, subElemCount, subElemArray);

	ACAPI_DisposeElemMemoHdls (&subElemArray[0].memo);
	ACAPI_DisposeElemMemoHdls (&subElemArray[1].memo);
	ACAPI_DisposeElemMemoHdls (&subElemArray[2].memo);

	if (err != NoError)
		return err;

	reporter.Add ("Floor Index successfully modified.");
	reporter.Write ();

	return NoError;
}


GSErrCode ModifyLibraryPartOfSelectedPipeTerminal ()
{
	Reporter reporter;

	GS::UniString newLibpartName = "Dry Sprinkler 27.gsm";
	GS::UnID unId;
	ERRCHK_NO_ASSERT (GetLibPartUnIdFromName (newLibpartName, unId));

	GS::Array<API_Neig>	selNeigs;
	API_SelectionInfo 	selectionInfo = {};

	ERRCHK_NO_ASSERT (ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false));

	for (const API_Neig& neig : selNeigs) {
		API_Element	elem = {};
		elem.header.guid = neig.guid;
		ERRCHK_NO_ASSERT (ACAPI_Element_Get (&elem));

		if (IsTerminal (elem.header.type.classID)) {
			ACAPI::Result<Terminal> terminal = Terminal::Get (Adapter::UniqueID (neig.guid));
			if (terminal.IsErr ())
				return terminal.UnwrapErr ().kind;

			if (terminal->GetDomain () != Domain::Piping)
				return NoError;

			reporter.Add ("Current Library Part Name of Selected Pipe Terminal", GetLibPartNameFromGuid (terminal->GetObjectId ()));

			ACAPI::Result<void> modifyResult = terminal->Modify ([&] (Terminal::Modifier& modifier) {
				modifier.SetObjectId (GSGuid2APIGuid (unId.GetMainGuid ()));
			}, "Modify the library part of selected Pipe Terminal.");

			if (modifyResult.IsErr ())
				return modifyResult.UnwrapErr ().kind;

			reporter.Add ("New Library Part Name of Selected Pipe Terminal", newLibpartName);
			reporter.Write ();
		}
	}

	return NoError;
}


GSErrCode ModifyLibraryPartOfDefaultPipeTerminal () 
{
	Reporter reporter;

	GS::UniString newLibpartName = "Hose Connection 27.gsm";
	GS::UnID unId;
	ERRCHK_NO_ASSERT (GetLibPartUnIdFromName (newLibpartName, unId));

	ACAPI::Result<ACAPI::MEP::TerminalDefault> terminalDefault = ACAPI::MEP::CreateTerminalDefault (ACAPI::MEP::Domain::Piping);
	if (terminalDefault.IsErr ())
		return terminalDefault.UnwrapErr ().kind;

	reporter.Add ("Current Library Part Name of Default Pipe Terminal", GetLibPartNameFromGuid (terminalDefault->GetObjectId ()));

	ACAPI::Result<void> modifyResult = terminalDefault->Modify ([&] (ACAPI::MEP::FittingDefault::Modifier& modifier) {
		modifier.SetObjectId (GSGuid2APIGuid (unId.GetMainGuid ()));
	});

	if (modifyResult.IsErr ())
		return modifyResult.UnwrapErr ().kind;

	reporter.Add ("New Library Part Name of Default Pipe Terminal", newLibpartName);
	reporter.Write ();

	ACAPI::Result<void> setAsDefaultResult = terminalDefault->SetAsArchicadDefault ();
	if (setAsDefaultResult.IsErr ())
		return setAsDefaultResult.UnwrapErr ().kind;

	return NoError;
}


GSErrCode ModifyLibraryPartOfSelectedCableCarrierRoutesFirstRigidSegment ()
{
	Reporter reporter;

	GS::UniString newLibpartName = "Cable Tray 27.gsm";
	GS::UnID unId;
	ERRCHK_NO_ASSERT (GetLibPartUnIdFromName (newLibpartName, unId));

	GS::Array<API_Neig>	selNeigs;
	API_SelectionInfo 	selectionInfo = {};

	ERRCHK_NO_ASSERT (ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false));

	for (const API_Neig& neig : selNeigs) {
		API_Element	elem = {};
		elem.header.guid = neig.guid;
		ERRCHK_NO_ASSERT (ACAPI_Element_Get (&elem));

		if (IsRoutingElement (elem.header.type.classID)) {
			ACAPI::Result<RoutingElement> routingElement = RoutingElement::Get (Adapter::UniqueID (neig.guid));
			if (routingElement.IsErr ())
				return routingElement.UnwrapErr ().kind;
			
			if (routingElement->GetDomain () != Domain::CableCarrier)
				return NoError;

			ACAPI::Result<RoutingSegment> routingSegment = RoutingSegment::Get (routingElement->GetRoutingSegmentIds ().front ());
			if (routingSegment.IsErr ())
				return routingSegment.UnwrapErr ().kind;

			RigidSegmentDefault rigidSegmentDefault = routingSegment->GetRigidSegmentDefaultParameters ();

			reporter.Add ("Current Library Part Name of Selected Cable Carrier Route's First Rigid Segment", GetLibPartNameFromGuid (rigidSegmentDefault.GetObjectId ()));

			ACAPI::Result<void> rigidSegmentModifyResult = rigidSegmentDefault.Modify ([&] (ElementDefault::Modifier& modifier) {
				modifier.SetObjectId (GSGuid2APIGuid (unId.GetMainGuid ()));
			});

			if (rigidSegmentModifyResult.IsErr ())
				return rigidSegmentModifyResult.UnwrapErr ().kind;

			ACAPI::Result<void> routingSegmentModifyResult = routingSegment->Modify ([&] (ACAPI::MEP::RoutingSegment::Modifier& modifier) {
				modifier.SetRigidSegmentDefaultParameters (rigidSegmentDefault);
			}, "Set back the modified Rigid Segment Default");

			if (routingSegmentModifyResult.IsErr ())
				return routingSegmentModifyResult.UnwrapErr ().kind;

			reporter.Add ("New Library Part Name of Selected Cable Carrier Route's First Rigid Segment", newLibpartName);
			reporter.Write ();
		}
	}

	return NoError;
}


GSErrCode ModifyLibraryPartOfSelectedCableCarrierRoutesDefaultRigidSegment ()
{
	Reporter reporter;

	GS::UniString newLibpartName = "Cable Tray 27.gsm";
	GS::UnID unId;
	ERRCHK_NO_ASSERT (GetLibPartUnIdFromName (newLibpartName, unId));

	GS::Array<API_Neig>	selNeigs;
	API_SelectionInfo 	selectionInfo = {};

	ERRCHK_NO_ASSERT (ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false));

	for (const API_Neig& neig : selNeigs) {
		API_Element	elem = {};
		elem.header.guid = neig.guid;
		ERRCHK_NO_ASSERT (ACAPI_Element_Get (&elem));

		if (IsRoutingElement (elem.header.type.classID)) {
			ACAPI::Result<RoutingElement> routingElement = RoutingElement::Get (Adapter::UniqueID (neig.guid));
			if (routingElement.IsErr ())
				return routingElement.UnwrapErr ().kind;

			if (routingElement->GetDomain () != Domain::CableCarrier)
				return NoError;

			RoutingSegmentDefault routingSegmentDefault = routingElement->GetRoutingSegmentDefaultParameters ();
			RigidSegmentDefault rigidSegmentDefault = routingSegmentDefault.GetRigidSegmentDefault ();

			reporter.Add ("Current Library Part Name of Selected Cable Carrier Route's Default Rigid Segment", GetLibPartNameFromGuid (rigidSegmentDefault.GetObjectId ()));

			ACAPI::Result<void> rigidSegmentModifyResult = rigidSegmentDefault.Modify ([&] (ElementDefault::Modifier& modifier) {
				modifier.SetObjectId (GSGuid2APIGuid (unId.GetMainGuid ()));
			});

			if (rigidSegmentModifyResult.IsErr ())
				return rigidSegmentModifyResult.UnwrapErr ().kind;

			ACAPI::Result<void> routingSegmentModifyResult = routingSegmentDefault.Modify ([&] (ACAPI::MEP::RoutingSegmentDefault::Modifier& modifier) {
				modifier.SetRigidSegmentDefault (rigidSegmentDefault);
			});

			if (routingSegmentModifyResult.IsErr ())
				return routingSegmentModifyResult.UnwrapErr ().kind;

			ACAPI::Result<void> routingElementModifyResult = routingElement->Modify ([&] (ACAPI::MEP::RoutingElement::Modifier& modifier) {
				modifier.SetRoutingSegmentDefaultParameters (routingSegmentDefault);
			}, "Set back the modified Routing Segment Default");

			if (routingElementModifyResult.IsErr ())
				return routingElementModifyResult.UnwrapErr ().kind;

			reporter.Add ("New Library Part Name of Selected Cable Carrier Route's Default Rigid Segment", newLibpartName);
			reporter.Write ();
		}
	}

	return NoError;
}


GSErrCode ModifyLibraryPartOfDefaultCableCarrierRoutesRigidSegment ()
{
	Reporter reporter;

	GS::UniString newLibpartName = "Cable Ladder 27.gsm";
	GS::UnID unId;
	ERRCHK_NO_ASSERT (GetLibPartUnIdFromName (newLibpartName, unId));

	ACAPI::Result<ACAPI::MEP::RoutingElementDefault> routingElementDefault = ACAPI::MEP::CreateRoutingElementDefault (ACAPI::MEP::Domain::CableCarrier);
	if (routingElementDefault.IsErr ())
		return routingElementDefault.UnwrapErr ().kind;

	RoutingSegmentDefault routingSegmentDefault = routingElementDefault->GetRoutingSegmentDefault ();
	RigidSegmentDefault rigidSegmentDefault = routingSegmentDefault.GetRigidSegmentDefault ();

	reporter.Add ("Current Library Part Name of Default Cable Carrier Route's Rigid Segment", GetLibPartNameFromGuid (rigidSegmentDefault.GetObjectId ()));

	ACAPI::Result<void> rigidSegmentModifyResult = rigidSegmentDefault.Modify ([&] (ElementDefault::Modifier& modifier) {
		modifier.SetObjectId (GSGuid2APIGuid (unId.GetMainGuid ()));
	});

	if (rigidSegmentModifyResult.IsErr ())
		return rigidSegmentModifyResult.UnwrapErr ().kind;

	ACAPI::Result<void> routingSegmentModifyResult = routingSegmentDefault.Modify ([&] (RoutingSegmentDefault::Modifier& modifier) {
		modifier.SetRigidSegmentDefault (rigidSegmentDefault);
	});

	if (routingSegmentModifyResult.IsErr ())
		return routingSegmentModifyResult.UnwrapErr ().kind;

	ACAPI::Result<void> routingElementModifyResult = routingElementDefault->Modify ([&] (RoutingElementDefault::Modifier& modifier) {
		modifier.SetRoutingSegmentDefault (routingSegmentDefault);
	});

	if (routingElementModifyResult.IsErr ())
		return routingElementModifyResult.UnwrapErr ().kind;

	reporter.Add ("New Library Part Name of Default Cable Carrier Route's Rigid Segment", newLibpartName);
	reporter.Write ();

	ACAPI::Result<void> setAsDefaultResult = routingElementDefault->SetAsArchicadDefault ();
	if (setAsDefaultResult.IsErr ())
		return setAsDefaultResult.UnwrapErr ().kind;

	return NoError;
}


} // namespace MEPExample