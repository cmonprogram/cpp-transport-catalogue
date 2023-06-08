#include "transport_catalogue.h"
//#include "tests.h"

using namespace std;
int main()
{
    catalogue::TransportCatalogue catalogue;
    detail::LoadString(catalogue, std::cin);
    //TransportCatalogue_AddStop();
}
