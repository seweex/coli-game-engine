#pragma once

#include <bit>
#include <span>
#include <memory>
#include <future>
#include <string_view>
#include <string>
#include <chrono>
#include <ranges>
#include <variant>
#include <fstream>
#include <concepts>
#include <optional>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <execution>
#include <syncstream>
#include <filesystem>

#include <locale>
#include <random>
#include <codecvt>
#include <cwchar>
#include <cstring>

#include <cuchar>

#include <set>
#include <array>
#include <vector>
#include <forward_list>
#include <unordered_map>
#include <unordered_set>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <nlohmann/json.hpp>
#include <tiny_obj_loader.h>

#include "Common/Identifiable.hxx"
#include "Common/KeepsChange.hxx"
#include "Common/Singleton.hxx"
#include "Common/Stateful.hxx"