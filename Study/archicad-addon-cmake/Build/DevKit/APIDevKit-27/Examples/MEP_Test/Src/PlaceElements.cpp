#include "PlaceElements.hpp"
#include "Reporter.hpp"

// API
#include "ACAPinc.h"

// ACAPI
#include "ACAPI/Result.hpp"

// MEPAPI
#include "ACAPI/MEPAdapter.hpp"

#include "ACAPI/MEPPort.hpp"

#include "ACAPI/MEPRoutingElement.hpp"
#include "ACAPI/MEPRoutingElementDefault.hpp"

#include "ACAPI/MEPTerminal.hpp"
#include "ACAPI/MEPTerminalDefault.hpp"
#include "ACAPI/MEPFlexibleSegment.hpp"
#include "ACAPI/MEPFlexibleSegmentDefault.hpp"
#include "ACAPI/MEPFittingDefault.hpp"
#include "ACAPI/MEPAccessoryDefault.hpp"
#include "ACAPI/MEPEquipmentDefault.hpp"


using namespace ACAPI::MEP;


namespace MEPExample {


namespace {


GSErrCode PlaceTerminals ()
{
// ! [Terminal-Placement-Example]
	{
		auto ductTerminalDefault = CreateTerminalDefault (Domain::Ventilation);
		if (ductTerminalDefault.IsErr ())
			return ductTerminalDefault.UnwrapErr ().kind;

		ACAPI::Result<UniqueID> terminalId = ductTerminalDefault->Place ({ 0.0, 10.0, 0.0 }, { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } }, GSGuid2APIGuid (GS::Guid::GenerateGuid));
		if (terminalId.IsErr ())
			return terminalId.UnwrapErr ().kind;

		ACAPI_WriteReport ("Ventilation Terminal placed successfully", false);
	}

	{
		auto pipeTerminalDefault = CreateTerminalDefault (Domain::Piping);
		if (pipeTerminalDefault.IsErr ())
			return pipeTerminalDefault.UnwrapErr ().kind;

		ACAPI::Result<UniqueID> terminalId = pipeTerminalDefault->Place ({ 10.0, 10.0, 0.0 }, { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } }, GSGuid2APIGuid (GS::Guid::GenerateGuid));
		if (terminalId.IsErr ())
			return terminalId.UnwrapErr ().kind;

		ACAPI_WriteReport ("Piping Terminal placed successfully", false);
	}
// ! [Terminal-Placement-Example]

	return NoError;
}


GSErrCode PlaceAccessories ()
{
// ! [Accessory-Placement-Example]
	{
		auto ductAccessoryDefault = CreateAccessoryDefault (Domain::Ventilation);
		if (ductAccessoryDefault.IsErr ())
			return ductAccessoryDefault.UnwrapErr ().kind;

		Orientation orientation = { { -1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } };
		ACAPI::Result<UniqueID> accessoryId = ductAccessoryDefault->Place ({ 0.0, 13.0, 0.0 }, orientation, GSGuid2APIGuid (GS::Guid::GenerateGuid));
		if (accessoryId.IsErr ())
			return accessoryId.UnwrapErr ().kind;

		ACAPI_WriteReport ("Ventilation Accessory placed successfully", false);
	}

	{
		auto pipeAccessoryDefault = CreateAccessoryDefault (Domain::Piping);
		if (pipeAccessoryDefault.IsErr ())
			return pipeAccessoryDefault.UnwrapErr ().kind;

		Orientation orientation = { { -1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } };
		ACAPI::Result<UniqueID> accessoryId = pipeAccessoryDefault->Place ({ 10.0, 13.0, 0.0 }, orientation, GSGuid2APIGuid (GS::Guid::GenerateGuid));
		if (accessoryId.IsErr ())
			return accessoryId.UnwrapErr ().kind;

		ACAPI_WriteReport ("Piping Accessory placed successfully", false);
	}
// ! [Accessory-Placement-Example]

	return NoError;
}


GSErrCode PlaceEquipment ()
{
// ! [Equipment-Placement-Example]
	auto equipmentDefault = CreateEquipmentDefault ();
	if (equipmentDefault.IsErr ())
		return equipmentDefault.UnwrapErr ().kind;

	Orientation orientation = { { -1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } };

	ACAPI::Result<UniqueID> equipmentId = equipmentDefault->Place ({ 20.0, 11.5, 0.0 }, orientation, GSGuid2APIGuid (GS::Guid::GenerateGuid));
	if (equipmentId.IsErr ())
		return equipmentId.UnwrapErr ().kind;

	ACAPI_WriteReport ("Equipment placed successfully", false);
// ! [Equipment-Placement-Example]

	return NoError;
}


