#pragma once
#include <sc2api/sc2_api.h>

enum BuildDirectiveType {
	BDT_BuildStructure,
	BDT_TrainUnit,
	BDT_ResearchUpgrade,
};

enum class BuildStructureDirectiveTarget : char {
	BSDT_HomeZone,
	BSDT_SameZoneAsId,
	BSDT_RampBlock,
	BSDT_Proxy,
	BSDT_NearestUnoccupiedExpansion,
	BSDT_ExpansionSpotNearPoint,
	BSDT_Any, // Placeholder, same as HomeZone
};

enum CompletionStatus {
	CS_NotStarted,
	CS_Started,
	CS_Finished,
};

enum class TrainedUnitSquadAssignment : char {
	TUSA_NewDefenseSquad,
	TUSA_NewHarrassmentSquad,
	TUSA_MainArmySquad,
	TUSA_SameSquadAsId,
	TUSA_OwnCommandCenterMiningSquad,
	TUSA_MiningSquadNearPoint,
};

union EnumParameter {
	BuildStructureDirectiveTarget structure_parameter;
	TrainedUnitSquadAssignment unit_parameter;

	EnumParameter() : structure_parameter(BuildStructureDirectiveTarget::BSDT_Any) {};
	EnumParameter(BuildStructureDirectiveTarget spm) : structure_parameter(spm) {};
	EnumParameter(TrainedUnitSquadAssignment upm) : unit_parameter(upm) {};
};

enum DirectiveRepeatability {
	DR_NoRepeat,
	DR_RepeatUntilNumAssignedToZone,
	
};

struct BuildDirective {
	BuildDirectiveType type;
	sc2::ABILITY_ID build_ability;
	int wait_finish_id;
	EnumParameter enum_parameter;
	int id_parameter;
	sc2::Point2D point_parameter;
	DirectiveRepeatability repeat;
	bool async_repeat;

	BuildDirective(BuildDirectiveType type, sc2::ABILITY_ID build_ability, EnumParameter enum_parameter = EnumParameter(),
		int id_parameter = -1, const sc2::Point2D& point_parameter = sc2::Point2D(0, 0), int wait_finish_id = -1) :
		type(type), build_ability(build_ability), enum_parameter(enum_parameter), id_parameter(id_parameter), point_parameter(point_parameter), wait_finish_id(wait_finish_id) {};

	BuildDirective(sc2::ABILITY_ID build_ability, BuildStructureDirectiveTarget spm, int id_parameter = -1, const sc2::Point2D& point_parameter = sc2::Point2D(0, 0), int wait_finish_id = -1) :
		type(BuildDirectiveType::BDT_BuildStructure), build_ability(build_ability), enum_parameter(EnumParameter(spm)), id_parameter(id_parameter), point_parameter(point_parameter), wait_finish_id(wait_finish_id), repeat(DR_NoRepeat), {};

	BuildDirective(sc2::ABILITY_ID build_ability, TrainedUnitSquadAssignment upm, int id_parameter = -1, const sc2::Point2D& point_parameter = sc2::Point2D(0, 0), int wait_finish_id = -1,
		DirectiveRepeatability repeat = DR_NoRepeat, bool async_repeat = false) :
		type(BuildDirectiveType::BDT_TrainUnit), build_ability(build_ability), enum_parameter(EnumParameter(upm)), id_parameter(id_parameter), point_parameter(point_parameter), wait_finish_id(wait_finish_id), repeat(repeat), async_repeat(async_repeat) {};
};

class BuildOrder {
public:
	BuildOrder() : directive_count(directive_alloc), directive_status(0), build_depots_if_required(true) {
		if (directive_alloc > 0) {
			directives = new const BuildDirective*[directive_alloc];
			status = new CompletionStatus[directive_alloc];
		}
		CreateBuildDirectives();
	}

private:
	const BuildDirective** directives;
	CompletionStatus* status;
	int directive_count;
	int directive_status;
	int internal_push_counter = 0;
protected:
	bool build_depots_if_required;
	const int directive_alloc = 0;
	const bool may_skip_waiting = false;
	void PushDirective(const BuildDirective* d);
	virtual void CreateBuildDirectives() = 0;
};

extern const BuildDirective* SCVBuildDirective;

class CommonBaseBuildOrder : public BuildOrder {
protected:
	const int directive_alloc = 8;
	virtual void CreateBuildDirectives();
};

enum BuildOrderError {
	BOE_Unknown,
	BOE_NoAppropriateStructureFound,  /* Thrown if train directive fails because no structure is capable of training the unit. */
	BOE_PrerequisiteNotMet,           /* Thrown if build directive fails due to a missing prerequisite. */
};