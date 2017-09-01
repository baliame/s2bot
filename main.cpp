#include <sc2api/sc2_api.h>

#include <iostream>
#include <fstream>

using namespace sc2;

#define FEATURE_LAYER_TEST

#ifdef FEATURE_LAYER_TEST
#include <sc2renderer/sc2_renderer.h>
const float kCameraWidth = 24.0f;
const int kFeatureLayerSize = 160;
const int kPixelDrawSize = 3;
const int kDrawSize = kFeatureLayerSize * kPixelDrawSize;

void DrawFeatureLayer1BPP(const SC2APIProtocol::ImageData& image_data, int off_x, int off_y) {
	assert(image_data.bits_per_pixel() == 1);
	int width = image_data.size().x();
	int height = image_data.size().y();
	renderer::Matrix1BPP(image_data.data().c_str(), width, height, off_x, off_y, kPixelDrawSize, kPixelDrawSize);
}

void DrawFeatureLayer1BPPID(const ImageData& image_data, int off_x, int off_y) {
	assert(image_data.bits_per_pixel == 1);
	int width = image_data.width;
	int height = image_data.height;
	renderer::Matrix1BPP(image_data.data.c_str(), width, height, off_x, off_y, kPixelDrawSize, kPixelDrawSize);
}

void DrawFeatureLayerUnits8BPP(const SC2APIProtocol::ImageData& image_data, int off_x, int off_y) {
	assert(image_data.bits_per_pixel() == 8);
	int width = image_data.size().x();
	int height = image_data.size().y();
	renderer::Matrix8BPPPlayers(image_data.data().c_str(), width, height, off_x, off_y, kPixelDrawSize, kPixelDrawSize);
}

void DrawFeatureLayerHeightMap8BPP(const SC2APIProtocol::ImageData& image_data, int off_x, int off_y) {
	assert(image_data.bits_per_pixel() == 8);
	int width = image_data.size().x();
	int height = image_data.size().y();
	renderer::Matrix8BPPHeightMap(image_data.data().c_str(), width, height, off_x, off_y, kPixelDrawSize, kPixelDrawSize);
}

void DrawFeatureLayerHeightMap8BPPID(const ImageData& image_data, int off_x, int off_y) {
	assert(image_data.bits_per_pixel == 8);
	int width = image_data.width;
	int height = image_data.height;
	renderer::Matrix8BPPHeightMap(image_data.data.c_str(), width, height, off_x, off_y, kPixelDrawSize, kPixelDrawSize);
}
#endif

static bool SampleImageData(const std::string& data, int width, int height, const Point2D& point, unsigned char& result) {
	Point2DI pointI(int(point.x), int(point.y));
	if (pointI.x < 0 || pointI.x >= width || pointI.y < 0 || pointI.y >= height) {
		return false;
	}

	// Image data is stored with an upper left origin.
	assert(data.size() == width * height);
	result = data[pointI.x + (height - 1 - pointI.y) * width];
	return true;
}

static bool SampleImageData(const ImageData& data, const Point2D& point, unsigned char& result) {
	return SampleImageData(data.data, data.width, data.height, point, result);
}

void ImageDataToPGM(std::ofstream& dest, const ImageData& source) {
	
	dest << "P2" << std::endl;
	dest << source.width << " " << source.height << std::endl;
	dest << "255" << std::endl;
	for (int y = 0; y < source.height; y++) {
		for (int x = 0; x < source.width; x++) {
			if (x != 0) {
				dest << " ";
			}
			unsigned char res;
			SampleImageData(source, Point2D(x, y), res);
			dest << (int)res;
		}
		dest << std::endl;
	}
}

class Bot : public Agent {
	void DumpImages() {
		const ObservationInterface* obs = Observation();
		auto game_info = obs->GetGameInfo();
		std::cout << "Hello, World!" << std::endl;

		std::ofstream heightmap("heightmap.pgm");
		ImageDataToPGM(heightmap, game_info.terrain_height);
		heightmap.close();

		std::ofstream pathing_grid("pathing_grid.pgm");
		ImageDataToPGM(pathing_grid, game_info.pathing_grid);
		heightmap.close();

		std::ofstream placement_grid("placement_grid.pgm");
		ImageDataToPGM(placement_grid, game_info.placement_grid);
		heightmap.close();
	}

public:
    virtual void OnGameStart() final {
		
#ifdef FEATURE_LAYER_TEST
		renderer::Initialize("Feature layers", 50, 50, kDrawSize, 2 * kDrawSize);
#endif
		Control()->GetObservation();
		// DumpImages();
		const ObservationInterface* obs = Observation();
		//FloodPartitionZones(obs);

	}

    virtual void OnStep() final {
        // std::cout << Observation()->GetGameLoop() << std::endl;

#ifdef FEATURE_LAYER_TEST
		Control()->GetObservation();
		const ObservationInterface* obs = Observation();
		const SC2APIProtocol::Observation* observation = Observation()->GetRawObservation();

		DrawFeatureLayerHeightMap8BPPID(obs->GetGameInfo().terrain_height, 0, 0);
		DrawFeatureLayerHeightMap8BPPID(obs->GetGameInfo().pathing_grid, 0, kDrawSize);

		renderer::Render();
#endif
	}

	virtual void OnGameEnd() final {
#ifdef FEATURE_LAYER_TEST
		renderer::Shutdown();
#endif
	}
};

int main(int argc, char* argv[]) {
    Coordinator coordinator;
    coordinator.LoadSettings(argc, argv);

#ifdef FEATURE_LAYER_TEST
	FeatureLayerSettings settings(kCameraWidth, kFeatureLayerSize, kFeatureLayerSize, kFeatureLayerSize, kFeatureLayerSize);
	coordinator.SetFeatureLayers(settings);
#endif

    Bot bot;
    coordinator.SetParticipants({
        CreateParticipant(Race::Terran, &bot),
        CreateComputer(Race::Zerg)
    });

    coordinator.LaunchStarcraft();
    coordinator.StartGame(kMapBelShirVestigeLE);

    while (coordinator.Update()) {
    }

    return 0;
}