#pragma once
#include <vector>
#include <memory>

template <typename T>
class hierarchy {
protected:
	T* parent;
	std::vector<std::unique_ptr<T>> children;

public:
	hierarchy(T* parent) : parent(parent) { if (parent) parent->adopt(reinterpret_cast<T*>(this)); }
	hierarchy(const hierarchy&) = delete;
	hierarchy(hierarchy&& o) noexcept {
		parent = o.parent;
		children = std::move(o.children);
	}

	void adopt(T* child) { 
		std::unique_ptr<T> temp;
		temp.reset(child);
		children.push_back(std::move(temp)); 
	}
	const std::vector<std::unique_ptr<T>>& get_children() const { return children; }
};