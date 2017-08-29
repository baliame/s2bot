#include "BuildOrder.h"

using namespace sc2;

const BuildDirective* SCVBuildDirective = new BuildDirective(ABILITY_ID::TRAIN_SCV, TrainedUnitSquadAssignment::TUSA_OwnCommandCenterMiningSquad);

void BuildOrder::PushDirective(const BuildDirective* d) {
	if (internal_push_counter >= directive_count) {
		return;
	}
	directives[internal_push_counter] = d;
	status[internal_push_counter++] = CS_NotStarted;
}

void CommonBaseBuildOrder::CreateBuildDirectives() {
	PushDirective(SCVBuildDirective);
	PushDirective(SCVBuildDirective);
	PushDirective(new BuildDirective(ABILITY_ID::BUILD_SUPPLYDEPOT, BuildStructureDirectiveTarget::BSDT_RampBlock));
	PushDirective(SCVBuildDirective);
	PushDirective(SCVBuildDirective);
	PushDirective(new BuildDirective(ABILITY_ID::BUILD_BARRACKS, BuildStructureDirectiveTarget::BSDT_RampBlock));
	PushDirective(SCVBuildDirective);
	PushDirective(new BuildDirective(ABILITY_ID::BUILD_REFINERY, BuildStructureDirectiveTarget::BSDT_HomeZone));
}