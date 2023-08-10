#include "transport_router.h"

void router::TransportRouter::SetSettings(TransportRouterSettings settings) {
	bus_wait_time = settings.bus_wait_time;
	bus_velocity = settings.bus_velocity;
}

void router::TransportRouter::AddStopEnge(const catalogue::parse_structs::Stop* stop1, const catalogue::parse_structs::Stop* stop2, double time, const std::string& name, int span_count) {
	//graph.AddEdge(graph::Edge<Type>{ stop1->id, stop1->id, bus_wait_time});
	graph.AddEdge(graph::Edge<Type>{ stop1->id, stop2->id, time, name, span_count});
}

std::optional<graph::Router<router::TransportRouter::Type>::RouteInfo> router::TransportRouter::FindWay(const catalogue::parse_structs::Stop* stop1, const catalogue::parse_structs::Stop* stop2, const graph::Router<Type>& router){
	std::optional<graph::Router<router::TransportRouter::Type>::RouteInfo> info = router.BuildRoute(stop1->id, stop2->id);
	return info;
}
