#include "layout.hpp"

void
layout_node::remove()
{
    if (parent_)
    {
        parent_->remove(this);
        parent_ = nullptr;
    }
}
