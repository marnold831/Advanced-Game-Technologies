#include "HighscorePacketReceiver.h"

#include "..\\GameTech\TutorialGame.h"

using namespace NCL;
using namespace CSC8503;
HighscorePacketReceiver::HighscorePacketReceiver(std::string name, NCL::CSC8503::TutorialGame* inGame) : game(inGame)
{
}

HighscorePacketReceiver::~HighscorePacketReceiver()
{
}

void HighscorePacketReceiver::ReceivePacket(int type, GamePacket* payload, int source)
{

	if (type == String_Message) {
		StringPacket* realPacket = (StringPacket*)payload;
		std::string msg = realPacket->GetStringFromData();
		

		vector<int> scores;
		vector<string> names;

		std::ofstream myfile;
		myfile.open(NCL::Assets::DATADIR + "highscores" + std::to_string(source) + ".txt");

		std::stringstream ss;
		ss << msg;
		string temp;
		int number;
		while (!ss.eof()) {
			ss >> temp;

			if (std::stringstream(temp) >> number)
				scores.push_back(number);
			else
				names.push_back(temp);

			temp = "";
		}

		for (int i = 0; i < scores.size(); ++i) {
			string output = names[i] + " " + std::to_string(scores[i]) + "\n";
			myfile << output;
		}
		if (source == 0) {
			game->ServerReceived(true);
		}
	}
}