GSErrCode PlaceFittings ()
{
// ! [Fitting-Placement-Example]
	{
		auto ductFittingDefault = CreateFittingDefault (Domain::Ventilation);
		if (ductFittingDefault.IsErr ())
			return ductFittingDefault.UnwrapErr ().kind;

		Orientation orientation = { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } };

		ACAPI::Result<UniqueID> fitting = ductFittingDefault->Place ({ 0.0, 20.0, 0.0 }, orientation, GSGuid2APIGuid (GS::Guid::GenerateGuid));
		if (fitting.IsErr ())
			return fitting.UnwrapErr ().kind;

		ACAPI_WriteReport ("Ventilation Fitting placed successfully", false);
	}

	{
		auto pipeFittingDefault = CreateFittingDefault (Domain::Piping);
		if (pipeFittingDefault.IsErr ())
			return pipeFittingDefault.UnwrapErr ().kind;

		Orientation orientation = { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } };

		ACAPI::Result<UniqueID> fitting = pipeFittingDefault->Place ({ 10.0, 20.0, 0.0 }, orientation, GSGuid2APIGuid (GS::Guid::GenerateGuid));
		if (fitting.IsErr ())
			return fitting.UnwrapErr ().kind;

		ACAPI_WriteReport ("Piping Fitting placed successfully", false);
	}

	{
		auto cableCarrierFittingDefault = CreateFittingDefault (Domain::CableCarrier);
		if (cableCarrierFittingDefault.IsErr ())
			return cableCarrierFittingDefault.UnwrapErr ().kind;

		Orientation orientation = { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } };

		ACAPI::Result<UniqueID> fitting = cableCarrierFittingDefault->Place ({ 20.0, 20.0, 0.0 }, orientation, GSGuid2APIGuid (GS::Guid::GenerateGuid));
		if (fitting.IsErr ())
			return fitting.UnwrapErr ().kind;

		ACAPI_WriteReport ("Cable Carrier Fitting placed successfully", false);
	}
// ! [Fitting-Placement-Example]

	return NoError;
}


GSErrCode PlaceFlexibleSegment ()
{
// ! [FlexibleSegment-Placement-Example]
	auto ductFlexibleSegmentDefault = CreateFlexibleSegmentDefault ();
	if (ductFlexibleSegmentDefault.IsErr ())
		return ductFlexibleSegmentDefault.UnwrapErr ().kind;

	std::vector<API_Coord3D> controlPoints = {
		{ 0.0, 30.0, 0.0 },
		{ 2.0, 31.0, 0.0 },
		{ 4.0, 29.0, 0.0 },
		{ 6.0, 30.0, 0.0 }
	};

	Orientation startOrientation = { { -1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } };
	Orientation endOrientation = { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } };

	ACAPI::Result<UniqueID> flexibleSegment = ductFlexibleSegmentDefault->Place (controlPoints, startOrientation, endOrientation, GSGuid2APIGuid (GS::Guid::GenerateGuid));
	if (flexibleSegment.IsErr ())
		return flexibleSegment.UnwrapErr ().kind;

	ACAPI_WriteReport ("Ventilation FlexibleSegment placed successfully", false);
// ! [FlexibleSegment-Placement-Example]

	return NoError;
}


ACAPI::Result<std::vector<UniqueID>> PlaceRoutes (const std::vector<std::pair<std::vector<API_Coord3D>, std::vector<RoutingSegmentCrossSectionData>>>& placementData, Domain domain)
{
// ! [RoutingElement-Placement-Example]
	std::vector<UniqueID> routeIds;
	routeIds.reserve (placementData.size ());

	ACAPI::Result<RoutingElementDefault> routingElementDefault = CreateRoutingElementDefault (domain);
	if (routingElementDefault.IsErr ())
		return { routingElementDefault.UnwrapErr (), ACAPI_GetToken () };

	for (const auto& data : placementData) {
		// Optionally, you could provide a GUID for the placed route with a third parameter
		ACAPI::Result<UniqueID> routingElemId = routingElementDefault->Place (data.first, data.second);
		if (routingElemId.IsErr ())
			return { routingElemId.UnwrapErr (), ACAPI_GetToken () };

		routeIds.emplace_back (*routingElemId);
	}

	return Ok (std::move (routeIds));
// ! [RoutingElement-Placement-Example]
}


