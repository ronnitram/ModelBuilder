#ifndef	IFCPRESETCOMMANDS_HPP
#define	IFCPRESETCOMMANDS_HPP

#pragma once

#include "UniString.hpp"

// Commands for preset manipulation
#define CREATEPRESET						'CRPR'
#define HASPRESET							'HAPR'
#define CLEARPRESET							'CLPR'
#define ADDNEWTYPEMAPPINGFORIMPORT			'AITM'
#define ADDNEWTYPEMAPPINGFOREXPORT			'AETM'
#define ADDNEWPROPERTYMAPPINGFOREXPORT		'AEPM'
#define ADDNEWPROPERTYMAPPINGFORIMPORT		'AIPM'


static const API_ModulID IfcMdid = {1198731108, 138575850};

void CreatePreset ();
void HasPreset ();
void ClearPreset ();
void AddNewTypeMappingForImport ();
void AddNewTypeMappingForExport (const GS::UniString& ifcSchema = "");
void AddNewPropertyMappingForExport ();
void AddNewPropertyMappingForImport ();

#endif
