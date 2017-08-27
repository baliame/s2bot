#include <sc2api/sc2_api.h>

#include <iostream>
#include <fstream>

using namespace sc2;

void ImageDataToPGM(std::ofstream& dest, const ImageData& source) {
	
	dest << "P2" << std::endl;
	dest << source.width << " " << source.height << std::endl;
	for (int y = 0; y < source.height; y++) {
		for (int x = 0; x < source.width; x++) {
			if (x != 0) {
				dest << " ";
			}
			unsigned char res;
			SampleImageData(source, Point2D(x, y), res);
			dest << res;
		}
		dest << std::endl;
	}
}

class Bot : public Agent {
public:
    virtual void OnGameStart() final {
		Control()->GetObservation();
		const sc2::ObservationInterface* obs = Observation();
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

    virtual void OnStep() final {
        std::cout << Observation()->GetGameLoop() << std::endl;
    }
};

int main(int argc, char* argv[]) {
    Coordinator coordinator;
    coordinator.LoadSettings(argc, argv);

    Bot bot;
    coordinator.SetParticipants({
        CreateParticipant(Race::Terran, &bot),
        CreateComputer(Race::Zerg)
    });

    coordinator.LaunchStarcraft();
    coordinator.StartGame(sc2::kMapBelShirVestigeLE);

    while (coordinator.Update()) {
    }

    return 0;
}