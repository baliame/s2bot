#pragma once
#include <sc2api/sc2_api.h>
#include <queue>

#define XRANGE(a) (int x = 0; x < a; x++)
#define YRANGE(a) (int y = 0; y < a; y++)

struct Zone {
	int zone_id;
	unsigned char zone_height;
	bool is_home_zone;
	bool is_ramp_zone;
	Zone** adjacent_zones;
};

struct PartitionPoint {
	int x;
	int y;
	Zone* source;
	PartitionPoint(int x, int y, Zone* source) : x(x), y(y), source(source) {}
};

struct PartitionData {
	int width, height;
	unsigned char** closed;
	std::queue<PartitionPoint> open;
	
	PartitionData(int width, int height) : width(width), height(height) {
		closed = new unsigned char*[width];
		for XRANGE(width) {
			closed[x] = new unsigned char[height];
		}
	}
};

void FloodPartitionZones(sc2::ObservationInterface* obs);