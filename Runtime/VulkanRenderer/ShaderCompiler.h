#pragma once
#include <string>
#include <vector>

namespace vkgfx {

class DefineList {
public:
	
private:
	struct Item {
		std::string key;
		int			value;
	};
	std::vector<Item> _defineList;
};

}
