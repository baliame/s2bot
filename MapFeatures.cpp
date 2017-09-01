#include "MapFeatures.h"

using namespace sc2;

int Zone::next_zone_id = 1;
std::vector<Zone*> all_zones;
ZoneStorage* zone_storage;

Zone::Zone() {
	zone_id = Zone::next_zone_id++;
}

unsigned char GetDataFromImage(const std::string& data, int width, int height, int x, int y) {
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return 0;
	}
	assert(data.size() == width * height);
	return data[x + (height - 1 - y) * width];
}

inline unsigned char GetDataFromImage(const ImageData& data, int x, int y) {
	return GetDataFromImage(data.data, data.width, data.height, x, y);
}

Zone* FindAdjacentZoneWithHeight(int x, int y, unsigned char h) {
	for (int dx = -1; dx <= 1; dx++) {
		for (int dy = -1; dy <= 1; dy++) {
			if (!(dx || dy)) {
				continue;
			}
			auto ptz = (*zone_storage)(x + dx, y + dy);
			if (ptz && ptz->zone_height == h) {
				return ptz;
			}
		}
	}
	return nullptr;
}

void FloodPartitionZones(ObservationInterface* obs) {
	auto& gi = obs->GetGameInfo();
	PartitionData data(gi.width, gi.height);
	auto sl = obs->GetStartLocation();
	std::vector<Point2DI> all_start_locations;
	all_start_locations.push_back(Point2DI(sl.x, sl.y));
	for (auto slpt : gi.enemy_start_locations) {
		all_start_locations.push_back(Point2DI(slpt.x, slpt.y));
	}
	zone_storage = new ZoneStorage(gi.width, gi.height);

	Zone* home = new Zone();
	home->zone_height = GetDataFromImage(gi.terrain_height, sl.x, sl.y);
	PartitionPoint p(sl.x, sl.y, home);
	data.open.push(p);


	
	while (!data.open.empty()) {
		PartitionPoint pt = data.open.front();
		data.open.pop();

		if (data.closed[pt.x][pt.y]) {
			continue;
		}

		unsigned char height = GetDataFromImage(gi.terrain_height, pt.x, pt.y);
		unsigned char pathability = GetDataFromImage(gi.pathing_grid, pt.x, pt.y);

		if (pathability > 0) {

			continue;
		}
		Zone* candidate;

		if (height != pt.source->zone_height) {

			candidate = FindAdjacentZoneWithHeight(pt.x, pt.y, height);
			if (candidate) {
				
			}
		}
		else {
			candidate = pt.source;
		}
	}
}