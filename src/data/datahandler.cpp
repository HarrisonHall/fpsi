

#include <vector>

#include "dataframe.hpp"
#include "datahandler.hpp"

namespace fpsi {

DataHandler::DataHandler() {}
DataHandler::~DataHandler() {}

void DataHandler::create_frame() {
  DataFrame new_frame;
  data.push_back(new_frame);
}

}
