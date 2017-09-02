#include "MapFeatures.h"

using namespace sc2;

int Zone::next_zone_id = 1;
ZoneStorage* zone_storage;
std::uniform_int_distribution<unsigned short> rnd(16, 255);
std::mt19937 rng;

#define XDELTA(x) (int dx = -x; dx <= x; dx++)
#define YDELTA(x) (int dy = -x; dy <= x; dy++)
#define SETCONTAINS(s, x) (s.find(x) != s.end())

Zone::Zone() {
	zone_id = Zone::next_zone_id++;
	if (zone_storage) {
		zone_storage->add_zone(this);
	}
	zone_color = rnd(rng);

	merged = true;
}

uint8_t GetDataFromImage(const std::string& data, int width, int height, int x, int y) {
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return 0;
	}
	assert(data.size() == width * height);
	return data[x + (height - 1 - y) * width];
}

inline uint8_t GetDataFromImage(const ImageData& data, int x, int y) {
	return GetDataFromImage(data.data, data.width, data.height, x, y);
}

Zone* FindAdjacentZoneWithHeight(int x, int y, uint8_t h) {
	for XDELTA(1) {
		for YDELTA(1) {
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

void FloodPartitionZones(const ObservationInterface* obs) {
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
	data.closed[p.x][p.y] = 1;

	
	while (!data.open.empty()) {
		PartitionPoint pt = data.open.front();
		data.open.pop();

		uint8_t height = GetDataFromImage(gi.terrain_height, pt.x, pt.y);
		uint8_t pathability = GetDataFromImage(gi.pathing_grid, pt.x, pt.y);

		if (pathability > 0) {
			bool pass = true;
			for (auto asl : all_start_locations) {
				if (pt.manhattan(asl) <= 5) {
					pass = false;
				}
			}
			if (pass) {
				continue;
			}
		}
		Zone* candidate;

		if (height != pt.source->zone_height) {
			candidate = FindAdjacentZoneWithHeight(pt.x, pt.y, height);
			if (!candidate) {
				candidate = new Zone();
				pt.source->adjacent_zones.insert(candidate);
				candidate->adjacent_zones.insert(pt.source);
				candidate->zone_height = height;
			}
		}
		else {
			candidate = pt.source;
		}

		candidate->tiles++;
		candidate->reverse_lookup.push_back(Point2DI(pt.x, pt.y));
		zone_storage->set(pt.x, pt.y, candidate);

		for XDELTA(1) {
			for YDELTA(1) {
				if (!(dx || dy)) {
					continue;
				}
				auto cx = pt.x + dx, cy = pt.y + dy;
				if (data.closed[cx][cy]) {
					if (auto z = (*zone_storage)(cx, cy)) {
						if (candidate != z && !SETCONTAINS(candidate->adjacent_zones, z)) {
							candidate->adjacent_zones.insert(z);
							z->adjacent_zones.insert(candidate);
						}
					}
				}
				else {
					PartitionPoint npt(cx, cy, candidate);
					data.closed[cx][cy] = 1;
					data.open.push(npt);
				}
			}
		}
	}

	for (Zone* z : *zone_storage) {
		if (z->merged) {
			continue;
		}
		bool merged = false;
		do {
			merged = false;
			for (Zone* a : z->adjacent_zones) {
				assert(a != z);
				if (a->merged) {
					continue;
				}
				if (z->zone_height == a->zone_height) {
					for (Zone* t : a->adjacent_zones) {
						if (t == z) {
							continue;
						}
						if (t->merged) {
							continue;
						}
						t->adjacent_zones.insert(z);
						z->adjacent_zones.insert(t);
					}
					z->tiles += a->tiles;
					for (auto coord : a->reverse_lookup) {
						zone_storage->set(coord.x, coord.y, z);
						z->reverse_lookup.push_back(coord);
					}
					zone_storage->remove_zone(a);
					a->merged = true;
					merged = true;
				}
			}
		} while (merged);
	}
}

Point2DIL* Point2DIL::Flip(Point2DIL* prev) {
	Point2DIL* next_temp = next;
	next = prev;
	if (!next_temp) {
		return this;
	}
	return next_temp->Flip(this);
}

void Point2DIL::Collapse() {
	if (next) {
		next->Collapse();
	}
	delete this;
}

struct AStarElem : Point2DI {
	int lastX;
	int lastY;
	int G;
};

Point2DIL* AStar(const Point2DI& from, const Point2DI& to, ObservationInterface* obs) {
	auto gi = obs->GetGameInfo();
	uint8_t** closed;
	AStarElem** map;
	closed = new uint8_t*[gi.width];
	map = new AStarElem*[gi.width];
	for XRANGE(gi.width) {
		closed[x] = new uint8_t[gi.height];
		map[x] = new AStarElem[gi.height];
	}

	auto H = [&to](int x, int y) -> int {
		int dx = qabs(x - to.x);
		int dy = qabs(y - to.y);
		return qmax(dx, dy);
	};

	auto F = [map, H](int x, int y) -> int {
		return map[x][y].G + H(x, y);
	};

	auto FCompare = [F](Point2DI L, Point2DI R) -> bool {
		return F(L.x, L.y) > F(R.x, R.y);
	};
	
	std::priority_queue<Point2DI, std::vector<Point2DI>, std::function<bool(Point2DI, Point2DI)>> open(FCompare);
	open.push(from);
	closed[from.x][from.y] = 1;

	bool success = false;
	while (!open.empty()) {
		auto pt = open.top();
		open.pop();

		if (pt.x == to.x && pt.y == to.y) {
			success = true;
			break;
		}

		for XDELTA(1) {
			for YDELTA(1) {
				auto cx = pt.x + dx, cy = pt.y + dy;
				if (!closed[cx][cy]) {
					map[cx][cy].G = map[pt.x][pt.y].G + 1;
					map[cx][cy].lastX = pt.x;
					map[cx][cy].lastY = pt.y;
					open.push(Point2DI(cx, cy));
					closed[cx][cy] = 1;
				}
			}
		}
	}

	Point2DIL* result = nullptr;
	if (success) {
		int cx = to.x, cy = to.y;
		Point2DIL* prev = nullptr;

		while (cx != from.x || cy != from.y) {
			Point2DIL* curr = new Point2DIL(cx, cy, nullptr);
			if (!result) {
				result = curr;
			}
			if (prev) {
				prev->next = curr;
			}
			prev = curr;
			int nx = map[cx][cy].lastX, ny = map[cx][cy].lastY;
			cx = nx; cy = ny;
		}

		result = result->Flip();
	}

	for XRANGE(gi.width) {
		delete[] closed[x];
		delete[] map[x];
	}
	delete[] closed;
	delete[] map;

	return result;
}

ImageData ZoneStorage::GenerateZonesImage() {
	ImageData target;
	target.width = w;
	target.height = h;
	target.bits_per_pixel = 8;
	target.data = std::string(w*h, 0);
	for XRANGE(w) {
		for YRANGE(h) {
			auto tn = x + (h - 1 - y) * w;
			auto z = (*this)(x, y);
			if (z) {
				target.data[tn] = z->zone_color;
			}
			else {
				target.data[tn] = 0;
			}
		}
	}
	return target;
}