// ! [RoutingElement-Connect-Example]
ACAPI::Result<void> ConnectRoutes (const std::vector<UniqueID>& routeIds)
{
	if (routeIds.empty ())
		return { ACAPI::Error (ErrParam, "Route vector is empty."), ACAPI_GetToken () };

	ACAPI::Result<RoutingElement> routingElem = RoutingElement::Get (routeIds[0]);
	if (routingElem.IsErr ())
		return { routingElem.UnwrapErr (), ACAPI_GetToken () };

	for (size_t i = 1; i < routeIds.size (); ++i) {
		ACAPI::Result<void> modifyResult = routingElem->Modify ([&](RoutingElement::Modifier& modifier) -> GSErrCode {
			ACAPI::Result<void> connectionResult = modifier.Connect (routeIds[i]);
			if (connectionResult.IsErr ())
				return connectionResult.UnwrapErr ().kind;

			return NoError;
		}, "Connect routes.");

		if (modifyResult.IsErr ())
			return { modifyResult.UnwrapErr (), ACAPI_GetToken () };
	}

	return ACAPI::Ok ();
}
// ! [RoutingElement-Connect-Example]


GSErrCode PlaceRoutesAndConnect (const std::vector<std::pair<std::vector<API_Coord3D>, std::vector<RoutingSegmentCrossSectionData>>>& placementData, Domain domain)
{
	if (placementData.size () < 2)
		return Error;

	// ! [RoutingElement-PlaceAndConnectWrapped-Example]
	ACAPI::Result<void> commandResult = ACAPI::CallUndoableCommand ([&]() -> GSErrCode {
		ACAPI::Result<std::vector<UniqueID>> routeIds = PlaceRoutes (placementData, domain);

		if (routeIds.IsErr ())
			return routeIds.UnwrapErr ().kind;

		ACAPI::Result<void> connectionResult = ConnectRoutes (*routeIds);

		if (connectionResult.IsErr ())
			return connectionResult.UnwrapErr ().kind;

		return NoError;
	}, ACAPI_GetToken (), "Place and connect routes.");
	// ! [RoutingElement-PlaceAndConnectWrapped-Example]

	if (commandResult.IsErr ())
		return commandResult.UnwrapErr ().kind;

	return NoError;
}


} // namespace


GSErrCode PlaceMEPElements ()
{
	ERRCHK_NO_ASSERT (PlaceTerminals ());

	ERRCHK_NO_ASSERT (PlaceAccessories ());

	ERRCHK_NO_ASSERT (PlaceEquipment ());

	ERRCHK_NO_ASSERT (PlaceFittings ());

	ERRCHK_NO_ASSERT (PlaceFlexibleSegment ());

	return NoError;
}


GSErrCode CopyPlaceSelectedTerminals ()
{
	GS::Array<API_Neig>	selNeigs;
	API_SelectionInfo 	selectionInfo = {};

	ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false);

	for (const API_Neig& neig : selNeigs) {
		API_Element	elem = {};
		elem.header.guid = neig.guid;
		ACAPI_Element_Get (&elem);

		if (elem.header.type.typeID == API_ExternalElemID && IsTerminal (elem.header.type.classID)) {
			ACAPI::Result<Terminal> terminal = Terminal::Get (Adapter::UniqueID (neig.guid));
			TerminalDefault terminalDefault = terminal->PickUpDefault ();
			ACAPI::Result<UniqueID> newTerminalId = terminalDefault.Place ({ 10.0, 10.0, 0.0 }, { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } });

			if (newTerminalId.IsErr ())
				return newTerminalId.UnwrapErr ().kind;

			Reporter reporter;
			reporter.Add ("New terminal placed successfully!");
			reporter.Add ("ID of the new terminal:", *newTerminalId);
			reporter.Write ();
		}
	}

	return NoError;
}


