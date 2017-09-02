#pragma once
#include <sc2api/sc2_api.h>
#include <queue>
#include <set>
#include <vector>
#include <random>
#include <iostream>

#define XRANGE(a) (int x = 0; x < a; x++)
#define YRANGE(a) (int y = 0; y < a; y++)

struct Zone {
	Zone();

	int zone_id;
	uint8_t zone_height;
	bool is_home_zone;
	bool is_ramp_zone;
	unsigned tiles;
	std::set<Zone*> adjacent_zones;
	static int next_zone_id;
	std::vector<sc2::Point2DI> reverse_lookup;
	uint8_t zone_color;

	bool merged;
};

struct ZoneStorage {
	ZoneStorage(int width, int height) : points(new Zone*[width*height]), w(width), h(height) {
		memset(points, 0, sizeof(Zone*)*w*h);
	}
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
		return points[h * x + y];
	}

	void set(int x, int y, Zone* z) {
		points[h * x + y] = z;
	}

	void add_zone(Zone* z) {
		all_zones.insert(z);
	}

	void remove_zone(Zone* z) {
		all_zones.erase(all_zones.find(z));
	}

	auto begin() {
		return all_zones.begin();
	}

	auto end() {
		return all_zones.end();
	}

	sc2::ImageData GenerateZonesImage();

private:
	std::set<Zone*> all_zones;
	Zone** points;
	int w, h;
};

extern ZoneStorage* zone_storage;
#define qabs(x) ((x) < 0 ? -(x) : (x))
#define qmax(x, y) ((x) > (y) ? (x) : (y))

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
	uint8_t** closed;
	std::queue<PartitionPoint> open;
	
	PartitionData(int width, int height) : width(width), height(height) {
		closed = new uint8_t*[width];
		for XRANGE(width) {
			closed[x] = new uint8_t[height];
			memset(closed[x], 0, height * sizeof(uint8_t));
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

void FloodPartitionZones(const sc2::ObservationInterface* obs);

struct Point2DIL : sc2::Point2DI {
	Point2DIL* next;

	Point2DIL(int x, int y, Point2DIL* next) : Point2DI(x, y), next(next) {}
	Point2DIL* Flip(Point2DIL* prev = nullptr);
	void Collapse();
};

Point2DIL* AStar(const sc2::Point2DI& from, const sc2::Point2DI& to, sc2::ObservationInterface* obs);