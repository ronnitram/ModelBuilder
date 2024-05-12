#include "Preferences.hpp"
#include "Reporter.hpp"

// ACAPI
#include "ACAPI/Result.hpp"

// MEPAPI
#include "ACAPI/MEPUniqueID.hpp"

#include "ACAPI/MEPPreferenceTableContainerBase.hpp"
#include "ACAPI/MEPDuctPreferenceTableContainer.hpp"
#include "ACAPI/MEPPipePreferenceTableContainer.hpp"
#include "ACAPI/MEPCableCarrierPreferenceTableContainer.hpp"
#include "ACAPI/MEPPreferenceTableBase.hpp"


using namespace ACAPI::MEP;


namespace MEPExample {


GSErrCode QueryMEPPreferences ()
{
// ! [PreferenceTableContainer-Getters-Example]
	Reporter preferenceReporter;

	auto enumerateTableContent = [&](const PreferenceTableBase& table, PreferenceTableShape shape) {
		preferenceReporter.Add ("Preference name", table.GetName ());
		preferenceReporter.Add ("Shape of the table", shape);

		for (uint32_t i = 0; i < *table.GetSize (shape); ++i) {
			preferenceReporter.SetTabCount (1);
			preferenceReporter.Add ("Value", *table.GetValue (shape, i), false);
			preferenceReporter.SetTabCount (0);
			preferenceReporter.Add (", ", false);
			preferenceReporter.Add ("Description", *table.GetDescription (shape, i));
		}

		preferenceReporter.AddNewLine ();
	};

	// Duct preferences
	{
		ACAPI::Result<DuctPreferenceTableContainer> container = GetDuctPreferenceTableContainer ();

		if (container.IsErr ())
			return container.UnwrapErr ().kind;

		std::vector<UniqueID> tableIds = container->GetPreferenceTables ();

		preferenceReporter.Add ("Duct preferences");
		for (UniqueID uniqueId : tableIds) {
			ACAPI::Result<PreferenceTableBase> table = PreferenceTableBase::Get (uniqueId);

			if (table.IsErr ())
				continue;

			enumerateTableContent (*table, PreferenceTableShape::Rectangular);
			enumerateTableContent (*table, PreferenceTableShape::Circular);
		}

		preferenceReporter.Write ();
	}

	// Pipe preferences
	{
		ACAPI::Result<PipePreferenceTableContainer> container = GetPipePreferenceTableContainer ();

		if (container.IsErr ())
			return container.UnwrapErr ().kind;

		std::vector<UniqueID> tableIds = container->GetPreferenceTables ();

		preferenceReporter.Add ("Pipe preferences");
		for (UniqueID uniqueId : tableIds) {
			ACAPI::Result<PreferenceTableBase> table = PreferenceTableBase::Get (uniqueId);

			if (table.IsErr ())
				continue;

			enumerateTableContent (*table, PreferenceTableShape::Circular);
		}

		preferenceReporter.Write ();
	}

	// Cable carrier preferences
	{
		ACAPI::Result<CableCarrierPreferenceTableContainer> container = GetCableCarrierPreferenceTableContainer ();

		if (container.IsErr ())
			return container.UnwrapErr ().kind;

		std::vector<UniqueID> tableIds = container->GetPreferenceTables ();

		preferenceReporter.Add ("CableCarrier preferences");
		for (UniqueID uniqueId : tableIds) {
			ACAPI::Result<PreferenceTableBase> table = PreferenceTableBase::Get (uniqueId);

			if (table.IsErr ())
				continue;

			enumerateTableContent (*table, PreferenceTableShape::Rectangular);
		}

		preferenceReporter.Write ();
	}
// ! [PreferenceTableContainer-Getters-Example]

	return NoError;
}


GSErrCode ModifyMEPPreferences ()
{
// ! [PreferenceTableContainer-Modification-Example]
	Reporter preferenceReporter;

	ACAPI::Result<DuctPreferenceTableContainer> container = GetDuctPreferenceTableContainer ();

	if (container.IsErr ())
		return container.UnwrapErr ().kind;

	preferenceReporter.Add ("Adding a new table to the duct container...");
	ACAPI::Result<void> modifyResult = container->Modify ([&](PreferenceTableContainerBase::Modifier& modifier) -> GSErrCode {
		ACAPI::Result<UniqueID> tableId = modifier.AddNewTable ("Duct sizes - added from MEP_Test");

		if (tableId.IsErr ())
			return tableId.UnwrapErr ().kind;

		return NoError;
	}, "Modify the content of duct container.");

	if (modifyResult.IsErr ())
		return modifyResult.UnwrapErr ().kind;

	preferenceReporter.Add ("New table successfully added.");
	preferenceReporter.AddNewLine ();

	std::vector<UniqueID> tableIds = container->GetPreferenceTables ();

	preferenceReporter.Add ("Deleting the first row of every shape of every duct table.");
	for (UniqueID uniqueId : tableIds) {
		ACAPI::Result<PreferenceTableBase> table = PreferenceTableBase::Get (uniqueId);

		if (table.IsErr ())
			continue;

		preferenceReporter.Add ("Trying to delete from", table->GetName ());
		table->Modify ([&](PreferenceTableBase::Modifier& modifier) {
			preferenceReporter.SetTabCount (1);
			ACAPI::Result<void> rDeleteResult = modifier.Delete (PreferenceTableShape::Rectangular, 0);

			if (rDeleteResult.IsOk ())
				preferenceReporter.Add ("Deletion of the first element from rectangular sizes was successful.");
			else
				preferenceReporter.Add ("Deletion unsuccessful", GS::UniString (rDeleteResult.UnwrapErr ().text));

			ACAPI::Result<void> cDeleteResult = modifier.Delete (PreferenceTableShape::Circular, 0);

			if (cDeleteResult.IsOk ())
				preferenceReporter.Add ("Deletion of the first element from circular sizes was successful.");
			else
				preferenceReporter.Add ("Deletion unsuccessful", GS::UniString (cDeleteResult.UnwrapErr ().text));
		}, "Delete from preference table.");

		preferenceReporter.SetTabCount (0);
	}

	preferenceReporter.Write ();
// ! [PreferenceTableContainer-Modification-Example]

	return NoError;
}


} // namespace MEPExample