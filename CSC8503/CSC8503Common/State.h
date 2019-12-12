#pragma once

namespace NCL {
	namespace CSC8503 {
		class State		{
		public:
			State() {}
			virtual ~State() {}
			virtual void Update() = 0; //Pure virtual base class
		};

		typedef void(*StateFunc)(void*, void*);

		class GenericState : public State		{
		public:
			GenericState(StateFunc someFunc, void* objectA, void* objectB) {
				func		= someFunc;
				funcObjectAData = objectA;
				funcObjectBData = objectB;
			}
			virtual void Update() {
				if (funcObjectAData != nullptr && funcObjectBData != nullptr) {
					func(funcObjectAData, funcObjectBData);
				}
			}
		protected:
			StateFunc func;
			void* funcObjectAData;
			void* funcObjectBData;
		};
	}
}