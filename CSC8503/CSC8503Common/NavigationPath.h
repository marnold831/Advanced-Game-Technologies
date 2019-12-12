#pragma once
#include "../../Common/Vector3.h"
#include "../..//Common/Vector4.h"
#include "Debug.h"
#include <vector>

namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		class NavigationPath		{
		public:
			NavigationPath() {}
			~NavigationPath() {}

			void	Clear() {
				waypoints.clear();
			}
			void	PushWaypoint(const Vector3& wp) {
				waypoints.emplace_back(wp);
			}
			bool	PopWaypoint(Vector3& waypoint) {
				if (waypoints.empty()) {
					return false;
				}
				waypoint = waypoints.back();
				waypoints.pop_back();
				return true;
			}
			void DrawLines() {
				for (auto i : waypoints) {
					(i, i + Vector3(0, 40, 0), Vector4(1, 1, 1, 1));
				}
			}

			bool IsEmpty() {
				return waypoints.empty();
			}
			Vector3 GetFirst() { return waypoints[0]; }

		protected:
			std::vector <Vector3> waypoints;
		};
	}
}