GSErrCode PlaceTwoContinuousRoutes ()
{
// ! [RoutingElement-PlacementData-Example]
	std::vector<API_Coord3D> referenceCoords1;
	referenceCoords1.push_back ({ 0.0, 0.0, 2.0 });
	referenceCoords1.push_back ({ 5.0, 0.0, 2.0 });
	referenceCoords1.push_back ({ 5.0, 5.0, 2.0 });
	referenceCoords1.push_back ({ 10.0, 5.0, 2.0 });
	referenceCoords1.push_back ({ 10.0, 10.0, 2.0 });

	std::vector<RoutingSegmentCrossSectionData> crossSectionData1;
	crossSectionData1.push_back ({ 0.0, 0.025, ConnectorShape::Circular });
	crossSectionData1.push_back ({ 0.0, 0.025, ConnectorShape::Circular });
	crossSectionData1.push_back ({ 0.0, 0.025, ConnectorShape::Circular });
	crossSectionData1.push_back ({ 0.0, 0.025, ConnectorShape::Circular });

	std::vector<API_Coord3D> referenceCoords2;
	referenceCoords2.push_back ({ 10.0, 10.0, 2.0 });
	referenceCoords2.push_back ({ 15.0, 10.0, 2.0 });
	referenceCoords2.push_back ({ 15.0, 15.0, 2.0 });

	std::vector<RoutingSegmentCrossSectionData> crossSectionData2;
	crossSectionData2.push_back ({ 0.0, 0.02, ConnectorShape::Circular });
	crossSectionData2.push_back ({ 0.0, 0.02, ConnectorShape::Circular });

	return PlaceRoutesAndConnect ({ { referenceCoords1, crossSectionData1 }, { referenceCoords2, crossSectionData2 } }, Domain::Piping);
// ! [RoutingElement-PlacementData-Example]
}


GSErrCode PlaceTwoRoutesInTShape ()
{
	std::vector<API_Coord3D> referenceCoords1;
	referenceCoords1.push_back ({ 0.0, 0.0, 2.0 });
	referenceCoords1.push_back ({ 5.0, 0.0, 2.0 });
	referenceCoords1.push_back ({ 10.0, 0.0, 2.0 });
	referenceCoords1.push_back ({ 15.0, 5.0, 2.0 });

	std::vector<RoutingSegmentCrossSectionData> crossSectionData1;
	crossSectionData1.push_back ({ 0.8, 0.5, ConnectorShape::Rectangular });
	crossSectionData1.push_back ({ 0.8, 0.5, ConnectorShape::Rectangular });
	crossSectionData1.push_back ({ 0.8, 0.5, ConnectorShape::Rectangular });

	std::vector<API_Coord3D> referenceCoords2;
	referenceCoords2.push_back ({ 2.5, 0.0, 2.0 });
	referenceCoords2.push_back ({ 2.5, -5.0, 2.0 });

	std::vector<RoutingSegmentCrossSectionData> crossSectionData2;
	crossSectionData2.push_back ({ 0.9, 0.7, ConnectorShape::Rectangular });

	return PlaceRoutesAndConnect ({ { referenceCoords1, crossSectionData1 }, { referenceCoords2, crossSectionData2 } }, Domain::Ventilation);
}


GSErrCode PlaceThreeRoutesInTShape ()
{
	std::vector<API_Coord3D> referenceCoords1;
	referenceCoords1.push_back ({ 0.0, 0.0, 2.0 });
	referenceCoords1.push_back ({ 5.0, 0.0, 2.0 });
	referenceCoords1.push_back ({ 10.0, 0.0, 2.0 });
	referenceCoords1.push_back ({ 12.5, 5.0, 2.0 });
	referenceCoords1.push_back ({ 15.0, 5.0, 2.0 });

	std::vector<RoutingSegmentCrossSectionData> crossSectionData1;
	crossSectionData1.push_back ({ 0.8, 0.4, ConnectorShape::Rectangular });
	crossSectionData1.push_back ({ 0.8, 0.4, ConnectorShape::Rectangular });
	crossSectionData1.push_back ({ 0.8, 0.4, ConnectorShape::Rectangular });
	crossSectionData1.push_back ({ 0.8, 0.4, ConnectorShape::Rectangular });

	std::vector<API_Coord3D> referenceCoords2;
	referenceCoords2.push_back ({ 15.0, 10.0, 2.0 });
	referenceCoords2.push_back ({ 15.0, 5.0, 2.0 });

	std::vector<RoutingSegmentCrossSectionData> crossSectionData2;
	crossSectionData2.push_back ({ 0.9, 0.5, ConnectorShape::Rectangular });

	std::vector<API_Coord3D> referenceCoords3;
	referenceCoords3.push_back ({ 15.0, 5.0, 2.0 });
	referenceCoords3.push_back ({ 20.0, 5.0, 2.0 });

	std::vector<RoutingSegmentCrossSectionData> crossSectionData3;
	crossSectionData3.push_back ({ 0.9, 0.5, ConnectorShape::Rectangular });

	return PlaceRoutesAndConnect ({ { referenceCoords1, crossSectionData1 }, { referenceCoords2, crossSectionData2 }, { referenceCoords3, crossSectionData3 } }, Domain::CableCarrier);
}


