#pragma once
#include "GameClient.h"
#include <iostream>

#include "../../Common/Assets.h"
#include <sstream>
#include <fstream>
namespace NCL {
	namespace CSC8503 {
		class TutorialGame;
	}
}

		class HighscorePacketReceiver : public PacketReceiver {
		public:

			HighscorePacketReceiver(std::string name, NCL::CSC8503::TutorialGame* game);
			~HighscorePacketReceiver();

			void ReceivePacket(int type, GamePacket* payload, int source);
			//HighscorePacketReceiver(std::string name) {
			//	this->name = name;
			//}

			//void ReceivePacket(int type, GamePacket* payload, int source) {
			//	if (type == String_Message) {
			//		StringPacket* realPacket = (StringPacket*)payload;
			//		std::string msg = realPacket->GetStringFromData();


			//		vector<int> scores;
			//		vector<string> names;

			//		std::ofstream myfile;
			//		myfile.open(NCL::Assets::DATADIR + "highscores" + std::to_string(source) + ".txt");

			//		std::stringstream ss;
			//		ss << msg;
			//		string temp;
			//		int number;
			//		while (!ss.eof()) {
			//			ss >> temp;

			//			if (std::stringstream(temp) >> number)
			//				scores.push_back(number);
			//			else
			//				names.push_back(temp);

			//			temp = "";
			//		}

			//		for (int i = 0; i < scores.size(); ++i) {
			//			string output = names[i] + " " + std::to_string(scores[i]) + "\n";
			//			myfile << output;
			//		}
			//		/*if (source == 0) {
			//			game->ServerReceived(true);
			//		}*/
			//	}

			//}

		protected:
			std::string name;
			NCL::CSC8503::TutorialGame* game;
			
		};
	
