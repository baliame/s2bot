#include "MapFeatures.h"

using namespace sc2;

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

void FloodPartitionZones(ObservationInterface* obs) {
	auto& gi = obs->GetGameInfo();
	PartitionData data(gi.width, gi.height);
	auto sl = obs->GetStartLocation();
	PartitionPoint p(sl.x, sl.y, nullptr);
	data.open.push(p);

	Zone* active;
	while (!data.open.empty()) {
		PartitionPoint pt = data.open.front();
		data.open.pop();

		if (data.closed[pt.x][pt.y]) {
			continue;
		}

		unsigned char height = GetDataFromImage(gi.terrain_height, pt.x, pt.y);
		//unsigned char pathability = GetDataFromImage(gi.pathing_grid, pt.x, pt.y);

		active = pt.source;
		if (!active) {
			active = new Zone();
			active->zone_height = height;
		}
		else if (height != active->zone_height) {
			
		}
	}
}