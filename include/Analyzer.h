#pragma once

#include <functional>

class Node;

template<typename Archive>
using Analyzer = std::function<void (const Node&, Archive&)>;