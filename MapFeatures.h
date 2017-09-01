#pragma once
#include <sc2api/sc2_api.h>
#include <queue>
#include <vector>
#include <iostream>

#define XRANGE(a) (int x = 0; x < a; x++)
#define YRANGE(a) (int y = 0; y < a; y++)

struct Zone {
	Zone();

	int zone_id;
	unsigned char zone_height;
	bool is_home_zone;
	bool is_ramp_zone;
	unsigned tiles;
	std::vector<Zone*> adjacent_zones;
	static int next_zone_id;
};

struct ZoneStorage {
	ZoneStorage(int width, int height) : points(new Zone*[width*height]), w(width), h(height) {}
	~ZoneStorage() {
		if (points) {
			delete[] points;
			points = nullptr;
		}
	}

	Zone* operator()(int x, int y) {
		if (x >= w || y >= h || x < 0 || y < 0) {
			std::cerr << "Map: " << w << "x" << h << "; requested coords: (" << x << ", " << y << ")" << std::endl;
			assert(false);  /* CRASH() */
		}
		return points[h * y + x];
	}

	void set(int x, int y, Zone* z) {
		points[h * y + x] = z;
	}

private:
	Zone** points;
	int w, h;
};

extern std::vector<Zone*> all_zones;
extern ZoneStorage* zone_storage;
#define qabs(x) (x < 0 ? -x : x)

struct PartitionPoint {
	int x;
	int y;
	Zone* source;
	PartitionPoint(int x, int y, Zone* source) : x(x), y(y), source(source) {}
	int manhattan(sc2::Point2DI target) {
		return qabs(x - target.x) + qabs(y - target.y);
	}
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

	~PartitionData() {
		if (closed) {
			for XRANGE(width) {
				delete[] closed[x];
			}
			delete[] closed;
			closed = nullptr;
		}
	}
};

void FloodPartitionZones(sc2::ObservationInterface* obs);