GSErrCode PlaceConnectedElements ()
{
	ACAPI::Result<void> commandResult = ACAPI::CallUndoableCommand ([&]() -> GSErrCode {
		auto ductTerminalDefault = CreateTerminalDefault (Domain::Ventilation);
		if (ductTerminalDefault.IsErr ())
			return ductTerminalDefault.UnwrapErr ().kind;

		ACAPI::Result<UniqueID> terminalId = ductTerminalDefault->Place ({ -10.0, -10.0, -0.225 }, { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } });
		if (terminalId.IsErr ())
			return terminalId.UnwrapErr ().kind;

		ACAPI::Result<Terminal> terminal = Terminal::Get (*terminalId);
		if (terminal.IsErr ())
			return terminal.UnwrapErr ().kind;

		ACAPI::Result<Port> terminalPort = Port::Get (terminal->GetPortIDs ()[0]);
		if (terminalPort.IsErr ())
			return terminalPort.UnwrapErr ().kind;

		terminalPort->Modify ([&](Port::Modifier& modifier) {
			modifier.SetShape (ConnectorShape::Circular);
			modifier.SetWidth (0.35);
		}, "Set Terminal Port size.");

		auto ductFlexibleSegmentDefault = CreateFlexibleSegmentDefault ();
		if (ductFlexibleSegmentDefault.IsErr ())
			return ductFlexibleSegmentDefault.UnwrapErr ().kind;

		std::vector<API_Coord3D> controlPoints = {
			terminalPort->GetPosition (),
			{ -5.0, -8.0, 0.0 },
			{ 0.0, -12.0, 0.0 },
			{ 5.0, -10.0, 0.0 }
		};

		Orientation startOrientation = { { -1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } };
		Orientation endOrientation = { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } };

		ACAPI::Result<UniqueID> flexibleSegmentId = ductFlexibleSegmentDefault->Place (controlPoints, startOrientation, endOrientation);
		if (flexibleSegmentId.IsErr ())
			return flexibleSegmentId.UnwrapErr ().kind;

	// ! [RoutingElement-PlaceAndConnectRouteToElement-Example]
		ACAPI::Result<FlexibleSegment> flexibleSegment = FlexibleSegment::Get (*flexibleSegmentId);
		if (flexibleSegment.IsErr ())
			return flexibleSegment.UnwrapErr ().kind;

		ACAPI::Result<Port> flexibleSegmentPort = Port::Get (flexibleSegment->GetPortIDs ()[0]);
		if (flexibleSegmentPort.IsErr ())
			return flexibleSegmentPort.UnwrapErr ().kind;

		flexibleSegmentPort->Modify ([&](Port::Modifier& modifier) {
			modifier.SetShape (ConnectorShape::Circular);
			modifier.SetWidth (0.35);

			ACAPI::Result<Port> flexibleSegmentPort2 = Port::Get (flexibleSegment->GetPortIDs ()[1]);
			flexibleSegmentPort2->Modify ([&](Port::Modifier& modifier) {
				modifier.SetShape (ConnectorShape::Circular);
				modifier.SetWidth (0.35);
			}, "Set FlexibleSegment Port2 size.");
		}, "Set FlexibleSegment Port size.");


		std::vector<API_Coord3D> referenceCoords;
		referenceCoords.push_back ({ 5.0, -10.0, 0.0 });
		referenceCoords.push_back ({ 8.0, -10.0, 0.0 });
		referenceCoords.push_back ({ 8.0, -10.0, 10.0 });

		std::vector<RoutingSegmentCrossSectionData> crossSectionData;
		crossSectionData.push_back ({ 0.3, 0.5, ConnectorShape::Rectangular });
		crossSectionData.push_back ({ 0.8, 0.5, ConnectorShape::Rectangular });

		ACAPI::Result<std::vector<UniqueID>> routingElementId = PlaceRoutes ({ { referenceCoords, crossSectionData } }, Domain::Ventilation);
		if (routingElementId.IsErr ())
			return routingElementId.UnwrapErr ().kind;

		ACAPI::Result<RoutingElement> routingElement = RoutingElement::Get ((*routingElementId)[0]);
		if (routingElement.IsErr ())
			return routingElement.UnwrapErr ().kind;

		routingElement->Modify ([&](RoutingElement::Modifier& modifier) {
			modifier.Connect (*flexibleSegmentId);
		}, "Connect RoutingElement to FlexibleSegment.");
	// ! [RoutingElement-PlaceAndConnectRouteToElement-Example]

		return NoError;
	}, ACAPI_GetToken (), "Place and connect MEPElements and RoutingElement.");

	if (commandResult.IsErr ())
		return commandResult.UnwrapErr ().kind;

	return NoError;
}


} // namespace MEPExample