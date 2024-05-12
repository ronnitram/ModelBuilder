#if !defined (TESTCOMMANDS_HPP)
#define TESTCOMMANDS_HPP

#pragma once

#include "APIEnvir.h"
#include "ACAPinc.h"


class CreateColumnCommand : public API_AddOnCommand {
public:

	virtual GS::String							GetName () const override;
	virtual GS::String							GetNamespace () const override;
	virtual GS::Optional<GS::UniString>			GetSchemaDefinitions () const override;
	virtual GS::Optional<GS::UniString>			GetInputParametersSchema () const override;
	virtual GS::Optional<GS::UniString>			GetResponseSchema () const override;
	
	virtual API_AddOnCommandExecutionPolicy		GetExecutionPolicy () const override			{ return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread; }
	virtual bool								IsProcessWindowVisible () const override				{ return true; }

	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;

	virtual void								OnResponseValidationFailed (const GS::ObjectState& response) const override;
};


class ProtectionInfoCommand : public API_AddOnCommand {
public:

	virtual GS::String							GetName () const override;
	virtual GS::String							GetNamespace () const override;
	virtual GS::Optional<GS::UniString>			GetSchemaDefinitions () const override;
	virtual GS::Optional<GS::UniString>			GetInputParametersSchema () const override;
	virtual GS::Optional<GS::UniString>			GetResponseSchema () const override;
	
	virtual API_AddOnCommandExecutionPolicy		GetExecutionPolicy () const override			{ return API_AddOnCommandExecutionPolicy::InstantExecutionOnParallelThread; }
	virtual bool								IsProcessWindowVisible () const override				{ return false; }

	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;

	virtual void								OnResponseValidationFailed (const GS::ObjectState& response) const override;
};


class FailedResponseValidationCommand : public API_AddOnCommand {
public:

	virtual GS::String							GetName () const override;
	virtual GS::String							GetNamespace () const override;
	virtual GS::Optional<GS::UniString>			GetSchemaDefinitions () const override;
	virtual GS::Optional<GS::UniString>			GetInputParametersSchema () const override;
	virtual GS::Optional<GS::UniString>			GetResponseSchema () const override;
	
	virtual API_AddOnCommandExecutionPolicy		GetExecutionPolicy () const override			{ return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread; }

	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;

	virtual void								OnResponseValidationFailed (const GS::ObjectState& response) const override;
};

#endif