#pragma once
#include <string>
#include <vector>
#include <map>
#include <utility>
#include "domain.h"
#include "router.h"
namespace router {
	class TransportRouter {
	public:
		using Type = double;
		void SetSettings(TransportRouterSettings settings);
		void ChangeSize(size_t size) {
			graph = graph::DirectedWeightedGraph<Type>(size);
		}
		graph::Router<Type> GetRouter() {
			return graph::Router<Type>(graph);
		}
		void AddStopEnge(const catalogue::parse_structs::Stop* stop1, const catalogue::parse_structs::Stop* stop2, double time, const std::string& name, int span_count);
		std::optional<graph::Router<router::TransportRouter::Type>::RouteInfo> FindWay(const catalogue::parse_structs::Stop* stop1, const catalogue::parse_structs::Stop* stop2, const graph::Router<Type>& router);
		const graph::Edge<Type>& GetEdge(graph::EdgeId id) {
			return graph.GetEdge(id);
		}
		int bus_wait_time = 0;
		double bus_velocity = 0;
	private:
		graph::DirectedWeightedGraph<Type> graph;
		
	};
}